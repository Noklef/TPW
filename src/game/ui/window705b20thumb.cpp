#include "window705b20thumb.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "window705a50.h"
#include "windowarea.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Window705b20ThumbEnv kGameEnv = {&Window_SendMessage,     &Window_PostMessage,      &Window_DefaultProc,    &Window_KeepRect,     &Window705b20_SetPos,
                                       &Window_Activate,        &Window705a50_SetFlag139, &Window705a50_SetState, &Window_CaptureMouse, &Window_ReleaseMouse,
                                       &Window705b20_NotifyScroll, &Window705b20_NotifyScrollEnd, &Window705b20_MoveInside, &Window_SetRect, &Window705b20_PosFromThumb};
const Window705b20ThumbEnv *g_env = &kGameEnv;

// The mouse object.
u8 *Mouse() { return *reinterpret_cast<u8 **>((uintptr_t)0x0077c48c); }

// A button's entry: held, then its press's x and y.
s16 &Button(u32 button, u32 offset) { return *reinterpret_cast<s16 *>((uintptr_t)(0x00faa598 + button * 12 + offset)); }

void Drag(u8 *window, u8 *bar, u32 a) {
    const Window705b20ThumbEnv *const E = g_env;
    u8 rect[12], bounds[12];
    GuiRect_Ctor(rect);
    GuiRect_Ctor(bounds);
    Window_GetRect20(window, nullptr, rect);
    WindowArea_GetRect2c(bar, nullptr, bounds);
    const s16 dx = static_cast<s16>(static_cast<s16>(a >> 16) - Button(0, 2));
    const s16 dy = static_cast<s16>(static_cast<s16>(a) - Button(0, 4));
    vcall<u32>(bar, 0x20);
    const u32 across = static_cast<s16>(Fn_66b454(bar)) & 2;
    GuiRectByValue copy;
    GuiRect_CopyCtor(copy.bytes, nullptr, bounds);
    E->moveInside(rect, copy, static_cast<s32>(dx), static_cast<s32>(dy), across);
    E->setRect(window, nullptr, rect);
    E->posFromThumb(bar);
    GuiRect_Dtor(bounds);
    GuiRect_Dtor(rect);
}

}  // namespace

extern "C" {

void Window705b20Thumb_SetEnv(const Window705b20ThumbEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __cdecl Window705b20Thumb_Proc(u8 *window, u32 message, u32 a, u32 b) {
    const Window705b20ThumbEnv *const E = g_env;
    u8 *const bar = reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_65d55a(window)));
    switch (message) {
        case 0xc:
            return static_cast<u32>(E->send(bar, message, a, b));
        case 0x13:
            if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) E->post(bar, 3, 0, 0);
            break;
        case 0x15:
            E->keepRect(window);
            E->setPos(bar, nullptr, -2, 0);
            break;
        case 0x10001:
            vcall<void>(Mouse(), 0x24, bar);
            if (at<u32>(window, 0x11c) == 0) E->setFlag139(window, nullptr, 1, 1);
            return 0;
        case 0x10002:
            vcall<void>(Mouse(), 0x28);
            if (at<u32>(window, 0x11c) == 0) E->setFlag139(window, nullptr, 0, 1);
            return 0;
        case 0x10003:
            vcall<void>(Mouse(), 0x20, static_cast<s32>(a) >> 16, a & 0xffff);
            if (at<u32>(window, 0x11c) & 1) Drag(window, bar, a);
            return 0;
        case 0x10004:
            E->setState(window, nullptr, 0, 1);
            at<u32>(window, 0x11c) &= ~(1u << (a & 0x1f));
            Button(a, 0) = 0;
            E->releaseMouse(window);
            E->setPos(bar, nullptr, -2, 0);
            E->keepRect(window);
            E->notifyScrollEnd(bar);
            return 0;
        case 0x10005:
            E->activate(window);
            if (Button(a, 0) == 0) {
                Button(a, 2) = static_cast<s16>(static_cast<s32>(b) >> 16);
                Button(a, 4) = static_cast<s16>(b);
                Button(a, 0) = 1;
                at<u32>(window, 0x11c) |= 1u << (a & 0x1f);
                E->setFlag139(window, nullptr, 0, 0);
                E->setState(window, nullptr, 1, 1);
                E->keepRect(window);
                E->captureMouse(window);
                E->notifyScroll(bar);
            }
            return 0;
        case 0x1000d: {
            const u32 size = vcall<u32>(bar, 0x20);
            vcall<void>(bar, 0x1c, size - a);
            break;
        }
        default:
            break;
    }
    return E->defaultProc(window, message, a, b);
}
}
