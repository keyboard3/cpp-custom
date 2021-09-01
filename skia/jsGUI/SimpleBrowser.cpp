/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SimpleBrowser.h"
#include "component.h"
#include "curl/curl.h"
#include "editor_layer.h"
#include "evalJs.h"
#include "filesystem"
#include "htmlParser.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "iostream"
#include "list"
#include "quickjs/quickjs-libc.h"
#include "stdlib.h"
#include "string.h"
using namespace std;

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
JSRuntime *rt;
JSContext *ctx;
DOM *rootDOM;
DivComponent *rootDrawObj;
sk_app::Window *globalWindow;
int barHeight = 40;
int canvasTop = barHeight;
string address;
void loadUri(string uri);
string getRemoteContent(string url);
string getFileContent(string filepath);
Application *Application::Create(int argc, char **argv, void *platformData) {
  return new SimpleBrowser(argc, argv, platformData);
}

void onSearchChar(char ch) {
  if (ch == '\n') {
    loadUri(address);
    return;
  }
  address += ch;
}
/*
kNativeGL_BackendType: openGL gpu
kVulkan_BackendType: vulkan gpu
kRaster_BackendType: raster cpu 上面都不支持最后会采用这个
*/
SimpleBrowser::SimpleBrowser(int argc, char **argv, void *platformData)
    : fBackendType(Window::kRaster_BackendType) //(Window::kNativeGL_BackendType
{
  initJsEngine();
  /*
    创建windowDelegate来跟踪某些事件
    Mac上创建Cocoa window,创建1280*960的矩形作为contentView
    将这个fwindow加入到gWindowMap上
   */
  SkGraphics::Init();
  globalWindow = fWindow = Window::CreateNativeWindow(platformData);
  fWindow->setRequestedDisplayParams(DisplayParams());
  // 向fWindow绘制这一层，绘制以及事件都会callback回调
  fWindow->pushLayer(this);
  // 初始化完成，attach会触发的onBackendCreated
  fWindow->attach(fBackendType);
  //初始化Html页面
  address = "file://" + string(filesystem::current_path().c_str()) +
            "/jsGUI/index.html";
  // address="https://raw.githubusercontent.com/keyboard3/cpp-custom/main/skia/jsGUI/index.html";
  loadUri(address);

  //添加地址栏
  addresEditor.fEditor.insert(Editor::TextPosition{0, 0}, address.c_str(),
                              address.length());
  addresEditor.setFont();
  addresEditor.setCharCallback(onSearchChar);
  fWindow->pushLayer(&addresEditor);
  addresEditor.onResize(fWindow->width(), barHeight);
  addresEditor.fEditor.paint(nullptr, Editor::PaintOpts());
}

/* 析构函数 */
SimpleBrowser::~SimpleBrowser() {
  freeJsEngine();
  freeHtmlPage();
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

/*
为 Raster 和 Ganesh backends创建画布的推荐方法是使用
SkSurface，它是一个管理画布命令所绘制到的内存的对象。

GPU Surfaces 必须有一个 GrContext 对象来管理 GPU
上下文以及纹理和字体的相关缓存。 GrContexts 与 OpenGL 上下文或 Vulkan
设备一对一匹配。也就是说，将使用相同的 OpenGL 上下文或 Vulkan 设备渲染的所有
SkSurface 都应该共享一个 GrContext。 Skia 不会为您创建 OpenGL 上下文或 Vulkan
设备。

window::onPaint
  fwindowContext 获取到 backBuffer(SkSurface*) 指针
  关闭脏标记
  调用每个 layer 的 onPrePaint 绘制前做准备
  调用每个 layer 的 onPaint 向 backBuffer(SkSurface*) 上绘制
  flushAndSubmit 刷新提交给 gpu
  swapBuffers 写入的buffer 不同平台不一样的处理。Android上 是
ANativeWindow_unlockAndPost(window)
*/
void SimpleBrowser::onPaint(SkSurface *surface) {
  auto canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);
  SkPaint paint;
  SkFont font;
  font.setSubpixel(true);

  std::list<DivComponent *> divs;
  if (rootDrawObj != nullptr)
    divs.push_back(rootDrawObj);
  for (auto div : divs) {
    // std::cout << "渲染 div" << std::endl;
    // div有背景就画一个矩形
    if (div->background != 0) {
      paint.setColor(div->background);
      SkRect rect =
          SkRect::MakeXYWH(div->x, div->y + canvasTop, div->width, div->height);
      canvas->drawRect(rect, paint);
    }
    if (div->innerText.length() != 0) {
      font.setSize(div->fontSize);
      paint.setColor(div->color);
      // std::cout << "onPaint:innerText:" << div->innerText << std::endl;
      canvas->drawSimpleText(div->innerText.c_str(), div->innerText.length(),
                             SkTextEncoding::kUTF8, div->x + div->paddingLeft,
                             canvasTop + div->y + div->height / 2 +
                                 div->fontSize / 2,
                             font, paint);
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
  rootDrawObj->innerText = "global response";
  fWindow->inval();
  return false;
}
/* 从点击事件中找到相应view上注册的点击回调 */
Click *SimpleBrowser::onFindClickHandler(SkScalar x, SkScalar y,
                                         skui::ModifierKey modi) {
  std::list<DivComponent *> divs;
  if (rootDrawObj != nullptr)
    divs.push_back(rootDrawObj);
  DivComponent *findDiv = nullptr;
  for (auto div : divs) {
    bool isOuter = false;
    if (x < div->x || y < div->y + canvasTop)
      isOuter = true;
    if (x > div->x + div->width || y > div->y + div->height + canvasTop)
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

//===----------------------------------------------------------------------===//
// 初始化GUI的DOM树和脚本能力
//===----------------------------------------------------------------------===//
map<string, DOM *> *idDomMap;
SkColor getColor(JSContext *ctx, JSValue colorVal);
SkColor getColor(string strColor);
SkColor getColor(string strColor, SkColor defColor);
JSContext *JS_NewCustomContext(JSRuntime *rt);
DivComponent *domToDiv(DOM *dom, JSContext *ctx);
JSValue readyJSDocument();
void drawRoot();
void parseDOM(DivComponent *parentDiv, DOM *curDOM, JSContext *ctx);

void SimpleBrowser::initJsEngine() {
  rt = JS_NewRuntime();
  ctx = JS_NewCustomContext(rt);

  /* 给global对象添加console能力 */
  js_std_add_helpers(ctx, 0, 0);
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global_obj, "window", global_obj);
}

void SimpleBrowser::freeJsEngine() {
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
}

void loadUri(string uri) {
  idDomMap = new map<string, DOM *>();
  cout << "loadUri:" << uri << endl;
  string htmlContent = "";
  if (uri.find("file://") != std::string::npos) {
    htmlContent = getFileContent(uri.substr(7));
  } else if (uri.find("http") != std::string::npos) {
    htmlContent = getRemoteContent(uri);
  } else {
    perror("无效的uri\n");
    return;
  }
  if (htmlContent.length() == 0) {
    perror("没有内容\n");
    return;
  }
  rootDOM = parseHtml(htmlContent);
  readyJSDocument();
  drawRoot();
}

void SimpleBrowser::freeHtmlPage() {
  //释放申请的资源
}

void drawRoot() {
  rootDrawObj = domToDiv(rootDOM, ctx);
  if (rootDOM->children->type != node_list)
    return;
  //将根节点html内容下的子节点，递归解析出绘图树
  for (auto child : *rootDOM->children->list) {
    parseDOM(rootDrawObj, child, ctx);
  }
  globalWindow->inval();
}

JSContext *JS_NewCustomContext(JSRuntime *rt) {
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

/*
给element的html设置内容
*/
JSValue js_setInnerHtml(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  string attributeKey = JS_ToCString(ctx, argv[0]);
  string htmlStr = JS_ToCString(ctx, argv[1]);
  string id = JS_ToCString(ctx, JS_GetPropertyStr(ctx, this_val, "id"));
  if (idDomMap->find(id) == idDomMap->end())
    return JS_UNDEFINED;

  DOM *dom = (*idDomMap)[id];
  dom->innerHtml = htmlStr;

  parseHtmlToDOM(dom->innerHtml, dom);
  drawRoot();
  return JS_UNDEFINED;
}
/*
给element提供根据key来设置attribute属性值的能力
*/
JSValue js_setAttribute(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  string attributeKey = JS_ToCString(ctx, argv[0]);
  string value = JS_ToCString(ctx, argv[1]);
  string id = JS_ToCString(ctx, JS_GetPropertyStr(ctx, this_val, "id"));
  if (idDomMap->find(id) == idDomMap->end())
    return JS_UNDEFINED;

  DOM *dom = (*idDomMap)[id];
  dom->attributes.erase(attributeKey);
  dom->attributes.emplace(attributeKey, value);
  drawRoot();
  return JS_UNDEFINED;
}

/*
给element提供根据key获取attribute属性值的能力
*/
JSValue js_getAttribute(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  string attributeKey = JS_ToCString(ctx, argv[0]);
  string id = JS_ToCString(ctx, JS_GetPropertyStr(ctx, this_val, "id"));
  if (idDomMap->find(id) == idDomMap->end())
    return JS_UNDEFINED;

  DOM *dom = (*idDomMap)[id];
  if (dom->attributes.find(attributeKey) != dom->attributes.end())
    return JS_NewString(ctx, dom->attributes[attributeKey].c_str());

  return JS_UNDEFINED;
}

/*
给document提供getElementById的api，以通过定义id属性来找到DOM节点
并将这个c++ DOM对象映射成js的element对象。给这个element上提供如下方法
setAttribute,getAttribute, innerText,innerHtml
*/
JSValue js_getElementById(JSContext *ctx, JSValueConst this_val, int argc,
                          JSValueConst *argv) {
  string id = JS_ToCString(ctx, argv[0]);
  if (idDomMap->find(id) == idDomMap->end())
    return JS_UNDEFINED;

  //给基本属性设置赋值操作时，更新UI
  DOM *dom = (*idDomMap)[id];
  auto getStrValue = [&](string key) -> const char * {
    if (dom->attributes.find(key) != dom->attributes.end())
      return dom->attributes[key].c_str();
    return "";
  };
  JSValue element_obj = JS_NewObject(ctx);
  auto initProperty = [&](string key) {
    string reKey = "_" + key;
    if (key == "innerHtml") {
      JS_SetPropertyStr(ctx, element_obj, reKey.c_str(),
                        JS_NewString(ctx, dom->innerHtml.c_str()));
    } else {
      JS_SetPropertyStr(ctx, element_obj, reKey.c_str(),
                        JS_NewString(ctx, getStrValue(key)));
    }

    string getValue = "()=>{ return this." + reKey + ";}";
    JSValue getValueFun =
        JS_EvalThis(ctx, element_obj, getValue.c_str(), getValue.length(),
                    (key + "_getValue").c_str(), JS_EVAL_TYPE_GLOBAL);
    string setValue = "(val)=>{ this.setAttribute('" + key + "',val);this." +
                      reKey + "=val;}";
    if (key == "innerHtml") {
      setValue = "(val)=>{ this.setInnerHtml('" + key + "',val);this." + reKey +
                 "=val;}";
    }
    JSValue setValueFun =
        JS_EvalThis(ctx, element_obj, setValue.c_str(), setValue.length(),
                    (key + "_setValue").c_str(), JS_EVAL_TYPE_GLOBAL);
    JSAtom nameAtom = JS_NewAtom(ctx, key.c_str());
    JS_DefineProperty(ctx, element_obj, nameAtom, JS_UNDEFINED, getValueFun,
                      setValueFun,
                      JS_PROP_C_W_E | JS_PROP_HAS_GET | JS_PROP_HAS_SET);
  };
  initProperty("id");
  initProperty("width");
  initProperty("height");
  initProperty("paddingLeft");
  initProperty("innerHtml");
  //设置更新方法
  JSValue jsGetAttribute =
      JS_NewCFunction(ctx, js_getAttribute, "getAttribute", 1);
  JSValue jsSetAttribute =
      JS_NewCFunction(ctx, js_setAttribute, "setAttribute", 1);
  JSValue jsSetInnerHtml =
      JS_NewCFunction(ctx, js_setInnerHtml, "setInnerHtml", 1);
  JS_SetPropertyStr(ctx, element_obj, "getAttribute", jsGetAttribute);
  JS_SetPropertyStr(ctx, element_obj, "setAttribute", jsSetAttribute);
  JS_SetPropertyStr(ctx, element_obj, "setInnerHtml", jsSetInnerHtml);
  return element_obj;
}

JSValue readyJSDocument() {
  JSValue document_obj = JS_NewObject(ctx);
  JS_SetPropertyStr(
      ctx, document_obj, "getElementById",
      JS_NewCFunction(ctx, js_getElementById, "getElementById", 1));

  JSValue global_obj = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global_obj, "document", document_obj);
}

DivComponent *domToDiv(DOM *dom, JSContext *ctx) {
  DivComponent *div = new DivComponent();
  auto hasAttribute = [&](string key) -> bool {
    return dom->attributes.find(key) != dom->attributes.end();
  };
  auto getNumValue = [&](string key, int32_t defVal) -> int32_t {
    if (hasAttribute(key))
      return stoi(dom->attributes[key], nullptr, defVal);
    return defVal;
  };
  auto getStrValue = [&](string key) -> string {
    if (hasAttribute(key))
      return dom->attributes[key];
    return "";
  };

  div->width = getNumValue("width", 0);
  div->height = getNumValue("height", 0);
  div->x = getNumValue("x", 0);
  div->y = getNumValue("y", 0);
  div->fontSize = getNumValue("fontSize", 12);
  div->paddingLeft = getNumValue("paddingLeft", 0);
  div->background = getColor(getStrValue("background"));
  div->color = getColor(getStrValue("color"), SK_ColorBLACK);

  //给onclick属性上构建一个匿名函数，绑定到绘图对象上
  if (hasAttribute("onclick")) {
    string clickFun = "()=>{" + dom->attributes["onclick"] + "}";
    JSValue ret =
        JS_Eval(ctx, clickFun.c_str(), clickFun.length(),
                (dom->name + "_onclick").c_str(), JS_EVAL_TYPE_GLOBAL);
    JSValue global_obj = JS_GetGlobalObject(ctx);
    if (ret.tag == JS_UNDEFINED.tag) {
      perror("no function found\n");
      return div;
    }
    div->clickThis = global_obj;
    div->onClick = ret;
  }

  //给id属性设置map映射表上
  if (hasAttribute("id"))
    idDomMap->insert({getStrValue("id"), dom});
  return div;
}

void parseDOM(DivComponent *parentDiv, DOM *curDOM, JSContext *ctx) {
  //遇到js标签执行js代码
  if (curDOM->name == "script") {
    JS_Eval(ctx, curDOM->children->strVal.c_str(),
            curDOM->children->strVal.length(), "<script>", JS_EVAL_TYPE_GLOBAL);
    //处理eval产生的微任务队列
    JSContext *ctx1;
    int err;
    err = JS_ExecutePendingJob(JS_GetRuntime(ctx), &ctx1);
    if (err < 0) {
      js_std_dump_error(ctx1);
    }
    return;
  }
  //构建绘图树
  DivComponent *dom = domToDiv(curDOM, ctx);
  //如果是本文节点设置绘制文本
  if (curDOM->children->type == node_str) {
    dom->innerText = curDOM->children->strVal;
    parentDiv->children.push_back(dom);
    return;
  }
  //如果是dom列表就递归解析到绘图节点上
  for (auto child : *curDOM->children->list) {
    parseDOM(dom, child, ctx);
  }
  parentDiv->children.push_back(dom);
}

SkColor getColor(JSContext *ctx, JSValue colorVal) {
  std::string strColor;
  if (JS_IsString(colorVal)) {
    strColor = JS_ToCString(ctx, colorVal);
  }
  return getColor(strColor);
}

SkColor getColor(string strColor) {
  return getColor(strColor, SK_ColorTRANSPARENT);
}
SkColor getColor(string strColor, SkColor defColor) {
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

  return defColor;
}

string getFileContent(string filepath) {
  string content, line;
  ifstream htmlFile(filepath);
  if (htmlFile.is_open()) {
    while (getline(htmlFile, line)) {
      content += line;
    }
    htmlFile.close();
  }
  return content;
}

/* resizable buffer */
typedef struct {
  char *buf;
  size_t size;
} memory;

/*使用回调而不用默认为了保证兼容性*/
size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *ctx) {
  size_t realsize = sz * nmemb;
  memory *mem = (memory *)ctx;
  char *ptr = (char *)realloc(mem->buf, mem->size + realsize);
  if (!ptr) {
    /* out of memory */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  mem->buf = ptr;
  memcpy(&(mem->buf[mem->size]), contents, realsize);
  mem->size += realsize;
  return realsize;
}

string getRemoteContent(string url) {
  CURLcode res;
  CURL *curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); /*跟随重定向*/

    memory *mem = (memory *)malloc(sizeof(memory));
    mem->size = 0;
    mem->buf = (char *)malloc(1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, grow_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, mem);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, mem);
    /* 执行请求， res 会得到返回码 */
    res = curl_easy_perform(curl);
    /* 检查错误 */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* 记得清理 */
    curl_easy_cleanup(curl);
    return string(mem->buf);
  }
  return "";
}