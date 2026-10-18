#include "window.h"

#include <cstdint>

#include "../core/accessors.h"
#include "msgqueue.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

u8 *__fastcall Window_FindChild(u8 *window, void *, u32 id) {
    u8 *found = nullptr;
    for (u8 *child = at<u8 *>(window, 0x5c); child && !found; child = at<u8 *>(child, 0x54))
        if (Fn_65d7bc(child) == id) found = child;
    return found;
}

s32 __fastcall Window_CallHandler(u8 *window, void *, u32 message, u32 a, u32 b) {
    using Handler = s32(__cdecl *)(u8 *, u32, u32, u32);
    const Handler handler = at<Handler>(window, 0x114);
    return handler ? handler(window, message, a, b) : -1;
}

s32 __cdecl Window_SendMessage(u8 *window, u32 message, u32 a, u32 b) {
    using Hook = void(__cdecl *)(u8 *, u32, u32, u32);
    if (!window || Fn_659710(window)) return -1;
    if (const Hook hook = *reinterpret_cast<Hook *>((uintptr_t)0x00faa5fc)) hook(window, message, a, b);
    return Window_CallHandler(window, nullptr, message, a, b);
}

u32 __fastcall Window_ImplSlot7(u8 *window, void *, u32 a, u32 b) {
    u8 *const impl = at<u8 *>(window, 0x9c);
    if (!impl) return 0;
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32)>(at<void **>(impl, 0)[7])(impl, nullptr, a, b);
}

u32 __fastcall Window_ImplSlot6(u8 *window) {
    u8 *const impl = at<u8 *>(window, 0x9c);
    if (!impl) return (u32)(uintptr_t)window;  // sic
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *)>(at<void **>(impl, 0)[6])(impl, nullptr);
}

u32 __fastcall Window_DwordA0(u8 *window, void *, u32 index) {
    const s32 i = static_cast<s16>(index);
    return i < 16 ? at<u32>(window, 0xa0 + 4 * i) : 0;
}

u32 __cdecl Window_PostMessage(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const input = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f4);
    u8 *const queue = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f0);
    if (message == 1 || message == 3 || message == 2) {
        if (!input || !window || Fn_659710(window)) return 0;
        MsgQueue_Push(input, nullptr, (u32)(uintptr_t)window, message, a, b);
        return 1;
    }
    if (message == 0x1000a || message == 0x1000b || message == 0x1000c) {
        MsgQueue_Push(queue, nullptr, (u32)(uintptr_t)window, message, a, b);
        return 0;
    }
    if (!queue || !window || Fn_659710(window)) return 0;
    MsgQueue_Push(queue, nullptr, (u32)(uintptr_t)window, message, a, b);
    return 1;
}

u8 *__fastcall Window_HitTest(u8 *window, void *, u32 x, u32 y) {
    u8 *found = nullptr;
    if (at<u32>(window, 0x48) & 2 || at<s8>(window, 4)) return found;
    if (!(at<u32>(window, 0x48) & 1) && !(at<u32>(window, 0x48) & 0xf0000000)) return found;
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54))
        if (u8 *const hit = Window_HitTest(child, nullptr, (u16)x, (u16)y)) found = hit;
    u8 *const rect = at<u8 *>(window, 0x44);
    using Contains = u32(__fastcall *)(u8 *, void *, u32, u32);
    if ((s16)reinterpret_cast<Contains>(at<void **>(rect, 0)[0])(rect, nullptr, (u16)x, (u16)y) && !found) found = window;
    return found;
}

u32 __cdecl Window_UpdateHover() {
    const s16 x = *reinterpret_cast<s16 *>((uintptr_t)0x00faa540);
    const s16 y = *reinterpret_cast<s16 *>((uintptr_t)0x00faa544);
    u8 *const root = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5d8);
    if (!root || *reinterpret_cast<u32 *>((uintptr_t)0x00faa5e8)) return 0;
    u8 *const hit = Window_HitTest(root, nullptr, (u16)x, (u16)y);
    u8 *&hovered = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5e0);
    if (hit != hovered) {
        if (hovered) Window_PostMessage(hovered, 0x10002, 0, 0);
        hovered = hit;
        Window_PostMessage(hovered, 0x10001, (u32)(s32)x << 16 | (u32)(s32)y, 0);
    }
    return 1;
}
}
