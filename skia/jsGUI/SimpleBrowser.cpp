/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SimpleBrowser.h"
#include "component.h"
#include "evalJs.h"
#include "filesystem"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "iostream"
#include "list"
#include "quickjs/quickjs-libc.h"
#include "thread"

#define countof(x) (sizeof(x) / sizeof((x)[0]))
using namespace sk_app;

/*
以mac平台为示例-启动过程：
向NSApp设置AppDelegate来捕获某些全局事件
Application::Create进行window的初始化，即调用下面的new SimpleBrowser过程
等待NSApp run:
会启动事件循环,当接收到applicationDidFinishLaunching时就停掉该循环，交给自己接管

然后while循环阻塞读取event,然后通过sendEvent。
sendEvent是将事件分派给合适的响应者的方法——NSApp处理app事件，事件记录中指示的NSWindow对象处理与窗口相关的事件，鼠标和按键事件转发到合适的NSWindow对象进一步分派.
将窗口失效标志 fIsContentInvalidated 视为单独的事件流。 Window::onPaint()
将清除失效标志，有效地将其从流中删除
*/
Application *Application::Create(int argc, char **argv, void *platformData) {
  return new SimpleBrowser(argc, argv, platformData);
}

SimpleBrowser::SimpleBrowser(int argc, char **argv, void *platformData)
    : fBackendType(Window::kRaster_BackendType) //(Window::kNativeGL_BackendType
{
  initJsEngine();
  initJsPage();
  /*
    创建windowDelegate来跟踪某些事件
    Mac上创建Cocoa window,创建1280*960的矩形作为contentView
    将这个fwindow加入到gWindowMap上
   */
  SkGraphics::Init();
  fWindow = Window::CreateNativeWindow(platformData);
  fWindow->setRequestedDisplayParams(DisplayParams());

  // 向fWindow绘制这一层，绘制以及事件都会callback回调
  fWindow->pushLayer(this);
  // 初始化完成，attach会触发的onBackendCreated
  fWindow->attach(fBackendType);
}

/* 析构函数 */
SimpleBrowser::~SimpleBrowser() {
  fWindow->detach();
  delete fWindow;
}
/* 修改窗口的window的title */
void SimpleBrowser::updateTitle() {
  std::cout << "updateTitle" << std::endl;
  if (!fWindow || fWindow->sampleCount() <= 1)
    return;

  SkString title("Hello World ");
  title.append(Window::kRaster_BackendType == fBackendType ? "Raster"
                                                           : "OpenGL");
  fWindow->setTitle(title.c_str());
}
/* attach完成之后会触发调用 */
void SimpleBrowser::onBackendCreated() {
  std::cout << "onBackendCreated" << std::endl;
  this->updateTitle();
  fWindow->show();
  //标记脏标记，loop下一次会重绘
  fWindow->inval();
}

void SimpleBrowser::onPaint(SkSurface *surface) {
  auto canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);
  SkPaint paint;
  SkFont font;
  font.setSubpixel(true);

  std::list<DivComponent *> divs;
  if (this->root != nullptr)
    divs.push_back(this->root);
  for (auto div : divs) {
    // std::cout << "渲染 div" << std::endl;
    // div有背景就画一个矩形
    if (div->background != 0) {
      paint.setColor(div->background);
      SkRect rect = SkRect::MakeXYWH(div->x, div->y, div->width, div->height);
      canvas->drawRect(rect, paint);
    }
    if (div->color != 0 && div->innerText.length() != 0) {
      font.setSize(div->fontSize);
      paint.setColor(div->color);
      // std::cout << "onPaint:innerText:" << div->innerText << std::endl;
      canvas->drawSimpleText(div->innerText.c_str(), div->innerText.length(),
                             SkTextEncoding::kUTF8, div->x + div->paddingLeft,
                             div->y + div->height / 2 + div->fontSize / 2, font,
                             paint);
    }
    for (auto childDiv : div->children)
      divs.push_back(childDiv);
  }
}

void SimpleBrowser::onIdle() {
  /*
    修改fwindow的 fIsContentInvalidated
    为TRUE,在loop没有事件流之后，就会触发PaintWindows完成重绘
    绘制完成之后，还会继续调用onIdle()设置脏标记，以便让下次循环时继续绘制
    如果是Android，则还会fSkiaAndroidApp->postMessage(Message(kContentInvalidated));
    这里没有动画，不做此处理
  */
  // fWindow->inval();
}
/* 应该是键盘输入的回调 */
bool SimpleBrowser::onChar(SkUnichar c, skui::ModifierKey modifiers) {
  std::cout << "onChar" << c << std::endl;
  if (' ' == c) {
    fBackendType = Window::kRaster_BackendType == fBackendType
                       ? Window::kRaster_BackendType // kNativeGL_BackendType
                       : Window::kRaster_BackendType;
    fWindow->detach();
    fWindow->attach(fBackendType);
  }
  return true;
}

/* 从mouse事件中识别出点击事件 */
bool SimpleBrowser::onMouse(int x, int y, skui::InputState clickState,
                            skui::ModifierKey modifierKeys) {
  SkPoint point = SkPoint::Make(x, y);
  auto dispatch = [this](Click *c) {
    return c->fHasFunc ? this->onComponentClick(c) : this->onClick(c);
  };

  switch (clickState) {
  case skui::InputState::kDown:
    std::cout << "kDown:" << x << " " << y << " 寻找div" << std::endl;
    fClick = nullptr;
    fClick.reset(this->onFindClickHandler(point.x(), point.y(), modifierKeys));
    if (!fClick) {
      return false;
    }
    fClick->fPrev = fClick->fCurr = fClick->fOrig = point;
    fClick->fState = skui::InputState::kDown;
    fClick->fModifierKeys = modifierKeys;
    return true;
  case skui::InputState::kMove:
    if (fClick) {
      fClick->fPrev = fClick->fCurr;
      fClick->fCurr = point;
      fClick->fState = skui::InputState::kMove;
      fClick->fModifierKeys = modifierKeys;
    }
    return false;
  case skui::InputState::kUp:
    std::cout << "kUp:" << x << " " << y << std::endl;
    if (fClick) {
      fClick->fPrev = fClick->fCurr;
      fClick->fCurr = point;
      fClick->fState = skui::InputState::kUp;
      fClick->fModifierKeys = modifierKeys;
      bool result = dispatch(fClick.get());
      fClick = nullptr;
      return result;
    }
    return false;
  default:
    // Ignore other cases
    SkASSERT(false);
    break;
  }
  SkASSERT(false);
  return false;
}
/* 组件响应点击事件 */
bool SimpleBrowser::onComponentClick(Click *c) {
  std::cout << "组件响应点击事件:" << c->fCurr.fX << "," << c->fCurr.fY
            << std::endl;
  JSValue ret = JS_Call(ctx, c->fFunc, c->fthis, 0, NULL);
  /* 执行完毕释放掉函数的返回结果，因为回调可以被多次触发 */
  JS_FreeValue(ctx, ret);
  fWindow->inval();
}
/* 当没找到组件响应点击事件的时候，全局响应点击事件 */
bool SimpleBrowser::onClick(Click *c) {
  std::cout << "无人响应，则全局响应点击事件:" << c->fCurr.fX << ","
            << c->fCurr.fY << std::endl;
  root->innerText = "global response";
  fWindow->inval();
  return false;
}
/* 从点击事件中找到相应view上注册的点击回调 */
Click *SimpleBrowser::onFindClickHandler(SkScalar x, SkScalar y,
                                         skui::ModifierKey modi) {
  std::list<DivComponent *> divs;
  if (this->root != nullptr)
    divs.push_back(root);
  DivComponent *findDiv = nullptr;
  for (auto div : divs) {
    bool isOuter = false;
    if (x < div->x || y < div->y)
      isOuter = true;
    if (x > div->x + div->width || y > div->y + div->height)
      isOuter = true;
    if (!isOuter) {
      findDiv = div;
    }
    for (auto childDiv : div->children)
      divs.push_back(childDiv);
  }
  if (findDiv != nullptr) {
    if (findDiv->onClick.tag == JS_TAG_UNDEFINED) {
      return new Click();
    }
    Click *click = new Click(findDiv->clickThis, findDiv->onClick);
    std::cout << "div有点击事件" << std::endl;
    return click;
  }
  std::cout << "这个坐标没有div" << std::endl;
  return nullptr;
}
static JSContext *JS_NewCustomContext(JSRuntime *rt) {
  JSContext *ctx = JS_NewContextRaw(rt);
  if (!ctx)
    return NULL;
  //添加对Object,Function,Array,Number,String,Symbol等构造函数proto及自身初始化
  JS_AddIntrinsicBaseObjects(ctx);
  //初始化标准的Date
  JS_AddIntrinsicDate(ctx);
  //给ctx的eval_internal初始化__JS_EvalInternal的能力
  JS_AddIntrinsicEval(ctx);
  //初始化下面的标准对象
  JS_AddIntrinsicStringNormalize(ctx);
  JS_AddIntrinsicPromise(ctx);
  return ctx;
}
DivComponent *parseDivComponent(JSContext *ctx, JSValue domVal);
SkColor getColor(JSContext *ctx, JSValue colorVal);
SimpleBrowser *globalBrowser = nullptr;
JSValue setRootDOM(JSContext *ctx, JSValueConst this_val, int argc,
                   JSValueConst *argv) {
  JSValueConst rootDom;
  rootDom = argv[0];

  globalBrowser->root = parseDivComponent(ctx, rootDom);
  return JS_UNDEFINED;
}
DivComponent *parseDivComponent(JSContext *ctx, JSValue domVal) {
  DivComponent *comp = new DivComponent();
  int32_t x, y, width, height, paddingLeft, fontSize;
  auto getPropertyInt = [&](int32_t *val, const char *prop) {
    JSValue jsValue = JS_GetPropertyStr(ctx, domVal, prop);
    JS_ToInt32(ctx, val, jsValue);
  };
  /* 解析div主要的属性 */
  getPropertyInt(&x, "x");
  getPropertyInt(&y, "y");
  getPropertyInt(&width, "width");
  getPropertyInt(&height, "height");
  getPropertyInt(&paddingLeft, "paddingLeft");
  getPropertyInt(&fontSize, "fontSize");
  comp->x = x;
  comp->y = y;
  comp->width = width;
  comp->height = height;
  comp->paddingLeft = paddingLeft;
  comp->fontSize = fontSize;
  comp->background =
      getColor(ctx, JS_GetPropertyStr(ctx, domVal, "background"));
  comp->color = getColor(ctx, JS_GetPropertyStr(ctx, domVal, "color"));
  JSValue textValue = JS_GetPropertyStr(ctx, domVal, "innerText");
  comp->innerText = JS_ToCString(ctx, textValue);

  JSValue clickValue = JS_GetPropertyStr(ctx, domVal, "onClick");
  comp->onClick = clickValue;
  comp->clickThis = domVal;

  /* 在JS中数组在底层也是JS对象，通过index属性访问 */
  JSValue childrenValue = JS_GetPropertyStr(ctx, domVal, "children");
  int32_t childLen;
  JSValue lenVal = JS_GetPropertyStr(ctx, childrenValue, "length");
  JS_ToInt32(ctx, &childLen, lenVal);

  for (int i = 0; i < childLen; i++) {
    DivComponent *child =
        parseDivComponent(ctx, JS_GetPropertyUint32(ctx, childrenValue, i));
    comp->children.push_back(child);
  }
  return comp;
}
void SimpleBrowser::initJsEngine() {
  rt = JS_NewRuntime();
  ctx = JS_NewCustomContext(rt);

  /* 给global对象添加console能力 */
  js_std_add_helpers(ctx, 0, 0);
  /* 给global对象添加setRootDOM能力 */
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global_obj, "setRootDOM",
                    JS_NewCFunction(ctx, setRootDOM, "setRootDOM", 1));
  globalBrowser = this;
}

void SimpleBrowser::initJsPage() {
  std::string module_name =
      std::string(std::filesystem::current_path().c_str()) + "/jsGUI/index.js";
  JSValue fileName = JS_NewString(ctx, module_name.c_str());
  js_loadScript(ctx, JS_UNDEFINED, 1, &fileName);

  std::cout << "eval_file:" << std::endl;
  /* 解析运行index.js的生成的quickjs的二进制指令 */
  // js_std_eval_binary(ctx, qjsc_index, qjsc_index_size, 0);
  /* 执行掉后面的微任务队列 */
  JSContext *ctx1;
  int err;
  err = JS_ExecutePendingJob(JS_GetRuntime(ctx), &ctx1);
  if (err < 0) {
    js_std_dump_error(ctx1);
  }
}

SkColor getColor(JSContext *ctx, JSValue colorVal) {
  if (JS_IsString(colorVal)) {
    std::string strColor = JS_ToCString(ctx, colorVal);
    if (strColor == "transparent")
      return SK_ColorTRANSPARENT;
    if (strColor == "black")
      return SK_ColorBLACK;
    if (strColor == "dkgray")
      return SK_ColorDKGRAY;
    if (strColor == "gray")
      return SK_ColorGRAY;
    if (strColor == "ltgray")
      return SK_ColorLTGRAY;
    if (strColor == "white")
      return SK_ColorWHITE;
    if (strColor == "red")
      return SK_ColorRED;
    if (strColor == "green")
      return SK_ColorGREEN;
    if (strColor == "blue")
      return SK_ColorBLUE;
    if (strColor == "yellow")
      return SK_ColorYELLOW;
    if (strColor == "cyan")
      return SK_ColorCYAN;
    if (strColor == "magenta")
      return SK_ColorMAGENTA;
  }
  return SK_ColorTRANSPARENT;
}