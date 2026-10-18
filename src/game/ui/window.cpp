#include "window.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "guitimers.h"
#include "msgqueue.h"
#include "windowtimers.h"

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

s32 __fastcall Window_CallDefault(u8 *window, void *, u32 message, u32 a, u32 b) {
    using Handler = s32(__cdecl *)(u8 *, u32, u32, u32);
    return at<Handler>(window, 0x118)(window, message, a, b);
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

s32 __fastcall WindowImpl_Release(u8 *impl) {
    at<s16>(impl, 0x12) = (s16)(at<s16>(impl, 0x12) - 1);
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(at<void **>(impl, 0)[5])(impl, nullptr, 0);
    return (s32)at<s16>(impl, 0x12) - (1 - (s32)at<s16>(impl, 0x10));
}

u16 __fastcall Window_IsShown(u8 *window) {
    u16 shown = 1;
    for (u8 *w = window; w && shown; w = at<u8 *>(w, 0x60))
        if (!(at<u32>(w, 0x48) & 1)) shown = 0;
    return shown;
}

u8 *__fastcall Window_SaveRect(u8 *window) { return GuiRect_Assign(window + 0x2c, nullptr, window + 8); }

u16 __fastcall Window_GetRect(u8 *window, void *, u8 *rect, u32 inner) {
    if (!rect) return 0;
    GuiRect_Assign(rect, nullptr, window + 8);
    if ((s16)inner != 0)
        vcall<void>(rect, 0xc, -(s32)at<s16>(window, 0xf8), -(s32)at<s16>(window, 0xfa), -(s32)at<s16>(window, 0xfc),
                    -(s32)at<s16>(window, 0xfe));
    return 1;
}

u16 __fastcall Window_SetSavedRect(u8 *window, void *, const u8 *rect) {
    if (!rect) return 0;
    GuiRect_Assign(window + 0x2c, nullptr, rect);
    return 1;
}

u8 *__fastcall Window_OffsetByParents(u8 *window, void *, u8 *rect) {
    u8 *previous = window;  // the pushed words' upper halves: dy's from EAX (the window, then the last parent), dx's from the parent
    for (u8 *p = at<u8 *>(window, 0x60); p; p = at<u8 *>(p, 0x60)) {
        const u32 dy = ((u32)(uintptr_t)previous & 0xffff0000u) | at<u16>(p, 0xe);  // sic
        const u32 dx = ((u32)(uintptr_t)p & 0xffff0000u) | at<u16>(p, 0xc);         // sic
        reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>(at<void **>(rect, 0)[2])(rect, nullptr, dx, dy);
        previous = p;
    }
    return previous;
}

u8 *__fastcall Window_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    for (u32 off = 8; off <= 0x38; off += 0xc)
        GuiRect_Ctor(window + off);
    at<u32>(window, 0) = 0x705980;
    window[4] = 0;
    at<u32>(window, 0x48) = flags;
    at<u32>(window, 0x114) = 0x65f6d1;
    at<u32>(window, 0x118) = 0x65f6d1;
    at<u8 *>(window, 0x60) = parent;
    for (const u32 off : {0x54u, 0x5cu, 0x58u, 0x64u, 0x68u, 0xe8u})
        at<u32>(window, off) = 0;
    window[0x135] = 0;
    for (const u32 off : {0xecu, 0xfeu, 0xfcu, 0xfau, 0xf8u})
        at<u16>(window, off) = 0;
    at<u32>(window, 0x100) = 0;
    at<s32>(window, 0x90) = -1;
    at<s32>(window, 0x94) = -1;
    for (u32 i = 0; i < 4; ++i)
        at<u32>(window, 0x120 + 4 * i) = 0;
    window[0x134] = 0;
    at<u32>(window, 0x130) = 0;
    at<u16>(window, 0x50) = 0xffff;
    at<u32>(window, 0x8c) = id;
    at<u32>(window, 0x11c) = 0;
    at<u16>(window, 0xc) = (u16)left;
    at<u16>(window, 0xe) = (u16)top;
    at<u16>(window, 0x10) = (u16)right;
    at<u16>(window, 0x12) = (u16)bottom;
    if ((flags & 0x400) == 0x400) Window_OffsetByParents(window, nullptr, window + 8);
    Window_SaveRect(window);
    *reinterpret_cast<u8 **>((uintptr_t)0x00faa5e4) = window;
    at<u32>(window, 0x4c) = 0;
    at<u32>(window, 0x98) = 0;
    at<u32>(window, 0x9c) = 0;
    at<u8 *>(window, 0x44) = window + 8;
    GuiRect_Assign(window + 0x14, nullptr, window + 8);
    if (parent) {
        at<s16>(window, 0xe0) = (s16)(at<s16>(parent, 0xe0) + 2);
        at<u16>(window, 0xe4) = at<u16>(parent, 0xe4);
        at<u16>(window, 0xe6) = at<u16>(parent, 0xe6);
    } else {
        at<s16>(window, 0xe0) = 1;
        at<u16>(window, 0xe4) = (u16)left;    // sic: both overwritten below
        at<u16>(window, 0xe6) = (u16)bottom;
    }
    at<u16>(window, 0xe4) = (u16)((s16)left + (((s32)(s16)right - (s16)left) >> 1));
    at<u16>(window, 0xe6) = (u16)((s16)top + (((s32)(s16)bottom - (s16)top) >> 1));
    if (at<s16>(window, 0xe0) > 0x100) at<s16>(window, 0xe0) = 0x100;
    at<u16>(window, 0xe2) = 0;
    std::memset(window + 0xa0, 0, 0x40);
    if (flags & 1) Window_UpdateHover();
    Window_PostMessage(window, 0x15, 0, 0);
    return window;
}

u32 __cdecl Window_ForgetGlobals(u8 *window) {
    for (const uintptr_t global : {0x00faa5e0u, 0x00faa5dcu, 0x00faa5e8u})
        if (*reinterpret_cast<u8 **>(global) == window) *reinterpret_cast<u8 **>(global) = nullptr;
    for (u32 i = 0; i < 3; ++i)
        if (reinterpret_cast<u8 **>((uintptr_t)0x00faa588)[i] == window) reinterpret_cast<u8 **>((uintptr_t)0x00faa588)[i] = nullptr;
    return 3;  // sic: EAX is left as the loop's count
}

u32 __fastcall Window_DropModal(u8 *window) {
    u8 *&modal = *reinterpret_cast<u8 **>((uintptr_t)0x00faa57c);
    const u32 high = (u32)(uintptr_t)modal & 0xffff0000u;  // sic: only AX is set, EAX held the modal window
    if (modal != window) return high;
    modal = nullptr;
    *reinterpret_cast<u32 *>((uintptr_t)0x00faa578) = 0;
    return high | 1;
}

u32 __fastcall Window_Destroying(u8 *window) {
    Window_SendMessage(window, 0x14, 0, 0);
    window[4] = 1;
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54))
        Window_Destroying(child);
    Window_DropModal(window);
    u8 *&focus = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5ec);
    u8 *const old = focus;
    if (focus == window) focus = nullptr;
    u8 *&other = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5dc);
    if (other == window) other = nullptr;
    return (u32)(uintptr_t)old;  // EAX is left as [0xfaa5ec] as read
}

u32 __cdecl Window_CancelPosted(u8 *window) {
    u8 *const queue = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f0);
    if (!queue || !window) return 0xffff;  // only AX is set
    return MsgQueue_Cancel(queue, nullptr, (u32)(uintptr_t)window);
}

u32 __cdecl Window_CancelPostedFirst(u8 *window) {
    u8 *const queue = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f4);
    if (!queue || !window) return 0xffff;  // only AX is set
    return MsgQueue_Cancel(queue, nullptr, (u32)(uintptr_t)window);
}

u32 __cdecl Window_CancelTimed(u8 *window) {
    u8 *const table = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f8);
    if (!table || !window) return 0xffff;  // only AX is set
    return GuiTimers_Cancel(table, nullptr, (u32)(uintptr_t)window);
}

u32 __fastcall Window_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705980;
    if (!window[4]) Window_Destroying(window);
    u8 *const manager = *reinterpret_cast<u8 **>((uintptr_t)0x0077c48c);
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *)>(at<void **>(manager, 0)[13])(manager, nullptr, window);
    for (const u32 off : {0x98u, 0x9cu}) {
        if (!at<u8 *>(window, off) || (s16)WindowImpl_Release(at<u8 *>(window, off)) != -1) continue;
        u8 *const impl = at<u8 *>(window, off);  // read again
        if (impl) reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(at<void **>(impl, 0)[0])(impl, nullptr, 1);
    }
    for (u32 i = 0; i < 4; ++i)
        if (at<u32>(window, 0x120 + 4 * i)) WindowTimers_Kill((u32)(uintptr_t)window, 0x80080 + i);
    u8 *const rect = at<u8 *>(window, 0x44);
    if (rect && rect != window + 8) reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(at<void **>(rect, 0)[4])(rect, nullptr, 1);
    Window_ForgetGlobals(window);
    Window_CancelPosted(window);
    Window_CancelPostedFirst(window);
    Window_CancelTimed(window);
    if (at<u8 *>(window, 0x60) && !at<u8 *>(window, 0x58)) at<u8 *>(at<u8 *>(window, 0x60), 0x5c) = at<u8 *>(window, 0x54);
    if (at<u8 *>(window, 0x58)) at<u8 *>(at<u8 *>(window, 0x58), 0x54) = at<u8 *>(window, 0x54);
    if (at<u8 *>(window, 0x54)) at<u8 *>(at<u8 *>(window, 0x54), 0x58) = at<u8 *>(window, 0x58);
    at<u8 *>(window, 0x60) = nullptr;
    while (u8 *const child = at<u8 *>(window, 0x5c))  // each child's deleting destructor unlinks it
        reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(at<void **>(child, 0)[2])(child, nullptr, 1);
    Window_UpdateHover();
    for (u32 off = 0x38; off > 8; off -= 0xc)
        GuiRect_Dtor(window + off);
    return GuiRect_Dtor(window + 8);
}

u8 *__fastcall Window_DeletingDtor(u8 *window, void *, u32 flags) {
    Window_Dtor(window);
    if (flags & 1) GameHeap_Free(window);
    return window;
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

void __cdecl Window_SaveInsetRect(u8 *window, s32 dx, s32 dy) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect(window, nullptr, rect, 1);
    GuiRect_Adjust(rect, nullptr, (u32)dx, (u32)dy, (u32)-dx, (u32)-dy);
    Window_SetSavedRect(window, nullptr, rect);
    GuiRect_Dtor(rect);
}
}
