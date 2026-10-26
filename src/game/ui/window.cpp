#include "window.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "cursor.h"
#include "guimanager.h"
#include "guirect.h"
#include "guitimers.h"
#include "msgqueue.h"
#include "windowtimers.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Slot 5 (thiscall, one argument) of `object`; returns the ECX the slot left (Window_NotifyShown pushes its upper half next).
u32 Slot5LeavingEcx(u8 *object, u32 arg) {
    u32 left;
    __asm {
        push arg
        mov ecx, object
        mov eax, [ecx]
        call dword ptr [eax + 0x14]
        mov left, ecx
    }
    return left;
}

// a / divisor by IDIV, so that a divisor of 0 (or -0x80000000 / -1) faults as the original's does.
s32 Idiv(s32 a, s32 divisor) {
    s32 q;
    __asm {
        mov eax, a
        cdq
        idiv divisor
        mov q, eax
    }
    return q;
}
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

void __fastcall Window_NotifyShown(u8 *window, void *edx, u32 shown) {
    const s16 s = (s16)shown;
    u32 dx = (u32)(uintptr_t)edx;
    if (at<u32>(window, 0x48) & 1) dx = (u32)(s32)s;
    else if (s != 0) return;
    u32 cx = (u32)(uintptr_t)window;
    if (u8 *const object = at<u8 *>(window, 0x98)) cx = Slot5LeavingEcx(object, (dx & 0xffff0000u) | (u16)s);  // sic
    if (u8 *const object = at<u8 *>(window, 0x9c)) vcall<void>(object, 0x14, (cx & 0xffff0000u) | (u16)s);   // sic
    using Handler = s32(__cdecl *)(u8 *, u32, s32, u32);
    if (const Handler handler = at<Handler>(window, 0x114)) handler(window, 0x11, s, 0);
    u8 *previous = window;
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) {
        Window_NotifyShown(child, previous, (u16)s);
        previous = child;
    }
}

u32 __fastcall Window_SetShown(u8 *window, void *, u32 shown) {
    const u16 s = (s16)shown != 0 ? 1 : 0;
    if (s == (at<u32>(window, 0x48) & 1)) return 1;
    if (s == 0) at<u32>(window, 0x48) &= ~1u;
    else at<u32>(window, 0x48) |= 1;
    if (s == 0 || Window_IsShown(window) != 0)
        Window_NotifyShown(window, reinterpret_cast<void *>(((uintptr_t)window & 0xffff0000u) | s), s);
    Window_UpdateHover();
    if (s != 0) return 1;
    u8 *const manager = *reinterpret_cast<u8 **>((uintptr_t)0x0077c48c);
    return (vcall<u32>(manager, 0x34, window) & 0xffff0000u) | 1;
}

u32 __fastcall Window_SetEnabled(u8 *window, void *, u32 enabled) {
    const s16 e = (s16)enabled;
    if (e != 0) {
        if (!(at<u32>(window, 0x48) & 2)) return 0;
        at<u32>(window, 0x48) &= ~2u;
    } else {
        if (at<u32>(window, 0x48) & 2) return 0;
        at<u32>(window, 0x48) |= 2;
    }
    using Handler = s32(__cdecl *)(u8 *, u32, s32, u32);
    if (const Handler handler = at<Handler>(window, 0x114)) handler(window, 0x17, e, 0);
    Window_UpdateHover();
    return 1;
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

u8 *__fastcall Window_SetHitArea(u8 *window, void *, u8 *area) {
    u8 *const old = at<u8 *>(window, 0x44);
    if (old && old != window + 8) vcall<void>(old, 0x10, 1u);
    at<u8 *>(window, 0x44) = area ? area : window + 8;
    return window;
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

u32 __fastcall Window_Offset(u8 *window, void *, u32 dx, u32 dy) {
    const s16 x = (s16)dx, y = (s16)dy;
    if (x == 0 && y == 0) return 0;
    using Offset = void(__fastcall *)(u8 *, void *, u32, u32);
    const auto offset = [&](u8 *rect) { reinterpret_cast<Offset>(at<void **>(rect, 0)[2])(rect, nullptr, (u16)x, (u16)y); };
    offset(window + 8);
    if (at<u8 *>(window, 0x44) != window + 8) offset(at<u8 *>(window, 0x44));
    offset(window + 0x2c);
    at<u16>(window, 0xe4) += (u16)x;
    at<u16>(window, 0xe6) += (u16)y;
    u8 event[0x18];  // (the rest is left as it is)
    at<u16>(event, 0xc) = at<u16>(window, 0xe4);
    at<u16>(event, 0xe) = at<u16>(window, 0xe6);
    if (u8 *const object = at<u8 *>(window, 0x98)) {
        at<u8 *>(event, 0x14) = window + 8;
        vcall<void>(object, 0xc, 0x50u, event);
    }
    if (u8 *const object = at<u8 *>(window, 0x9c)) {
        at<u8 *>(event, 0x14) = window + 0x2c;
        vcall<void>(object, 0xc, 0x50u, event);
    }
    Window_SendMessage(window, 0x13, (u32)(s32)x, (u32)(s32)y);
    u32 left = (u32)(uintptr_t)window;  // EAX: the window, or the last child's result
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) left = Window_Offset(child, nullptr, (u16)x, (u16)y);
    return (left & 0xffff0000u) | 1;
}

void __fastcall Window_Set4c(u8 *window, void *, u8 *object) {
    if (object == at<u8 *>(window, 0x4c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's byte +8 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    at<u8 *>(window, 0x4c) = object;
    inner[8] = object ? at<u8>(object, 0x10) : 0xff;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x10000u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x10000u, event);
}

void __fastcall Window_NotifyC00(u8 *window, void *, u32 a, u32 b) {
    if (!at<u8 *>(window, 0x98) && !at<u8 *>(window, 0x9c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's bytes +4 / +5 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    inner[4] = (u8)a;
    inner[5] = (u8)b;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0xc00u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0xc00u, event);
}

void __fastcall Window_Notify3000(u8 *window, void *, u32 a, u32 b) {
    if (!at<u8 *>(window, 0x98) && !at<u8 *>(window, 0x9c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's bytes +6 / +7 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    inner[6] = (u8)a;
    inner[7] = (u8)b;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x3000u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x3000u, event);
}

void __fastcall Window_NotifyC000(u8 *window, void *, u32 a, u32 b) {
    if (!at<u8 *>(window, 0x98) && !at<u8 *>(window, 0x9c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's words +0xa / +0xc and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    at<u16>(inner, 0xa) = (u16)a;
    at<u16>(inner, 0xc) = (u16)b;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0xc000u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0xc000u, event);
}

void __fastcall Window_Notify20000(u8 *window, void *, u32 a) {
    if (!at<u8 *>(window, 0x98) && !at<u8 *>(window, 0x9c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's byte +9 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    inner[9] = (u8)a;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x20000u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x20000u, event);
}

void __fastcall Window_Notify40000(u8 *window, void *, u32 a, u32 b, u32 c, u32 d) {
    if (!at<u8 *>(window, 0x98) && !at<u8 *>(window, 0x9c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's bytes +0x10-+0x13 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    inner[0x10] = (u8)a;
    inner[0x11] = (u8)b;
    inner[0x12] = (u8)c;
    inner[0x13] = (u8)d;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x40000u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x40000u, event);
}

void __fastcall Window_NotifyImpl100(u8 *window, void *, u32 a) {
    if (!at<u8 *>(window, 0x9c)) return;
    u8 inner[0x14], event[0x18];  // (only inner's dword +0 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    at<u32>(inner, 0) = a;
    vcall<void>(at<u8 *>(window, 0x9c), 0xc, 0x100u, event);
}

void __fastcall Window_NotifyObject98_100(u8 *window, void *, u32 a) {
    if (!at<u8 *>(window, 0x98)) return;
    u8 inner[0x14], event[0x18];  // (only inner's dword +0 and event's pointer +0x10 set)
    at<u8 *>(event, 0x10) = inner;
    at<u32>(inner, 0) = a;
    vcall<void>(at<u8 *>(window, 0x98), 0xc, 0x100u, event);
}

void __fastcall Window_SetObject98(u8 *window, void *, u8 *object) {
    if (object == at<u8 *>(window, 0x98)) return;
    if (u8 *const old = at<u8 *>(window, 0x98)) {
        if ((s16)WindowImpl_Release(old) == -1) vcall<u32>(old, 0, 1u);
    }
    at<u8 *>(window, 0x98) = object;
    if (!at<u8 *>(window, 0x98)) return;
    Fn_6617f0(at<u8 *>(window, 0x98));
    u8 inner[0x14], event[0x1c];  // (event's dword +0, its bytes +0xa / +0xb and inner but its byte +8 left as they are)
    at<u16>(event, 4) = 0;
    at<u16>(event, 6) = at<u16>(window, 0xe0);
    at<u16>(event, 8) = at<u16>(window, 0xe2);
    at<u8 *>(event, 0x14) = window + 8;
    at<u16>(event, 0xc) = at<u16>(window, 0xe4);
    at<u16>(event, 0xe) = at<u16>(window, 0xe6);
    u32 flags = 0x57;
    if (at<u8 *>(window, 0x4c)) {   // (else the pointer +0x10 is left as it is too)
        flags |= 0x10000;
        inner[8] = at<u8>(at<u8 *>(window, 0x4c), 0x10);
        at<u8 *>(event, 0x10) = inner;
    }
    at<u32>(event, 0x18) = flags;
    vcall<void>(at<u8 *>(window, 0x98), 0xc, flags, event);
    if ((s16)Window_IsShown(window) != 0) vcall<void>(at<u8 *>(window, 0x98), 0x14, 1u);
}

void __fastcall Window_SetImpl(u8 *window, void *, u8 *impl) {
    if (impl == at<u8 *>(window, 0x9c)) return;
    if (at<u8 *>(window, 0x9c) && (s16)WindowImpl_Release(at<u8 *>(window, 0x9c)) == -1) {
        if (u8 *const o = at<u8 *>(window, 0x98)) vcall<u32>(o, 0, 1u);   // sic: +0x98's slot 0, not the released one's
    }
    at<u8 *>(window, 0x9c) = impl;
    if (!at<u8 *>(window, 0x9c)) return;
    Fn_6617f0(at<u8 *>(window, 0x9c));
    u8 inner[0x14], event[0x1c];  // (event's dword +0, its bytes +0xa / +0xb and inner but its byte +8 left as they are)
    at<u16>(event, 4) = 0;
    at<u16>(event, 6) = (u16)(at<s16>(window, 0xe0) + 1);
    at<u16>(event, 8) = at<u16>(window, 0xe2);
    at<u8 *>(event, 0x14) = window + 0x2c;
    at<u16>(event, 0xc) = at<u16>(window, 0xe4);
    at<u16>(event, 0xe) = at<u16>(window, 0xe6);
    u32 flags = 0x57;
    if (at<u8 *>(window, 0x4c)) {   // (else the pointer +0x10 is left as it is too)
        flags |= 0x10000;
        inner[8] = at<u8>(at<u8 *>(window, 0x4c), 0x10);
        at<u8 *>(event, 0x10) = inner;
    }
    at<u32>(event, 0x18) = flags;
    vcall<void>(at<u8 *>(window, 0x9c), 0xc, flags, event);
    if ((s16)Window_IsShown(window) != 0) vcall<void>(at<u8 *>(window, 0x9c), 0x14, 1u);
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

u32 __cdecl Window_PostTimed(u8 *window, u32 message, u32 a, u32 b, u32 delay) {
    u8 *const table = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f8);
    if (!table || !window || message == 1 || message == 3 || message == 2) return 0;
    GuiTimers_Add(table, nullptr, (u32)(uintptr_t)window, message, a, b, delay);
    return 1;
}

void __cdecl Gui_ResetQueues() {
    if (u8 *const table = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f8)) GuiTimers_Reset(table);
    if (u8 *const first = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f4)) MsgQueue_Clear(first);
    if (u8 *const queue = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5f0)) MsgQueue_Clear(queue);
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

u32 __fastcall Window_SetDwordA0(u8 *window, void *, u32 index, u32 value) {
    const s32 i = static_cast<s16>(index);
    if (i >= 16) return 0;
    at<u32>(window, 0xa0 + 4 * i) = value;  // sic: a negative index writes before +0xa0
    return (value & 0xffff0000u) | 1;
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

u32 __fastcall Window_SetFocus(u8 *window, void *, u32 a) {
    u8 *&focus = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5ec);
    if (focus == window) return (u32)(uintptr_t)focus & 0xffff0000u;  // only AX is set
    if (focus) Window_PostMessage(focus, 0x20, a, 0);
    focus = window;
    return (Window_PostMessage(focus, 0x1f, a, 0) & 0xffff0000u) | 1;
}

u32 __fastcall Window_KillFocus(u8 *window, void *, u32 a) {
    u8 *&focus = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5ec);
    if (focus != window) return (u32)(uintptr_t)focus & 0xffff0000u;  // only AX is set
    Window_PostMessage(focus, 0x20, a, 0);
    focus = nullptr;
    return (Window_UpdateHover() & 0xffff0000u) | 1;
}

u32 __cdecl Gui_MouseDown(u32 button, u32 x, u32 y) {
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x0077c488) == 0) return 0;  // only AX is set
    const s32 b = (s16)button;
    u8 *const *const hovered = reinterpret_cast<u8 *const *>((uintptr_t)0x00faa5e0);
    Window_PostMessage(*hovered, 0x10005, (u32)b, ((u32)(s32)(s16)x << 16) | (u32)(s32)(s16)y);
    reinterpret_cast<u8 **>((uintptr_t)0x00faa588)[b] = *hovered;
    const u32 bit = 1u << (b & 31);
    *reinterpret_cast<u16 *>((uintptr_t)0x00faa5bc) |= (u16)bit;
    return (bit & 0xffff0000u) | 1;
}

u32 __cdecl Gui_MouseUp(u32 button, u32 x, u32 y) {
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x0077c488) == 0) return 0;  // only AX is set
    const s32 b = (s16)button;
    u8 **const windows = reinterpret_cast<u8 **>((uintptr_t)0x00faa588);
    Window_PostMessage(windows[b], 0x10004, (u32)b, ((u32)(s32)(s16)x << 16) | (u32)(s32)(s16)y);
    windows[b] = nullptr;
    const u32 mask = ~(1u << (b & 31));
    *reinterpret_cast<u16 *>((uintptr_t)0x00faa5bc) &= (u16)mask;
    return (mask & 0xffff0000u) | 1;
}

u32 __cdecl Gui_MouseWheel(u32 delta, u32 x, u32 y) {
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x0077c488) == 0) return 0;  // only AX is set
    u8 *const capture = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5e8);
    u8 *const target = capture ? capture : *reinterpret_cast<u8 **>((uintptr_t)0x00faa5e0);
    Window_PostMessage(target, 0x1000d, delta, ((u32)(s32)(s16)x << 16) | (u32)(s32)(s16)y);
    return 1;  // (AX; the rest the post's answer, 0)
}

u32 __cdecl Gui_MouseMove(u32 x, u32 y) {
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x0077c488) == 0) return 0;  // only AX is set
    *reinterpret_cast<u16 *>((uintptr_t)0x00faa540) = (u16)x;
    *reinterpret_cast<u16 *>((uintptr_t)0x00faa544) = (u16)y;
    u8 *const root = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5d8);
    if (!root) return 0;
    const u32 xy = ((u32)(s32)(s16)x << 16) | (u32)(s32)(s16)y;
    if (u8 *const capture = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5e8)) {
        Window_PostMessage(capture, 0x10003, xy, 0);
        return 1;
    }
    u8 *const hit = Window_HitTest(root, nullptr, (u16)x, (u16)y);
    u8 *&hovered = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5e0);
    if (hit != hovered) {
        if (hovered) Window_PostMessage(hovered, 0x10002, 0, 0);
        hovered = hit;
        Window_PostMessage(hovered, 0x10001, xy, 0);
    } else {
        Window_PostMessage(hovered, 0x10003, xy, 0);
    }
    return 1;
}

void __cdecl Gui_PollMouseMove() {
    s16 *const now = reinterpret_cast<s16 *>((uintptr_t)0x00faa540);   // (x, then y at +4)
    s16 *const seen = reinterpret_cast<s16 *>((uintptr_t)0x00faa5cc);  // (x, then y at +4)
    const u32 x_high = (u32)(s32)seen[0] & 0xffff0000u;
    u32 y_high;
    if (now[0] != seen[0])
        y_high = (u32)(s32)now[0] & 0xffff0000u;
    else if (now[2] != seen[2])
        y_high = (u32)(s32)seen[2] & 0xffff0000u;
    else
        return;
    seen[0] = now[0];
    seen[2] = now[2];
    Gui_MouseMove(x_high | (u16)now[0], y_high | (u16)now[2]);
}
}

extern "C" {

u16 __fastcall Window_Unlink(u8 *window) {
    if (at<u8 *>(window, 0x60) && !at<u8 *>(window, 0x58)) at<u8 *>(at<u8 *>(window, 0x60), 0x5c) = at<u8 *>(window, 0x54);
    if (at<u8 *>(window, 0x58)) at<u8 *>(at<u8 *>(window, 0x58), 0x54) = at<u8 *>(window, 0x54);
    if (at<u8 *>(window, 0x54)) at<u8 *>(at<u8 *>(window, 0x54), 0x58) = at<u8 *>(window, 0x58);
    at<u8 *>(window, 0x54) = nullptr;
    at<u8 *>(window, 0x58) = nullptr;
    at<u8 *>(window, 0x60) = nullptr;
    return (u16)Window_UpdateHover();
}

u16 __fastcall Window_SetHandler(u8 *window, void *, u32 handler) {
    if (handler) at<u32>(window, 0x114) = handler;
    return 1;
}

void __fastcall Window_SetDepth(u8 *window, void *, u32 depth) {
    at<u16>(window, 0xe0) = (u16)depth;
    if (at<s16>(window, 0xe0) > 0x100) at<u16>(window, 0xe0) = 0x100;
    u8 event[0x18];  // (only the word +6 set)
    at<u16>(event, 6) = at<u16>(window, 0xe0);
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 2u, event);
    at<u16>(event, 6) = (u16)(at<s16>(window, 0xe0) + 1);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 2u, event);
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) Window_SetDepth(child, nullptr, (u32)((s16)depth + 2));
}

void __fastcall Window_SetWordE2(u8 *window, void *, u32 value) {
    at<u16>(window, 0xe2) = (u16)value;
    u8 event[0x18];  // (only the word +8 set)
    at<u16>(event, 8) = at<u16>(window, 0xe2);
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 4u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 4u, event);
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) Window_SetWordE2(child, nullptr, value);
}

void __fastcall Window_Notify1(u8 *window, void *, u32 value) {
    u8 event[0x18];  // (only the word +4 set)
    at<u16>(event, 4) = (u16)value;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 1u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 1u, event);
}

void __fastcall Window_Notify1b(u8 *window, void *, u32 value) {
    u8 event[0x18];  // (only the word +4 set)
    at<u16>(event, 4) = (u16)value;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 1u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 1u, event);
}

u32 __fastcall Window_Notify80(u8 *window, void *, u32 value) {
    u8 event[0x18];  // (only the dword +0 set)
    at<u32>(event, 0) = value;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x80u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x80u, event);
    return 1;
}

void __fastcall Window_Notify80000(u8 *window, void *, u32 value) {
    u8 event[0x18];  // (only the word +0xa set)
    at<u16>(event, 0xa) = (u16)value;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x80000u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x80000u, event);
}

u32 __fastcall Window_GetSize(u8 *window, void *, s16 *width, s16 *height, u32 inner) {
    GuiRect_GetSize(window + 8, nullptr, (u16 *)width, (u16 *)height);
    if ((s16)inner != 0) {
        if (width) *width = (s16)(*width - (s16)(at<s16>(window, 0xfc) - at<s16>(window, 0xf8)));
        if (height) *height = (s16)(*height - (s16)(at<s16>(window, 0xfe) - at<s16>(window, 0xfa)));
    }
    return 1;
}

void __fastcall Window_PostIfHit(u8 *window, void *, u32 message, u32 button, u32 point) {
    if ((s16)vcall<u32>(window + 0x2c, 0, (u32)((s32)point >> 16), point & 0xffff) == 0) return;
    const u8 *const entry = reinterpret_cast<const u8 *>((uintptr_t)0x00faa598 + button * 0xc);
    const u32 press = (u32)((s32)*reinterpret_cast<const s16 *>(entry + 2) << 16) | (u32)(s32)*reinterpret_cast<const s16 *>(entry + 4);  // sic
    Window_PostMessage(window, message | 0x1000, button, press);
}

void __fastcall Window_Activate(u8 *window) {
    u8 *&active = *reinterpret_cast<u8 **>((uintptr_t)0x00faa5dc);
    if (active == window) return;
    Window_PostMessage(active, 0x1b, 0, 0);
    active = window;
    Window_PostMessage(active, 0x1a, 0, 0);
}

u8 *__fastcall Window_RaiseToTop(u8 *window) {
    if (!at<u8 *>(window, 0x54)) return nullptr;
    u8 *const prev = at<u8 *>(window, 0x58);
    u8 *const next = at<u8 *>(window, 0x54);
    u8 *last = window;
    while (at<u8 *>(last, 0x54)) last = at<u8 *>(last, 0x54);
    at<u8 *>(window, 0x54) = nullptr;
    at<u8 *>(last, 0x54) = window;
    at<u8 *>(window, 0x58) = last;
    if (prev) at<u8 *>(prev, 0x54) = next;
    if (next) {
        at<u8 *>(next, 0x58) = prev;
        if (!prev) at<u8 *>(at<u8 *>(next, 0x60), 0x5c) = next;
    }
    Window_PostMessage(window, 0x18, 0, 0);
    Window_PostMessage(last, 0x19, 0, 0);
    return last;
}

u32 __fastcall Window_OpenTree(u8 *window) {
    if (at<u32>(window, 0x48) & 1) {
        if (!(at<u32>(window, 0x48) & 0xf0000000u)) {
            vcall<void>(window, 0);
            Window_SendMessage(window, 3, 0, 0);
        }
        for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) Window_OpenTree(child);
    }
    return 1;
}

u32 __fastcall Window_CloseTree(u8 *window) {
    if (at<u32>(window, 0x48) & 1) {
        u8 *child = at<u8 *>(window, 0x5c);
        if (child) {
            while (at<u8 *>(child, 0x54)) child = at<u8 *>(child, 0x54);
            for (; child; child = at<u8 *>(child, 0x58)) Window_CloseTree(child);
        }
        if (!(at<u32>(window, 0x48) & 0xf0000000u)) {
            Window_SendMessage(window, 3, 0, 0);
            vcall<void>(window, 0);
        }
    }
    return 1;
}

u32 __fastcall Window_SetRect(u8 *window, void *, u8 *rect) {
    if (!rect || GuiRect_Equals(rect, nullptr, window + 8)) return 1;
    u16 width, height, oldWidth, oldHeight;
    GuiRect_GetSize(rect, nullptr, &width, &height);
    GuiRect_GetSize(window + 8, nullptr, &oldWidth, &oldHeight);
    if ((s16)width == (s16)oldWidth && (s16)height == (s16)oldHeight) {
        Window_Offset(window, nullptr, (u32)(at<s16>(rect, 4) - at<s16>(window, 0xc)), (u32)(at<s16>(rect, 6) - at<s16>(window, 0xe)));
        Window_UpdateHover();
        return 1;
    }
    GuiRect_Assign(window + 0x20, nullptr, window + 8);
    GuiRect_Assign(window + 8, nullptr, rect);
    Window_ResizeChildren(window, nullptr, (u32)(at<s16>(window, 0xc) - at<s16>(window, 0x24)), (u32)(at<s16>(window, 0xe) - at<s16>(window, 0x26)),
                          (u32)(at<s16>(window, 0x10) - at<s16>(window, 0x28)), (u32)(at<s16>(window, 0x12) - at<s16>(window, 0x2a)));
    u8 event[0x28];  // (only the dword +0x14 set)
    at<u8 *>(event, 0x14) = window + 8;
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x10u, event);
    at<u8 *>(event, 0x14) = window + 0x2c;
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x10u, event);
    Window_SendMessage(window, 0x12, (u32)(s32)(s16)width, (u32)(s32)(s16)height);
    Window_UpdateHover();
    return 1;
}

void __fastcall Window_ResizeChildren(u8 *window, void *, u32 dl, u32 dt, u32 dr, u32 db) {
    u8 *child = at<u8 *>(window, 0x5c);
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    vcall<void>(window + 0x2c, 0xc, (u32)(u16)dl, (u32)(u16)dt, (u32)(u16)dr, (u32)(u16)db);
    const s16 l = (s16)dl, t = (s16)dt, r = (s16)dr, b = (s16)db;
    for (; child; child = at<u8 *>(child, 0x54)) {
        u32 flags = at<u32>(child, 0x48);
        Window_GetRect(child, nullptr, rect, 1);
        if ((flags & 0xf0000) == 0xf0000) {
            if (l || r || t || b) {
                at<u16>(rect, 4) += l;
                at<u16>(rect, 8) += r;
                at<u16>(rect, 6) += t;
                at<u16>(rect, 0xa) += b;
                Window_SetRect(child, nullptr, rect);
            }
            continue;
        }
        if ((flags & 0x30000) == 0x30000) {
            if (l || r) {
                at<u16>(rect, 4) += l;
                at<u16>(rect, 8) += r;
                Window_SetRect(child, nullptr, rect);
            }
            flags &= 0xfffcffff;
        } else if ((flags & 0xc0000) == 0xc0000) {
            if (t || b) {
                at<u16>(rect, 6) += t;
                at<u16>(rect, 0xa) += b;
                Window_SetRect(child, nullptr, rect);
            }
            flags &= 0xfff3ffff;
        }
        if ((flags & 0x10000) && l) Window_Offset(child, nullptr, (u16)l, 0);
        if ((flags & 0x20000) && r) Window_Offset(child, nullptr, (u16)r, 0);
        if ((flags & 0x40000) && t) Window_Offset(child, nullptr, 0, (u16)t);
        if ((flags & 0x80000) && b) Window_Offset(child, nullptr, 0, (u16)b);
    }
    GuiRect_Dtor(rect);
}

u32 __fastcall Window_SetRectTo(u8 *window, void *, u32 left, u32 top, u32 right, u32 bottom) {
    u8 rect[0xc];
    GuiRect_CtorWith(rect, nullptr, (u16)left, (u16)top, (u16)right, (u16)bottom);
    Window_SetRect(window, nullptr, rect);
    GuiRect_Dtor(rect);
    return 1;
}

u32 __fastcall Window_MoveTo(u8 *window, void *, const s16 *x, const s16 *y) {
    const s16 width = (s16)(at<s16>(window, 0x10) - at<s16>(window, 0xc));
    const s16 height = (s16)(at<s16>(window, 0x12) - at<s16>(window, 0xe));
    const s16 left = x ? *x : at<s16>(window, 0xc);
    const s16 top = y ? *y : at<s16>(window, 0xe);
    u8 rect[0xc];
    GuiRect_CtorWith(rect, nullptr, (u16)left, (u16)top, (u32)(left + width), (u32)(top + height));
    Window_SetRect(window, nullptr, rect);
    GuiRect_Dtor(rect);
    return 1;
}

void __fastcall Window_SetWordsE4(u8 *window, void *, u32 a, u32 b) {
    at<u16>(window, 0xe4) = (u16)a;
    at<u16>(window, 0xe6) = (u16)b;
    u8 event[0x18];  // (only the words +0xc / +0xe set)
    at<u16>(event, 0xc) = at<u16>(window, 0xe4);
    at<u16>(event, 0xe) = at<u16>(window, 0xe6);
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 0x40u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 0x40u, event);
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) Window_SetWordsE4(child, nullptr, (u16)a, (u16)b);
}

u32 __fastcall Window_AddChild(u8 *window, void *, u8 *child, u32 depth, u32 words) {
    if (!child || at<u8 *>(child, 0x60) || at<u8 *>(child, 0x54) || at<u8 *>(child, 0x58)) return 0;
    u8 *last = nullptr;
    at<u8 *>(child, 0x60) = window;
    if (at<u8 *>(window, 0x5c)) {
        last = at<u8 *>(window, 0x5c);
        while (at<u8 *>(last, 0x54)) last = at<u8 *>(last, 0x54);
        at<u8 *>(last, 0x54) = child;
    } else {
        at<u8 *>(window, 0x5c) = child;
    }
    at<u8 *>(child, 0x58) = last;
    if ((s8)depth != 0) Window_SetDepth(child, nullptr, (u32)((s32)at<s16>(window, 0xe0) + 2));
    if ((s8)words != 0) Window_SetWordsE4(child, nullptr, at<u16>(window, 0xe4), at<u16>(window, 0xe6));
    Window_UpdateHover();
    Window_KillFocus(window, nullptr, 0);
    return 1;
}

u32 __fastcall Window_ScriptStep(u8 *window) {
    u8 rect[0x10];
    GuiRect_Ctor(rect);
    if ((s8)at<u8>(window, 0x135) != 1 || !at<u8 *>(window, 0xe8) ||
        (s32)(GuiClock_Now() - at<u32>(window, 0xf4)) <= at<s32>(window, 0xf0)) {
        GuiRect_Dtor(rect);
        return 0xffff;
    }
    at<u32>(window, 0xf4) = GuiClock_Now();
    const s16 *const step = reinterpret_cast<const s16 *>(at<u8 *>(window, 0xe8) + (s32)at<s16>(window, 0xec) * 8);
    if (step[0] == -9999) {
        at<u8>(window, 0x135) = 0;
        at<u16>(window, 0xec) = 0;
        GuiRect_Assign(rect, nullptr, window + 8);
        GuiRect_Adjust(rect, nullptr, (u32)-(s32)at<s16>(window, 0xf8), (u32)-(s32)at<s16>(window, 0xfa), (u32)-(s32)at<s16>(window, 0xfc),
                       (u32)-(s32)at<s16>(window, 0xfe));
        Window_SetRect(window, nullptr, rect);
        at<u16>(window, 0xfe) = 0;
        at<u16>(window, 0xfc) = 0;
        at<u16>(window, 0xfa) = 0;
        at<u16>(window, 0xf8) = 0;
    } else if (step[0] == -9998) {
        Window_SetWordE2(window, nullptr, (u16)step[1]);
        at<u16>(window, 0xec) += 1;
    } else if (step[0] == -9997) {
        at<u16>(window, 0xec) = (u16)step[1];
        Window_ScriptStep(window);
    } else if (step[0] == -9996) {
        Window_Notify1(window, nullptr, (u32)(s32)step[1]);
        at<u16>(window, 0xec) += 1;
    } else {
        GuiRect_Assign(rect, nullptr, window + 8);
        for (int i = 0; i < 4; ++i) at<u16>(rect, 4 + 2 * i) += (u16)step[i];
        for (int i = 0; i < 4; ++i) at<u16>(window, 0xf8 + 2 * i) += (u16)step[i];
        Window_SetRect(window, nullptr, rect);
        at<u16>(window, 0xec) += 1;
    }
    const u16 now = at<u16>(window, 0xec);
    GuiRect_Dtor(rect);
    return now;
}

u32 __fastcall Window_SlideStep(u8 *window) {
    u8 rect[0x10];
    GuiRect_Ctor(rect);
    if (at<u8>(window, 0x136)) {
        if (at<s16>(window, 0x10c) == 0) {
            Fn_662097(at<u8 *>(window, 0x100));
            at<u8>(window, 0x136) = 0;
            Window_SetRect(window, nullptr, window + 0x38);
            Window_PostMessage(window, 0x1d, 0, 0);
        } else {
            GuiRect_Assign(rect, nullptr, window + 8);
            GuiRect_Adjust(rect, nullptr, at<u16>(window, 0x104), at<u16>(window, 0x106), at<u16>(window, 0x108), at<u16>(window, 0x10a));
            Window_SetRect(window, nullptr, rect);
            at<u16>(window, 0x10c) -= 1;
        }
    }
    GuiRect_Dtor(rect);
    return 0xffff;
}

u32 __fastcall Window_SlideTo(u8 *window, void *, const u8 *target, s32 time, s32 interval) {
    at<s16>(window, 0x10c) = (s16)Idiv(time, interval);
    GuiRect_Assign(window + 0x38, nullptr, target);
    for (u32 i = 0; i < 4; ++i) {
        at<s16>(window, 0x104 + 2 * i) =
            (s16)Idiv((s32)*reinterpret_cast<const s16 *>(target + 4 + 2 * i) - at<s16>(window, 0xc + 2 * i), at<s16>(window, 0x10c));
    }
    if (at<u8 *>(window, 0x100)) Fn_6620cc(at<u8 *>(window, 0x100), nullptr, (u32)interval);
    else at<u8 *>(window, 0x100) = WindowTimers_Add((u32)(uintptr_t)window, (u32)interval, 0x80085);
    at<u8>(window, 0x136) = 1;
    return (WindowTimer_Start(at<u8 *>(window, 0x100)) & 0xffff0000u) | 1;
}

u32 __fastcall Window_ScriptTick(u8 *window) {
    u8 *const manager = *reinterpret_cast<u8 **>((uintptr_t)0x0077c48c);
    if (at<u8 *>(manager, 0x14) == window) vcall<void>(manager, 0x30);
    return Window_ScriptStep(window);
}

namespace {
u8 *press(u32 button) { return reinterpret_cast<u8 *>((uintptr_t)(0x00faa598u + button * 0xcu)); }
s32 iabs(s32 v) { return v < 0 ? -v : v; }
}  // namespace

u32 __cdecl Window_DefaultProc(u8 *window, u32 message, u32 a, u32 b) {
    if (message & 0x80000000u) return 0;
    u8 *const manager = *reinterpret_cast<u8 **>((uintptr_t)0x0077c48c);
    const s32 doubleTime = *reinterpret_cast<const s32 *>((uintptr_t)0x0077c480);
    switch (message) {
    case 1: Window_OpenTree(window); break;
    case 2: Window_CloseTree(window); break;
    case 3: vcall<void>(window, 0x10); break;
    case 4:
        if (window) vcall<void>(window, 8, 1u);
        return 0;
    case 5: Window_PostMessage(window, 4, 0, 0); break;
    case 6: Window_SetShown(window, nullptr, (u16)a); break;
    case 7: Window_Offset(window, nullptr, (u16)a, (u16)b); break;
    case 8: Window_SetRectTo(window, nullptr, (u32)((s32)a >> 16), a & 0xffff, (u32)((s32)b >> 16), b & 0xffff); break;
    case 9:
        if (Window_RaiseToTop(window) && a && *reinterpret_cast<const u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 1, 0, 0);
        break;
    case 12:
        *reinterpret_cast<u32 *>((uintptr_t)0x00faa618) = vcall<u32>(window, 0x2c);
        return 0x00faa618;
    case 0x10:
        if (a == 0x80084) Window_ScriptStep(window);
        else if (a == 0x80085) Window_SlideStep(window);
        break;
    case 0x1e: Window_ScriptTick(window); break;
    case 0x10001:
        Window_ApplyCursor(window);
        vcall<void>(manager, 0x24, window);
        break;
    case 0x10002: vcall<void>(manager, 0x28); break;
    case 0x10003: {
        const s16 x = (s16)((s32)a >> 16), y = (s16)a;
        vcall<void>(manager, 0x20, (u32)((s32)a >> 16), (u32)(u16)y);
        if (at<u32>(window, 0x11c)) {
            for (s32 i = 0; i < 3; ++i) {
                u8 *const e = press((u32)i);
                if (at<s16>(e, 0) == 1 && (iabs((s32)at<s16>(e, 2) - x) > 6 || iabs((s32)at<s16>(e, 4) - y) > 6)) at<u16>(e, 0) = 2;
            }
        }
        break;
    }
    case 0x10004: {
        Window_PostIfHit(window, nullptr, message, a, b);
        u8 *const e = press(a);
        const u32 bit = 1u << (a & 31);
        if (at<s16>(e, 0) == 1 && (at<u32>(window, 0x11c) & bit)) {
            const u32 now = GuiClock_Now();
            if ((s32)(now - at<u32>(e, 8)) < doubleTime)
                Window_PostMessage(window, 0x10006, a, (u32)((s32)at<s16>(e, 2) << 16 | (s32)at<s16>(e, 4)));  // sic
            at<u32>(e, 8) = now;
        } else {
            at<u32>(e, 8) = 0;
        }
        at<u32>(window, 0x11c) &= ~bit;
        at<u16>(e, 0) = 0;
        break;
    }
    case 0x10005: {
        Window_Activate(window);
        Window_PostIfHit(window, nullptr, message, a, b);
        const u32 now = GuiClock_Now();
        u8 *const e = press(a);
        if (at<s16>(e, 0) == 0) {
            at<u32>(window, 0x11c) |= 1u << (a & 31);
            at<u16>(e, 2) = (u16)((s32)b >> 16);
            at<u16>(e, 4) = (u16)b;
            at<u16>(e, 0) = 1;
            if (!(Fn_65d57c(window) & 8) && (s32)(now - at<u32>(e, 8)) < doubleTime) {
                Window_PostMessage(window, 0x10007, a, b);
                at<u16>(e, 0) = 2;
            }
            at<u32>(e, 8) = GuiClock_Now();
        }
        break;
    }
    case 0x10006:
    case 0x10007: Window_PostIfHit(window, nullptr, message, a, b); break;
    default: break;
    }
    return 0;
}
}
