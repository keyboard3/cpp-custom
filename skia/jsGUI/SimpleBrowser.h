/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SimpleBrowser_DEFINED
#define SimpleBrowser_DEFINED

#include "component.h"
#include "include/core/SkPoint.h"
#include "tools/SkMetaData.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
class SkCanvas;

class SimpleBrowser : public sk_app::Application, sk_app::Window::Layer {
public:
  SimpleBrowser(int argc, char **argv, void *platformData);
  ~SimpleBrowser() override;

  void onIdle() override;
  virtual bool onMouse(int x, int y, skui::InputState,
                       skui::ModifierKey) override;
  void onBackendCreated() override;
  void onPaint(SkSurface *) override;
  bool onChar(SkUnichar c, skui::ModifierKey modifiers) override;
  DivComponent *root;

private:
  JSRuntime *rt;
  JSContext *ctx;
  void updateTitle();

  sk_app::Window *fWindow;
  sk_app::Window::BackendType fBackendType;

  std::unique_ptr<Click> fClick;
  virtual Click *onFindClickHandler(SkScalar x, SkScalar y,
                                    skui::ModifierKey modi);
  virtual bool onClick(Click *);
  virtual bool onComponentClick(Click *c);
  virtual void initJsPage();
  virtual void initJsEngine();
};
#endif
