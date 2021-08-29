// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

// Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "tools/skui/ModifierKey.h"

#include "modules/skplaintexteditor/include/editor.h"

#include <fstream>
#include <memory>

using SkPlainTextEditor::Editor;
using SkPlainTextEditor::StringView;

#ifndef EDITOR
#define EDITOR
//搜索触发回调指针

static Editor::Movement convert(skui::Key key) {
  switch (key) {
  case skui::Key::kLeft:
    return Editor::Movement::kLeft;
  case skui::Key::kRight:
    return Editor::Movement::kRight;
  case skui::Key::kUp:
    return Editor::Movement::kUp;
  case skui::Key::kDown:
    return Editor::Movement::kDown;
  case skui::Key::kHome:
    return Editor::Movement::kHome;
  case skui::Key::kEnd:
    return Editor::Movement::kEnd;
  default:
    return Editor::Movement::kNowhere;
  }
}

struct Timer {
  double fTime;
  const char *fDesc;
  Timer(const char *desc = "") : fTime(SkTime::GetNSecs()), fDesc(desc) {}
  ~Timer() {
    SkDebugf("%s: %5d μs\n", fDesc, (int)((SkTime::GetNSecs() - fTime) * 1e-3));
  }
};

static constexpr float kFontSize = 18;
static const char *kTypefaces[3] = {"sans-serif", "serif", "monospace"};
static constexpr size_t kTypefaceCount = SK_ARRAY_COUNT(kTypefaces);

static constexpr SkFontStyle::Weight kFontWeight = SkFontStyle::kNormal_Weight;
static constexpr SkFontStyle::Width kFontWidth = SkFontStyle::kNormal_Width;
static constexpr SkFontStyle::Slant kFontSlant = SkFontStyle::kUpright_Slant;

struct EditorLayer : public sk_app::Window::Layer {
  SkString fPath;
  sk_app::Window *fParent = nullptr;
  // TODO(halcanary): implement a cross-platform clipboard interface.
  std::vector<char> fClipboard;
  Editor fEditor;
  Editor::TextPosition fTextPos{0, 0};
  Editor::TextPosition fMarkPos;
  int fPos = 0;    // window pixel position in file
  int fWidth = 0;  // window width
  int fHeight = 0; // window height
  int fMargin = 10;
  size_t fTypefaceIndex = 0;
  float fFontSize = kFontSize;
  bool fShiftDown = false;
  bool fBlink = false;
  bool fMouseDown = false;
  void (*onCharCallback)(char);
  void setCharCallback(void (*callback)(char)) {
    onCharCallback = callback;
  }

  void setFont() {
    fEditor.setFont(
        SkFont(SkTypeface::MakeFromName(
                   kTypefaces[fTypefaceIndex],
                   SkFontStyle(kFontWeight, kFontWidth, kFontSlant)),
               fFontSize));
  }

  void loadFile(const char *path) {
    if (sk_sp<SkData> data = SkData::MakeFromFileName(path)) {
      fPath = path;
      fEditor.insert(Editor::TextPosition{0, 0}, (const char *)data->data(),
                     data->size());
    } else {
      fPath = "output.txt";
    }
  }

  void onPaint(SkSurface *surface) override {
    SkCanvas *canvas = surface->getCanvas();
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect({0, 0, (float)fWidth, (float)fHeight});
    canvas->translate(fMargin, (float)(fMargin - fPos));
    Editor::PaintOpts options;
    options.fCursor = fTextPos;
    options.fCursorColor = {1, 0, 0, fBlink ? 0.0f : 1.0f};
    options.fBackgroundColor = SkColor4f{0.8f, 0.8f, 0.8f, 1};
    options.fCursorColor = {1, 0, 0, fBlink ? 0.0f : 1.0f};
    if (fMarkPos != Editor::TextPosition()) {
      options.fSelectionBegin = fMarkPos;
      options.fSelectionEnd = fTextPos;
    }
#ifdef SK_EDITOR_DEBUG_OUT
    {
      Timer timer("shaping");
      fEditor.paint(nullptr, options);
    }
    Timer timer("painting");
#endif // SK_EDITOR_DEBUG_OUT
    fEditor.paint(canvas, options);
  }

  void onResize(int width, int height) override {
    if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
      fHeight = height;
      if (width != fWidth) {
        fWidth = width;
        fEditor.setWidth(fWidth - 2 * fMargin);
      }
      this->inval();
    }
  }

  void onAttach(sk_app::Window *w) override { fParent = w; }

  bool scroll(int delta) {
    int maxPos = std::max(0, fEditor.getHeight() + 2 * fMargin - fHeight / 2);
    int newpos = std::max(0, std::min(fPos + delta, maxPos));
    if (newpos != fPos) {
      fPos = newpos;
      this->inval();
    }
    return true;
  }

  void inval() {
    if (fParent) {
      fParent->inval();
    }
  }

  bool onMouseWheel(float delta, skui::ModifierKey) override {
    this->scroll(-(int)(delta * fEditor.font().getSpacing()));
    return true;
  }

  bool onMouse(int x, int y, skui::InputState state,
               skui::ModifierKey modifiers) override {
    bool mouseDown = skui::InputState::kDown == state;
    if (mouseDown) {
      fMouseDown = true;
    } else if (skui::InputState::kUp == state) {
      fMouseDown = false;
    }
    bool shiftOrDrag =
        sknonstd::Any(modifiers & skui::ModifierKey::kShift) || !mouseDown;
    if (fMouseDown) {
      return this->move(fEditor.getPosition({x - fMargin, y + fPos - fMargin}),
                        shiftOrDrag);
    }
    return false;
  }

  bool onChar(SkUnichar c, skui::ModifierKey modi) override {
    using sknonstd::Any;
    modi &= ~skui::ModifierKey::kFirstPress;
    if (!Any(modi & (skui::ModifierKey::kControl | skui::ModifierKey::kOption |
                     skui::ModifierKey::kCommand))) {
      if(c=='\n') onCharCallback(c);
      if (((unsigned)c < 0x7F && (unsigned)c >= 0x20)) {
        char ch = (char)c;
        onCharCallback(ch);
        fEditor.insert(fTextPos, &ch, 1);

#ifdef SK_EDITOR_DEBUG_OUT
        SkDebugf("insert: %X'%c'\n", (unsigned)c, ch);
#endif // SK_EDITOR_DEBUG_OUT
        return this->moveCursor(Editor::Movement::kRight);
      }
    }
    static constexpr skui::ModifierKey kCommandOrControl =
        skui::ModifierKey::kCommand | skui::ModifierKey::kControl;
    if (Any(modi & kCommandOrControl) && !Any(modi & ~kCommandOrControl)) {
      switch (c) {
      case 'p':
        for (StringView str : fEditor.text()) {
          SkDebugf(">>  '%.*s'\n", (int)str.size, str.data);
        }
        return true;
      case 's': {
        std::ofstream out(fPath.c_str());
        size_t count = fEditor.lineCount();
        for (size_t i = 0; i < count; ++i) {
          if (i != 0) {
            out << '\n';
          }
          StringView str = fEditor.line(i);
          out.write(str.data, str.size);
        }
      }
        return true;
      case 'c':
        if (fMarkPos != Editor::TextPosition()) {
          fClipboard.resize(fEditor.copy(fMarkPos, fTextPos, nullptr));
          fEditor.copy(fMarkPos, fTextPos, fClipboard.data());
          return true;
        }
        return false;
      case 'x':
        if (fMarkPos != Editor::TextPosition()) {
          fClipboard.resize(fEditor.copy(fMarkPos, fTextPos, nullptr));
          fEditor.copy(fMarkPos, fTextPos, fClipboard.data());
          (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
          this->inval();
          return true;
        }
        return false;
      case 'v':
        if (fClipboard.size()) {
          fEditor.insert(fTextPos, fClipboard.data(), fClipboard.size());
          this->inval();
          return true;
        }
        return false;
      case '0':
        fTypefaceIndex = (fTypefaceIndex + 1) % kTypefaceCount;
        this->setFont();
        return true;
      case '=':
      case '+':
        fFontSize = fFontSize + 1;
        this->setFont();
        return true;
      case '-':
      case '_':
        if (fFontSize > 1) {
          fFontSize = fFontSize - 1;
          this->setFont();
        }
      }
    }
#ifdef SK_EDITOR_DEBUG_OUT
    debug_on_char(c, modifiers);
#endif // SK_EDITOR_DEBUG_OUT
    return false;
  }

  bool moveCursor(Editor::Movement m, bool shift = false) {
    return this->move(fEditor.move(m, fTextPos), shift);
  }

  bool move(Editor::TextPosition pos, bool shift) {
    if (pos == fTextPos || pos == Editor::TextPosition()) {
      if (!shift) {
        fMarkPos = Editor::TextPosition();
      }
      return false;
    }
    if (shift != fShiftDown) {
      fMarkPos = shift ? fTextPos : Editor::TextPosition();
      fShiftDown = shift;
    }
    fTextPos = pos;

    // scroll if needed.
    SkIRect cursor = fEditor.getLocation(fTextPos).roundOut();
    if (fPos < cursor.bottom() - fHeight + 2 * fMargin) {
      fPos = cursor.bottom() - fHeight + 2 * fMargin;
    } else if (cursor.top() < fPos) {
      fPos = cursor.top();
    }
    this->inval();
    return true;
  }

  bool onKey(skui::Key key, skui::InputState state,
             skui::ModifierKey modifiers) override {
    if (state != skui::InputState::kDown) {
      return false; // ignore keyup
    }
    // ignore other modifiers.
    using sknonstd::Any;
    skui::ModifierKey ctrlAltCmd =
        modifiers & (skui::ModifierKey::kControl | skui::ModifierKey::kOption |
                     skui::ModifierKey::kCommand);
    bool shift = Any(modifiers & (skui::ModifierKey::kShift));
    if (!Any(ctrlAltCmd)) {
      // no modifiers
      switch (key) {
      case skui::Key::kPageDown:
        return this->scroll(fHeight * 4 / 5);
      case skui::Key::kPageUp:
        return this->scroll(-fHeight * 4 / 5);
      case skui::Key::kLeft:
      case skui::Key::kRight:
      case skui::Key::kUp:
      case skui::Key::kDown:
      case skui::Key::kHome:
      case skui::Key::kEnd:
        return this->moveCursor(convert(key), shift);
      case skui::Key::kDelete:
        if (fMarkPos != Editor::TextPosition()) {
          (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
        } else {
          auto pos = fEditor.move(Editor::Movement::kRight, fTextPos);
          (void)this->move(fEditor.remove(fTextPos, pos), false);
        }
        this->inval();
        return true;
      case skui::Key::kBack:
        if (fMarkPos != Editor::TextPosition()) {
          (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
        } else {
          auto pos = fEditor.move(Editor::Movement::kLeft, fTextPos);
          (void)this->move(fEditor.remove(fTextPos, pos), false);
        }
        this->inval();
        return true;
      case skui::Key::kOK:
        return this->onChar('\n', modifiers);
      default:
        break;
      }
    } else if (sknonstd::Any(ctrlAltCmd & (skui::ModifierKey::kControl |
                                           skui::ModifierKey::kCommand))) {
      switch (key) {
      case skui::Key::kLeft:
        return this->moveCursor(Editor::Movement::kWordLeft, shift);
      case skui::Key::kRight:
        return this->moveCursor(Editor::Movement::kWordRight, shift);
      default:
        break;
      }
    }
#ifdef SK_EDITOR_DEBUG_OUT
    debug_on_key(key, state, modifiers);
#endif // SK_EDITOR_DEBUG_OUT
    return false;
  }
};
#endif