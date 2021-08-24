#ifndef COMPONENT_DEFINED
#define COMPONENT_DEFINED
#include "include/core/SkPoint.h"
#include "vector"
#include "quickjs/quickjs.h"
#include "string"
#include "tools/SkMetaData.h"
#include "tools/sk_app/Window.h"

class Click {
public:
  Click() {}
  Click(JSValue fthis, JSValue f) : fthis(fthis), fFunc(f), fHasFunc(true) {}
  virtual ~Click() = default;

  SkPoint fOrig = {0, 0};
  SkPoint fPrev = {0, 0};
  SkPoint fCurr = {0, 0};
  skui::InputState fState = skui::InputState::kDown;
  skui::ModifierKey fModifierKeys = skui::ModifierKey::kNone;
  SkMetaData fMeta;

  JSValue fthis;
  JSValue fFunc;
  bool fHasFunc = false;
};

class DivComponent {
public:
  DivComponent() {
    x = y = width = height = 0;
    paddingLeft = 0;
    fontSize = color = background = 0;
    innerText = "";
    clickThis = JS_UNDEFINED;
    onClick = JS_UNDEFINED;
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
  JSValue clickThis;
  JSValue onClick;
  std::vector<DivComponent *> children;
};
#endif