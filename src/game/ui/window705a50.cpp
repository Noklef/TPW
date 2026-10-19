#include "window705a50.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "textpool.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

u16 __fastcall Window705a50_TextLength(const u8 *window) {
    const u16 *const text = *reinterpret_cast<const u16 *const *>(window + 0x138);
    return text ? (u16)Crt().wcslen(text) : 0;
}

u32 __fastcall Window705a50_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705a50;
    if (at<u8 *>(window, 0x148)) TextPool_Release(at<u8 *>(window, 0x148));
    return Window_Dtor(window);
}

u8 *__fastcall Window705a50_DeletingDtor(u8 *window, void *, u32 flags) {
    Window705a50_Dtor(window);
    if (flags & 1) GameHeap_Free(window);
    return window;
}

u16 __fastcall Window705a50_Look(const u8 *window) {
    const u8 *const w = window;
    if (*reinterpret_cast<const u32 *>(w + 0x48) & 2) return 1;
    if (w[0x139]) return (w[0x138] & 1) ? 3 : 2;
    if (w[0x138] & 2) return 4;
    if (w[0x138] & 1) return 5;
    return 0;
}

void __fastcall Window705a50_NotifyLook(u8 *window) {
    u8 *const listener = at<u8 *>(window, 0x9c);
    if (!listener) return;
    u8 inner[0x14], event[0x1c];  // (only inner's +0xa / +0xc / +0x10 and event's +0x10 / +0x18 set, per look)
    u32 code = 0;
    at<u8 *>(event, 0x10) = inner;
    switch (Window705a50_Look(window)) {
    case 4:
        at<s16>(inner, 0xa) = (s16)(at<s8>(window, 0x158) + 1);
        at<s16>(inner, 0xc) = at<s8>(window, 0x159);
        at<u32>(inner, 0x10) = at<u32>(window, 0x14c);
        code |= 0x4c000;
        break;
    case 3:
    case 5:
        at<s16>(inner, 0xa) = at<s8>(window, 0x158);
        at<s16>(inner, 0xc) = at<s8>(window, 0x159);
        at<u32>(inner, 0x10) = at<u32>(window, 0x154);
        code |= 0x4c000;
        break;
    case 0:
    case 2:
        at<s16>(inner, 0xa) = 0;
        at<s16>(inner, 0xc) = 0;
        at<u32>(inner, 0x10) = at<u32>(window, 0x14c);
        code |= 0x4c000;
        break;
    default:
        break;
    }
    at<u32>(event, 0x18) = code;
    vcall<void>(listener, 0xc, code, event);
}

u16 __fastcall Window705a50_SetState(u8 *window, void *, u8 state, u8 notify) {
    if (state == at<u8>(window, 0x138)) return 0;
    u8 event[0x1c];  // (only the look +4 and the dword +0x18 set)
    at<u32>(event, 0x18) = 1;
    at<u8>(window, 0x138) = state;
    if (notify && global<u32>(0x00faa5d4)) Window_PostMessage(window, 1, 0, 0);
    at<u16>(event, 4) = Window705a50_Look(window);
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 1u, event);
    Window705a50_NotifyLook(window);
    return 1;
}
}
