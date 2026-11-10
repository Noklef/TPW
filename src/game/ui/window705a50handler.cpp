#include "window705a50handler.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "cursor.h"
#include "guimanager.h"
#include "window.h"
#include "window705a50.h"
#include "guisettings.h"

namespace {

template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Window705a50HandlerEnv kGameEnv = {&Window705a50_Repeat,  &Window_Activate,          &GuiClock_Now,         &Window705a50_SetFlag139,
                                         &Window705a50_SetState, &Window705a50_StartRepeat, &Window705a50_StopRepeat, &Window_PostMessage,
                                         &Window_ApplyCursor,   &Window_DefaultProc};
const Window705a50HandlerEnv *g_env = &kGameEnv;

u8 *Mouse() { return global<u8 *>((u32)(uintptr_t)&gGuisettings_77c48c); }
s16 &ButtonState(s32 i) { return global<s16>(0x00faa598 + i * 12); }

}  // namespace

void Window705a50Handler_SetEnv(const Window705a50HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s16 __fastcall Window705a50_StateWord(const u8 *window) { return static_cast<s8>(window[0x138]); }

u32 __cdecl Window705a50_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 0x1e:
            if (at<u32>(window, 0x11c)) g_env->repeat(window);
            break;
        case 0x10001:
            g_env->applyCursor(window);
            vcall<void>(Mouse(), 0x24, window);
            if (at<u32>(window, 0x11c)) {
                const s32 state = Window705a50_StateWord(window);
                for (s32 i = 0; i < 3; ++i) {
                    if (ButtonState(i) == 2) ButtonState(i) = 1;
                }
                g_env->setState(window, nullptr, static_cast<u8>(state | 2), 1);
                g_env->post(window, 0x10008, Fn_65d7bc(window), a);
            } else {
                g_env->setFlag139(window, nullptr, 1, 1);
            }
            return 0;
        case 0x10002:
            vcall<void>(Mouse(), 0x28);
            if (at<u32>(window, 0x11c)) {
                const s32 state = Window705a50_StateWord(window);
                for (s32 i = 0; i < 3; ++i) {
                    if (ButtonState(i) == 1) ButtonState(i) = 2;
                }
                g_env->setState(window, nullptr, static_cast<u8>(state & 0xfd), 1);
            } else {
                g_env->setFlag139(window, nullptr, 0, 1);
            }
            return 0;
        case 0x10003:
            vcall<void>(Mouse(), 0x20, static_cast<s32>(a) >> 16, a & 0xffff);
            return 0;
        case 0x10004: {
            if (a != 0) break;
            const bool pressed = ButtonState(0) == 1 && static_cast<s16>(g_env->stopRepeat(window)) != 0;
            const s32 state = Window705a50_StateWord(window);
            if (Fn_65d57c(window) & 0x10) {
                if (state & 2) g_env->setState(window, nullptr, (state & 1) ? 0 : 1, 1);
                vcall<void>(Mouse(), 0x1c);
            } else {
                g_env->setState(window, nullptr, 0, 1);
            }
            at<u32>(window, 0x11c) &= ~1u;
            ButtonState(0) = 0;
            global<u32>(0x00faa5a0) = 0;
            g_env->setFlag139(window, nullptr, 1, 1);
            if (pressed) {
                const s32 now = Window705a50_StateWord(window);
                const u32 id = Fn_65d7bc(window);
                g_env->post(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(window)), 0x100, id, static_cast<u32>(now));
            }
            return 0;
        }
        case 0x10005:
            if (a != 0) break;
            g_env->activate(window);
            g_env->now();
            if (ButtonState(0) == 0) {
                global<u32>(0x00faa5a0) = g_env->now();
                global<s16>(0x00faa59a) = static_cast<s16>(static_cast<s32>(b) >> 16);
                global<u16>(0x00faa59c) = static_cast<u16>(b);
                ButtonState(0) = 1;
                at<u32>(window, 0x11c) |= 1;
                g_env->setFlag139(window, nullptr, 0, 0);
                g_env->setState(window, nullptr, static_cast<u8>(Window705a50_StateWord(window) | 2), 1);
                g_env->startRepeat(window);
            }
            return 0;
        default:
            break;
    }
    return g_env->defaultProc(window, message, a, b);
}
}
