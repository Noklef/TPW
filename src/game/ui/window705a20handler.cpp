#include "window705a20handler.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/vcall.h"
#include "window.h"
#include "window705a20.h"
#include "window705a20char.h"
#include "windowtimers.h"

namespace {

template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const Window705a20HandlerEnv kGameEnv = {&Window705a20_MoveCaret,  &Window_SetFocus,          &Fn_662097,          &WindowTimer_Start,   &Window705a20_SetInsertMode,
                                         &Window705a20_SelectAll,  &Window705a20_ToggleInsert, &Window705a20_KeyDown, &Window705a20_KeyToParent, &Window705a20_TypeChar,
                                         &Window_PostIfHit,        &Window_DefaultProc,       &Window705a20_SetSelection};
const Window705a20HandlerEnv *g_env = &kGameEnv;

u32 Distance(s32 v) { return v < 0 ? 0u - static_cast<u32>(v) : static_cast<u32>(v); }

// The place nearest the point, inside the window: the two handlers' common part.
s32 PlaceAt(u8 *window, u32 point) {
    const s32 last = static_cast<s32>(Crt().wcslen(at<const u16 *>(window, 0x138)));
    const s32 height = vcall<s16>(at<u8 *>(window, 0x4c), 4);
    const s32 y = static_cast<s32>((point & 0xffff) - at<s16>(window, 0x32)) + (height >> 1);
    const s32 x = (static_cast<s32>(point) >> 16) - at<s16>(window, 0x30);
    return Window705a20_NearestPlace(at<u8 *>(window, 0x9c), x, y, last);
}

}  // namespace

void Window705a20Handler_SetEnv(const Window705a20HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Window705a20_NearestPlace(u8 *list, s32 x, s32 y, s32 last) {
    s32 best = -1;
    if (!list) return best;
    u32 least = 0xffffffff;
    for (s32 i = 0; i <= last; ++i) {
        u8 *const place = vcall<u8 *>(list, 0x10, 1, i);
        const u32 distance = Distance(static_cast<s32>(static_cast<u32>(x) - static_cast<u32>(static_cast<s32>(at<s16>(place, 4))))) +
                             (Distance(static_cast<s32>(static_cast<u32>(y) - static_cast<u32>(static_cast<s32>(at<s16>(place, 6))))) << 14);
        if (distance < least) {
            least = distance;
            best = i;
        }
    }
    return best;
}

void __fastcall Window705a20_MouseDown(u8 *window, void *, u32 button, u32 point) {
    if (at<u8>(window, 0x161) || button != 0 || !at<u32>(window, 0x4c) || !at<u32>(window, 0x138) || !at<u32>(window, 0x9c)) return;
    if (!vcall<u32>(at<u8 *>(window, 0x9c), 0x10, 1, 0)) return;
    const s32 place = PlaceAt(window, point);
    if (place > -1) g_env->setSelection(window, nullptr, static_cast<u16>(place), 1, 0xffffffff);
}

void __fastcall Window705a20_MouseMove(u8 *window, void *, u32 point) {
    if (at<u8>(window, 0x161) || at<u8>(window, 0x162) || !(at<u32>(window, 0x11c) & 1) || !at<u32>(window, 0x138) || !at<u32>(window, 0x9c)) return;
    if (!vcall<u32>(at<u8 *>(window, 0x9c), 0x10, 1, 0)) return;
    const s32 place = PlaceAt(window, point);
    if (place > -1) g_env->setSelection(window, nullptr, 0xffffffff, 1, static_cast<u16>(place));
}

u32 __cdecl Window705a20_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 0x10:
            g_env->toggleInsert(window);
            break;
        case 0x13:
            g_env->moveCaret(window, nullptr, static_cast<u16>(a), static_cast<u16>(b));
            break;
        case 0x1a:
            g_env->setFocus(window, nullptr, 1);
            break;
        case 0x1f:
            if (at<u32>(window, 0x164)) g_env->timerStart(at<u8 *>(window, 0x164));
            g_env->insertMode(window, nullptr, 1);
            if (a == 0) g_env->selectAll(window);
            break;
        case 0x20:
            if (at<u32>(window, 0x164)) g_env->timerStop(at<u8 *>(window, 0x164));
            g_env->insertMode(window, nullptr, 0);
            break;
        case 0x10003:
            Window705a20_MouseMove(window, nullptr, a);
            break;
        case 0x10005:
            Window705a20_MouseDown(window, nullptr, a, b);
            break;
        case 0x10006:
            if (!Fn_668800(window)) {
                g_env->postIfHit(window, nullptr, message, a, b);
                g_env->setFocus(window, nullptr, 1);
            }
            break;
        case 0x10007:
            if (!Fn_668800(window)) {
                g_env->postIfHit(window, nullptr, message, a, b);
                g_env->selectAll(window);
            }
            break;
        case 0x1000a:
            g_env->keyDown(window, nullptr, static_cast<u16>(a), b);
            break;
        case 0x1000b:
            g_env->keyToParent(window, nullptr, static_cast<u16>(a), b);
            break;
        case 0x1000c:
            g_env->typeChar(window, nullptr, static_cast<u16>(a));
            break;
        default:
            break;
    }
    return g_env->defaultProc(window, message, a, b);
}
}
