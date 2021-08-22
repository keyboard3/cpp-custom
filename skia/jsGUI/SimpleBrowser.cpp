/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SimpleBrowser.h"
#include "component.h"
#include "quickjs/quickjs-libc.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "iostream"

using namespace sk_app;

Application *Application::Create(int argc, char **argv, void *platformData)
{
    return new SimpleBrowser(argc, argv, platformData);
}

SimpleBrowser::SimpleBrowser(int argc, char **argv, void *platformData)
    : fBackendType(Window::kRaster_BackendType) //(Window::kNativeGL_BackendType
{
    std::cout << "应用初始化设置 root div" << std::endl;
    root = new DivComponent();
    root->background = SK_ColorDKGRAY;
    root->x = 10;
    root->y = 10;
    root->width = 400;
    root->height = 200;
    root->paddingLeft = 40;
    root->innerText = "hello world";
    root->color = SK_ColorWHITE;
    root->fontSize = 48;

    JSRuntime *rt = JS_NewRuntime();
    /* 根据平台参数创建window */
    SkGraphics::Init();
    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->setRequestedDisplayParams(DisplayParams());

    // 向fWindow绘制这一层，绘制以及事件都会callback回调
    fWindow->pushLayer(this);

    fWindow->attach(fBackendType);
}
/* 析构函数 */
SimpleBrowser::~SimpleBrowser()
{
    fWindow->detach();
    delete fWindow;
}
/* 修改窗口的window的title */
void SimpleBrowser::updateTitle()
{
    std::cout << "updateTitle" << std::endl;
    if (!fWindow || fWindow->sampleCount() <= 1)
        return;

    SkString title("Hello World ");
    title.append(Window::kRaster_BackendType == fBackendType ? "Raster" : "OpenGL");
    fWindow->setTitle(title.c_str());
}
/* 大概是初始化完，向系统弹出窗口了 */
void SimpleBrowser::onBackendCreated()
{
    std::cout << "onBackendCreated" << std::endl;
    this->updateTitle();
    fWindow->show();
    fWindow->inval();
}

void SimpleBrowser::onPaint(SkSurface *surface)
{
    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    SkFont font;
    font.setSubpixel(true);

    std::list<DivComponent *> divs = {this->root};
    for (auto div : divs)
    {
        // std::cout << "渲染 div" << std::endl;

        // div有背景就画一个矩形
        if (div->background != 0)
        {
            // std::cout << "渲染 背景" << std::endl;
            paint.setColor(div->background);
            SkRect rect = SkRect::MakeXYWH(div->x, div->y, div->width, div->height);
            canvas->drawRect(rect, paint);
        }
        if (div->color != 0 && div->innerText.length() != 0)
        {
            font.setSize(div->fontSize);
            paint.setColor(div->color);
            canvas->drawSimpleText(div->innerText.c_str(), div->innerText.length(), SkTextEncoding::kUTF8, div->x + div->paddingLeft, div->y + div->height / 2 + div->fontSize / 2, font, paint);
        }
        for (auto childDiv : div->children)
            divs.push_back(childDiv);
    }
}

void SimpleBrowser::onIdle()
{
    // 持续重绘
    fWindow->inval();
}
/* 应该是键盘输入的回调 */
bool SimpleBrowser::onChar(SkUnichar c, skui::ModifierKey modifiers)
{
    std::cout << "onChar" << c << std::endl;
    if (' ' == c)
    {
        fBackendType = Window::kRaster_BackendType == fBackendType ? Window::kRaster_BackendType //kNativeGL_BackendType
                                                                   : Window::kRaster_BackendType;
        fWindow->detach();
        fWindow->attach(fBackendType);
    }
    return true;
}

/* 从mouse事件中识别出点击事件 */
bool SimpleBrowser::onMouse(int x, int y, skui::InputState clickState, skui::ModifierKey modifierKeys)
{
    SkPoint point = SkPoint::Make(x, y);
    auto dispatch = [this](Click *c)
    {
        std::cout << "调用点击事件 " << (c->fHasFunc ? "对象是组件" : "对象是window") << std::endl;
        return c->fHasFunc ? c->fFunc(c) : this->onClick(c);
    };

    switch (clickState)
    {
    case skui::InputState::kDown:
        std::cout << "kDown:" << x << " " << y << " 寻找div" << std::endl;
        fClick = nullptr;
        fClick.reset(this->onFindClickHandler(point.x(), point.y(), modifierKeys));
        if (!fClick)
        {
            return false;
        }
        fClick->fPrev = fClick->fCurr = fClick->fOrig = point;
        fClick->fState = skui::InputState::kDown;
        fClick->fModifierKeys = modifierKeys;
        dispatch(fClick.get());
        return true;
    case skui::InputState::kMove:
        if (fClick)
        {
            fClick->fPrev = fClick->fCurr;
            fClick->fCurr = point;
            fClick->fState = skui::InputState::kMove;
            fClick->fModifierKeys = modifierKeys;
            return dispatch(fClick.get());
        }
        return false;
    case skui::InputState::kUp:
        std::cout << "kUp:" << x << " " << y << std::endl;
        if (fClick)
        {
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

/* 从点击事件中找到相应view上注册的点击回调 */
Click *SimpleBrowser::onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi)
{
    std::list<DivComponent *> divs = {this->root};
    for (auto div : divs)
    {
        bool isOuter = false;
        if (x < div->x || y < div->y)
            isOuter = true;
        if (x > div->x + div->width || y > div->y + div->height)
            isOuter = true;
        if (!isOuter)
        {
            std::cout << "找到div" << std::endl;
            if (div->onClick == nullptr) {
                return new Click();
            }
            std::cout << "div有点击事件" << std::endl;
            Click *c = new Click(div->onClick);
            return c;
        }
        for (auto childDiv : div->children)
            divs.push_back(childDiv);
    }
    std::cout << "这个坐标没有div" << std::endl;
    return nullptr;
}

/* 当没找到组件响应点击事件的时候，全局响应点击事件 */
bool SimpleBrowser::onClick(Click *c)
{
    std::cout << "无人响应，则全局响应点击事件:" << c->fCurr.fX << "," << c->fCurr.fY << std::endl;
    root->innerText = "global response";
    return false;
}