#ifndef COMPONENT_DEFINED
#define COMPONENT_DEFINED
#include "tools/sk_app/Window.h"
#include "include/core/SkPoint.h"
#include "tools/SkMetaData.h"
#include "string"
#include "list"

class Click
{
public:
    Click() {}
    Click(std::function<bool(Click *)> f) : fFunc(f), fHasFunc(true) {}
    virtual ~Click() = default;

    SkPoint fOrig = {0, 0};
    SkPoint fPrev = {0, 0};
    SkPoint fCurr = {0, 0};
    skui::InputState fState = skui::InputState::kDown;
    skui::ModifierKey fModifierKeys = skui::ModifierKey::kNone;
    SkMetaData fMeta;

    std::function<bool(Click *)> fFunc;
    bool fHasFunc = false;
};

class DivComponent
{
public:
    DivComponent()
    {
        x = y = width = height = 0;
        paddingLeft = 0;
        fontSize = color = background = 0;
        innerText = "";
        onClick = nullptr;
    }
    SkScalar x;
    SkScalar y;
    SkScalar width;
    SkScalar height;
    SkScalar paddingLeft;
    std::string innerText;
    int32_t fontSize;
    SkColor color;
    SkColor background;
    std::function<bool(Click *)> onClick;
    std::list<DivComponent *> children;
};
#endif