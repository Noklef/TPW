// The input filters: see devicefilter.h.
#include "devicefilter.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/misc.h"
#include "../core/result.h"
#include "../core/sectionlock.h"
#include "../core/selectors.h"
#include "../core/vcall.h"
#include "../core/vdtor.h"
#include "../math/x87.h"

namespace {

AddrCalls g_calls;

constexpr u32 kListen = 0x005fc030, kUnlisten = 0x005fc100, kQueue = 0x00fa7cb0, kDispatch = 0x006336d0;   // (the event queue's, by its addresses)
constexpr s32 kEndOfList = 6;

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> const T &at(const u8 *base, u32 offset) { return *reinterpret_cast<const T *>(base + offset); }

template <typename F> F import(u32 slot) { return *reinterpret_cast<F const *>(static_cast<uintptr_t>(slot)); }

template <typename... A> u32 com(u8 *object, u32 offset, A... args) { return comcall<u32>(object, offset, args...); }

// _ftol: the low dword of the truncated value (NaN: 0).
s32 Ftol(float v) { return (s32)(u32)(long long)v; }

// The chain's next filter, or the message marked at the chain's end.
void passOn(u8 *self, u8 *message, u32 slot) {
    if (u8 *const next = at<u8 *>(self, 8)) vcall<void>(next, slot, message);
    else at<u32>(message, 8) = 0;
}

// The holder at the slot replaced (the new one kept, the old released), the state read from it.
void setHolder(u8 *self, u32 slot, u8 *holder) {
    if (holder) vcall<void>(holder, 4);
    if (u8 *const old = at<u8 *>(self, slot)) vcall<void>(old, 0);
    at<u8 *>(self, slot) = holder;
    KeyFilter_ReadState(self, nullptr, holder ? static_cast<u32>(reinterpret_cast<uintptr_t>(holder + 0x10)) : 0);
}

template <typename... A> void handler(u32 function, A... args) { reinterpret_cast<void(__cdecl *)(A...)>(static_cast<uintptr_t>(function))(args...); }

}  // namespace

void InputFilter_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __fastcall InputFilter_SetNext(u8 *self, void *, u8 *object) {
    if (u8 *const old = at<u8 *>(self, 8)) vcall<void>(old, 0);
    if (object) vcall<void>(object, 4);
    at<u8 *>(self, 8) = object;
}

void __fastcall InputFilter_CloseHandle(u8 *self) {
    if (const u32 handle = at<u32>(self, 8)) {
        import<s32(__stdcall *)(u32)>(0x006fd13c)(handle);
        at<u32>(self, 8) = 0;
    }
}

void __fastcall InputFilter_SetSink(u8 *self, void *, u8 *object) {
    if (u8 *const old = at<u8 *>(self, 0xc)) {
        at<u32>(self, 0xc) = 0;
        vcall<void>(old, 0);
    }
    if (object) vcall<void>(object, 4);
    at<u8 *>(self, 0xc) = object;
}

void __fastcall KeyFilter_OnMessage(u8 *self, void *, u8 *message) {
    self[at<u32>(message, 0x10) + 0x1c] = at<u8>(message, 0x1c);
    passOn(self, message, 0x18);
}

void __stdcall InputSubs_Dispatch(u32 a, u32 b, u8 *object) { vcall<void>(object, 0, a, b); }

void __fastcall InputSubs_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703744;
    InputSubs_Unsubscribe(self);
}

void __fastcall InputSubs_Subscribe(u8 *self, void *, const s32 *list, u32 object) {
    InputSubs_Unsubscribe(self);
    at<u32>(self, 8) = object;
    at<const s32 *>(self, 4) = list;
    for (s32 channel = *list; channel != kEndOfList; channel = *++list) g_calls.Th(kListen, kQueue, channel, kDispatch, at<u32>(self, 8));
}

void __fastcall InputSubs_Unsubscribe(u8 *self) {
    if (!at<const s32 *>(self, 4)) return;
    while (*at<const s32 *>(self, 4) != kEndOfList) {
        g_calls.Th(kUnlisten, kQueue, *at<const s32 *>(self, 4), kDispatch, at<u32>(self, 8));
        ++at<const s32 *>(self, 4);
    }
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = 0;
}

void __fastcall HandlerFilter_OnMouse(u8 *self, void *, u8 *message) {
    if (at<u8 *>(self, 0xc)) {
        const u32 bits = at<u32>(message, 0x20), low = bits & 7, flag = bits & 8;
        const s32 what = at<s32>(message, 0x10);
        if (what >= 0xf0 && what <= 0xf3) {
            if (const u32 f = at<u32>(at<u8 *>(self, 0xc), 0x10)) handler(f, what, flag, low, at<u32>(message, 0x14), at<u32>(message, 0x18), at<u32>(self, 0x10));
            if (const u32 f = at<u32>(at<u8 *>(self, 0xc), 4)) handler(f, what, flag, at<u32>(message, 0x14), at<u32>(message, 0x18), at<u32>(self, 0x10));
        } else {
            if (const u32 f = at<u32>(at<u8 *>(self, 0xc), 0xc)) handler(f, what, flag, low, at<u32>(self, 0x10));
            if (const u32 f = at<u32>(at<u8 *>(self, 0xc), 0)) handler(f, what, flag, at<u32>(self, 0x10));
        }
    }
    passOn(self, message, 0x18);
}

void __fastcall HandlerFilter_OnChar(u8 *self, void *, u8 *message) {
    if (const u32 f = at<u32>(at<u8 *>(self, 0xc), 8)) {
        const u32 context = at<u32>(self, 0x10);
        handler(f, (context & 0xffff0000u) | at<u16>(message, 0xc), at<u32>(message, 0x10), 0u, context);   // (the context's top half stays in the register, sic)
    }
    passOn(self, message, 0x10);
}

void __fastcall DeviceHolder_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703c80;
    if (u8 *const device = at<u8 *>(self, 0xc)) com(device, 8);
}

u32 *__fastcall DeviceHolder_GetDevice(u8 *self, void *, u32 *result, u32 *device) {
    if (!device) {
        *result = 0x80004003;
        return result;
    }
    *device = at<u32>(self, 0xc);
    com(at<u8 *>(self, 0xc), 4);
    *result = 0;
    return result;
}

u32 *__fastcall DeviceHolder_SetLevel(u8 *self, void *, u32 *result) {
    if (!at<u8 *>(self, 0xc)) {
        *result = 0x80004003;
        return result;
    }
    const u32 window = Obj_Get1cOrWindow(self);
    u32 hr = 0x4ffe0002;
    const bool acquired = at<u32>(self, 0x10) != 0;
    const bool same = (!window || window == at<u32>(self, 0x18)) && at<u32>(self, 8) == at<u32>(self, 4);
    if (!acquired || !same) {   // (the release's result stands for what follows)
        hr = com(at<u8 *>(self, 0xc), 0x20);
        if (static_cast<s32>(hr) < 0) {
            *result = hr;
            return result;
        }
    }
    if (window != at<u32>(self, 0x18)) {
        hr = com(at<u8 *>(self, 0xc), 0x34, at<u32>(self, 0x18), 0xau);
        if (static_cast<s32>(hr) >= 0) at<u32>(self, 8) = 0xa;
    }
    if ((window && window != at<u32>(self, 0x18)) || at<u32>(self, 8) != at<u32>(self, 4)) {
        hr = com(at<u8 *>(self, 0xc), 0x34, window, at<u32>(self, 4));
        if (static_cast<s32>(hr) >= 0) {
            at<u32>(self, 0x18) = window;
            at<u32>(self, 8) = at<u32>(self, 4);
        }
    }
    if (hr == 1) hr = 0x4ffe0002;
    *result = hr;
    return result;
}

u32 *__fastcall DeviceHolder_Apply(u8 *self, void *, u32 *result) {
    if (!at<u8 *>(self, 0xc)) {
        *result = 0xcffd0200;
        return result;
    }
    u32 hr;
    DeviceHolder_SetLevel(self, nullptr, &hr);
    if (static_cast<s32>(hr) >= 0) {
        if (at<u32>(self, 0x10)) hr = com(at<u8 *>(self, 0xc), 0x1c);
        if (hr == 1) hr = 0x4ffe0002;
    }
    *result = hr;
    return result;
}

void __fastcall ComHolder_Release(u32 *self) {
    if (*self) com(reinterpret_cast<u8 *>(static_cast<uintptr_t>(*self)), 8);
}

void __fastcall KeyFilter_ReadModifiers(u8 *self, void *, u8 *holder) {
    at<u32>(self, 0x20) = 0;
    if (!at<u32>(self, 0x1c) || at<u32>(self, 0x1c) + 0x10 != static_cast<u32>(reinterpret_cast<uintptr_t>(holder))) return;
    u32 device, result;
    u8 keys[0x100];
    DeviceHolder_GetDevice(holder, nullptr, &result, &device);
    if (static_cast<s32>(com(reinterpret_cast<u8 *>(static_cast<uintptr_t>(device)), 0x24, 0x100u, keys)) < 0) return;
    if ((keys[0x38] | keys[0xb8]) & 0x80) at<u32>(self, 0x20) |= 4;
    if ((keys[0x1d] | keys[0x9d]) & 0x80) at<u32>(self, 0x20) |= 2;
    if ((keys[0x2a] | keys[0x36]) & 0x80) at<u32>(self, 0x20) |= 1;
}

u8 *__fastcall MouseFilter_Ctor(u8 *self) {
    Class703770_Ctor(self);
    for (u32 offset = 8; offset <= 0x2c; offset += 4) at<u32>(self, offset) = 0;
    at<u32>(self, 0x30) = 0x3f800000;
    for (u32 offset = 0x34; offset <= 0x40; offset += 4) at<u32>(self, offset) = 0;
    at<u32>(self, 0x48) = 0;
    at<u32>(self, 0x4c) = 0;
    at<u32>(self, 0x5c) = 0;
    at<u8>(self, 0x60) = 0;
    at<u32>(self, 0) = 0x00703cc8;
    u32 mouse[3];
    import<s32(__stdcall *)(u32, u32, u32 *, u32)>(0x006fd374)(3, 0, mouse, 0);   // SystemParametersInfoA(SPI_GETMOUSE)
    at<u32>(self, 0x50) = mouse[0];
    at<u32>(self, 0x54) = mouse[1];
    at<u32>(self, 0x58) = mouse[2];
    at<u32>(self, 0x44) = import<u32(__stdcall *)()>(0x006fd32c)();   // GetDoubleClickTime
    const auto swap = import<s32(__stdcall *)(u32)>(0x006fd330);   // SwapMouseButton
    if (swap(0)) {
        at<u8>(self, 0x60) = 1;
        swap(1);
    }
    return self;
}

void __fastcall MouseFilter_OnMessage(u8 *self, void *, u8 *message) {
    s32 what = at<s32>(message, 0x10);
    if (at<u8>(self, 0x60)) {
        if (what == 0xf0) at<s32>(message, 0x10) = what = 0xf1;
        else if (what == 0xf1) at<s32>(message, 0x10) = what = 0xf0;
    }
    if (what == at<s32>(self, 0x48) && (at<u32>(message, 0x20) & 8)) {
        if (static_cast<s32>(at<u32>(message, 4) - at<u32>(self, 0x4c)) < 0) at<u32>(message, 0x20) |= 0x10;
        at<u32>(self, 0x48) = 0;
    } else if (what >= 0xf0 && what <= 0xf3) {
        at<s32>(self, 0x48) = what;
        at<u32>(self, 0x4c) = at<u32>(self, 0x44) + at<u32>(message, 4);
    }
    at<u32>(message, 0x14) = at<u32>(self, 0x10);
    at<u32>(message, 0x18) = at<u32>(self, 0x14);
    passOn(self, message, 0x18);
}

void __fastcall MouseFilter_ClampPosition(u8 *self) {
    // (FILD holds the int exactly, so the tests are of doubles. Only a value above the float passes the low bound's test; a NaN position takes the high bound.)
    if (static_cast<double>(at<s32>(self, 0x34)) > at<float>(self, 0x20)) at<float>(self, 0x20) = static_cast<float>(at<s32>(self, 0x34));
    if (!(static_cast<double>(at<s32>(self, 0x3c)) >= at<float>(self, 0x20))) at<float>(self, 0x20) = static_cast<float>(at<s32>(self, 0x3c));
    if (static_cast<double>(at<s32>(self, 0x38)) > at<float>(self, 0x24)) at<float>(self, 0x24) = static_cast<float>(at<s32>(self, 0x38));
    if (!(static_cast<double>(at<s32>(self, 0x40)) >= at<float>(self, 0x24))) at<float>(self, 0x24) = static_cast<float>(at<s32>(self, 0x40));
    at<s32>(self, 0x10) = Ftol(at<float>(self, 0x20));
    at<s32>(self, 0x14) = Ftol(at<float>(self, 0x24));
}

void __fastcall MouseFilter_OnBounds(u8 *self, void *, u32, const u32 *message) {
    if (message[0] != 7) return;
    const u32 bounds[4] = {0, 0, message[1], message[2]};
    Obj_Copy4At34(self - 0x64, nullptr, bounds);
}

u8 *__fastcall InputFilter_DeletingDtor(u8 *self, void *, u32 flags) {
    InputFilter_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall InputFilter_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703c18;
    if (u8 *const sink = at<u8 *>(self, 0xc)) vcall<void>(sink, 0);
    InputFilter_CloseHandle(self);
    at<u32>(self, 0) = 0x00703770;
}

u8 *__fastcall KeyFilter_Ctor(u8 *self) {
    Class703770_Ctor(self);
    at<u32>(self, 8) = 0;
    at<u32>(self, 0x14) = 0;
    at<u32>(self, 0x18) = 0;
    at<u32>(self, 0x10) = 0x00703744;
    at<u32>(self, 0x11c) = 0;
    at<u32>(self, 0x120) = 0;
    at<u32>(self, 0) = 0x00703c40;
    at<u32>(self, 0xc) = 0x00703c38;
    for (u32 offset = 0x1c; offset < 0x11c; offset += 4) at<u32>(self, offset) = 0;
    InputSubs_Subscribe(self + 0x10, nullptr, reinterpret_cast<const s32 *>(0x00703c30), static_cast<u32>(reinterpret_cast<uintptr_t>(self + 0xc)));
    return self;
}

void __fastcall KeyFilter_ReadState(u8 *self, void *, u32 holder) {
    u32 device = 0;
    u32 result;
    u8 *const holderBytes = reinterpret_cast<u8 *>(static_cast<uintptr_t>(holder));
    if (at<u32>(self, 0x11c) && at<u32>(self, 0x11c) + 0x10 == holder) {
        DeviceHolder_GetDevice(holderBytes, nullptr, &result, &device);
        u32 keys[0x40];
        if (static_cast<s32>(com(reinterpret_cast<u8 *>(static_cast<uintptr_t>(device)), 0x24, 0x100u, keys)) >= 0)
            for (u32 i = 0; i < 0x3c; ++i) at<u32>(self, 0x1c + 4 * i) = keys[i];
    }
    if (at<u32>(self, 0x120) && at<u32>(self, 0x120) + 0x10 == holder) {
        DeviceHolder_GetDevice(holderBytes, nullptr, &result, &device);
        u32 mouse[4];
        if (static_cast<s32>(com(reinterpret_cast<u8 *>(static_cast<uintptr_t>(device)), 0x24, 0x10u, mouse)) >= 0) at<u32>(self, 0x10c) = mouse[3];
    }
    if (device) com(reinterpret_cast<u8 *>(static_cast<uintptr_t>(device)), 8);
}

u8 *__fastcall DeviceHolder_DeletingDtor(u8 *self, void *, u32 flags) {
    DeviceHolder_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u32 *__fastcall DeviceHolder_SetMode(u8 *self, void *, u32 *result, u32 flags) {
    const u32 low = flags & 1, pair = flags & 4;
    if ((!low && !(flags & 2)) || (!pair && !(flags & 8))) {
        *result = 0xffffffff;
        return result;
    }
    if ((low && (flags & 2)) || (pair && (flags & 8))) {
        *result = 0xffffffff;
        return result;
    }
    if (low) at<u32>(self, 4) = (at<u32>(self, 4) & ~2u) | 1;
    else if (flags & 2) at<u32>(self, 4) = (at<u32>(self, 4) & ~1u) | 2;
    if (pair) at<u32>(self, 4) = (at<u32>(self, 4) & ~8u) | 4;
    else if (flags & 8) at<u32>(self, 4) = (at<u32>(self, 4) & ~4u) | 8;
    DeviceHolder_Apply(self, nullptr, result);
    return result;
}

u32 *__fastcall InputFilter_SubscribeC90(u8 *self, void *, u32 *result) {
    InputSubs_Subscribe(self + 0x10, nullptr, reinterpret_cast<const s32 *>(0x00703c90), self ? static_cast<u32>(reinterpret_cast<uintptr_t>(self + 0xc)) : 0);
    *result = 0;
    return result;
}

u32 *__fastcall KeyFilter_SetHolder(u8 *self, void *, u32 *result, u8 *holder) {
    at<u32>(self, 0x1c) = static_cast<u32>(reinterpret_cast<uintptr_t>(holder));
    KeyFilter_ReadModifiers(self, nullptr, holder ? holder + 0x10 : nullptr);
    *result = 0;
    return result;
}

u32 *__fastcall MouseFilter_Subscribe(u8 *self, void *, u32 *result) {
    InputSubs_Subscribe(self + 0x68, nullptr, reinterpret_cast<const s32 *>(0x00703cc0), self ? static_cast<u32>(reinterpret_cast<uintptr_t>(self + 0x64)) : 0);
    *result = 0;
    return result;
}

u8 *__fastcall DeviceFilter_Ctor(u8 *self) {
    Class703c18_Ctor(self);
    Class703c80_Ctor(self + 0x10);
    at<u32>(self, 0x10) = 0x00703ff0;
    at<u32>(self, 0) = 0x00703fd8;
    return self;
}

u8 *__fastcall DeviceFilter_DeletingDtor(u8 *self, void *, u32 flags) {
    DeviceFilter_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall DeviceFilter_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703fd8;
    at<u32>(self, 0x10) = 0x00703ff0;
    if (at<u32>(self, 0x30)) GameHeap_Free(at<void *>(self, 0x30));
    DeviceHolder_Dtor(self + 0x10);
    InputFilter_Dtor(self);
}

void __fastcall ModifierFilter_OnKey(u8 *self, void *, u8 *message) {
    const u32 key = at<u32>(message, 0x10);
    u32 &flags = at<u32>(self, 0x20);
    if (at<u32>(message, 0x20) & 8) {
        switch (key) {
        case 0x1d: case 0x9d: flags |= 2; break;
        case 0x2a: case 0x36: flags |= 1; break;
        case 0x38: case 0xb8: flags |= 4; break;
        case 0xf:
            if (flags & 4) {
                at<u32>(message, 8) = 0;
                return;
            }
            break;
        default: break;
        }
    } else {
        switch (key) {
        case 0x1d: case 0x9d: flags &= ~2u; break;
        case 0x2a: case 0x36: flags &= ~1u; break;
        case 0x38: case 0xb8: flags &= ~4u; break;
        default: break;
        }
    }
    at<u32>(message, 0x20) = flags | (at<u32>(message, 0x20) & ~7u);
    passOn(self, message, 0x18);
}

void __fastcall KeyFilter_Dtor(u8 *self) {
    if (u8 *const holder = at<u8 *>(self, 0x120)) vcall<void>(holder, 0);
    if (u8 *const holder = at<u8 *>(self, 0x11c)) vcall<void>(holder, 0);
    InputSubs_Dtor(self + 0x10);
    at<u32>(self, 0xc) = 0x00703748;
    Class703780_DtorF(self);
}

void __fastcall CharFilter_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703ca0;
    at<u32>(self, 0xc) = 0x00703c98;
    InputSubs_Dtor(self + 0x10);
    at<u32>(self, 0xc) = 0x00703748;
    Class703780_DtorF(self);
}

u32 *__fastcall InputFilter_OpenEvent(u8 *self, void *, u32 *result) {
    InputFilter_CloseHandle(self);
    at<u32>(self, 8) = import<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd0d4)(0, 0, 0, 0);
    if (!at<u32>(self, 8)) return reinterpret_cast<u32 *>(Result_FromLastError(reinterpret_cast<s32 *>(result)));
    *result = 0;
    return result;
}

void __fastcall MouseFilter_OnMove(u8 *self, void *, u8 *message) {
    s32 delta = at<s32>(message, 0x10);
    const u32 axis = at<u32>(message, 0xc);
    if (at<u32>(self, 0x5c) && (axis == 0 || axis == 4)) {
        const s32 level = at<s32>(self, 0x58);
        if (level >= 1 && delta > at<s32>(self, 0x50)) delta *= 2;
        if (level == 2 && delta > at<s32>(self, 0x54)) delta *= 2;
    }
    const X87Reg scaled = X87_MulIntDouble(delta, at<float>(self, 0x30));
    if (axis == 0) {
        at<float>(self, 0x20) = static_cast<float>(scaled + at<float>(self, 0x20));
        at<float>(self, 0x28) = static_cast<float>(scaled + at<float>(self, 0x28));
    } else if (axis == 4) {
        at<float>(self, 0x24) = static_cast<float>(scaled + at<float>(self, 0x24));
        at<float>(self, 0x2c) = static_cast<float>(scaled + at<float>(self, 0x2c));
    }
    MouseFilter_ClampPosition(self);
    if (at<u32>(self, 0xc)) g_calls.Th(0x005fba10, at<u32>(self, 0xc), at<u32>(message, 4));
    passOn(self, message, 0x14);
}

void __fastcall InputFilter_DrainValues(u8 *self) {
    u32 guard[3] = {at<u32>(self, 0x34), 0, 0};
    SectionGuard_Enter(guard);
    u32 *from = at<u32 *>(self, 0x28);
    for (u32 count = from ? static_cast<u32>(static_cast<s32>(at<u32>(self, 0x2c) - at<u32>(self, 0x28)) >> 2) : 0; count; --count, ++from) vcall<void>(at<u8 *>(self, 0xc), 0x10, *from);
    u32 *to = at<u32 *>(self, 0x28);
    u32 *const end = at<u32 *>(self, 0x2c);
    while (from != end) *to++ = *from++;
    at<u32 *>(self, 0x2c) = to;
    import<s32(__stdcall *)(u32)>(0x006fd168)(at<u32>(self, 8));   // ResetEvent
    SectionGuard_Dtor(guard);
}

void __fastcall DeviceFilter_Poll(u8 *self) {
    u32 count = 0x20;
    if (com(at<u8 *>(self, 0x1c), 0x28, 0x10u, at<u32>(self, 0x30), &count, 0u) == 0x8007001e) {
        u32 result;
        DeviceHolder_Apply(self + 0x10, nullptr, &result);
        return;
    }
    if (count) vcall<void>(self, 0x14, count);
}

u8 *__fastcall BaseFilter_DeletingDtor(u8 *self, void *, u32 flags) {
    Class703780_DtorF(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall KeyFilter_OnDeviceEvent(u8 *self, void *, u32, const u32 *message) { KeyFilter_ReadState(self - 0xc, nullptr, message[1]); }

u32 *__fastcall KeyFilter_SetKeyboard(u8 *self, void *, u32 *result, u8 *holder) {
    setHolder(self, 0x11c, holder);
    *result = 0;
    return result;
}

u32 *__fastcall KeyFilter_SetMouse(u8 *self, void *, u32 *result, u8 *holder) {
    setHolder(self, 0x120, holder);
    *result = 0;
    return result;
}

u32 *__fastcall InputFilter_Reopen(u8 *self, void *, u32 *result) {
    u32 local;
    const u32 status = *InputFilter_OpenEvent(self, nullptr, &local);
    *result = static_cast<s32>(status) < 0 ? status : 0;
    return result;
}

u32 *__fastcall DeviceHolder_Open(u8 *self, void *, u32 *result) {
    u8 *input = nullptr;
    const s32 created = import<s32(__stdcall *)(u32, u32, u8 **, u32)>(0x006fd020)(Obj_Get14OrInstance(self), 0x500, &input, 0);   // DirectInputCreateA
    if (created < 0) {
        *result = static_cast<u32>(created);
        if (input) com(input, 8);
        return result;
    }
    u32 status = com(input, 0xc, vcall<u32>(self, 4), &at<u32>(self, 0xc), 0u);
    if (static_cast<s32>(status) >= 0) status = com(at<u8 *>(self, 0xc), 0x2c, vcall<u32>(self, 8));
    com(input, 8);
    if (static_cast<s32>(status) < 0) {
        if (u8 *const device = at<u8 *>(self, 0xc)) {
            at<u32>(self, 0xc) = 0;
            com(device, 8);
        }
    }
    *result = status;
    return result;
}

u32 *__fastcall DeviceFilter_Open(u8 *self, void *, u32 *result) {
    u32 local;
    u32 status = *InputFilter_Reopen(self, nullptr, &local);
    if (static_cast<s32>(status) >= 0) status = *DeviceHolder_Open(self + 0x10, nullptr, &local);
    if (static_cast<s32>(status) >= 0) status = com(at<u8 *>(self, 0x1c), 0x30, at<u32>(self, 8));
    if (static_cast<s32>(status) < 0) {
        *result = status;
        return result;
    }
    at<void *>(self, 0x30) = GameHeap_Alloc(0x200);
    if (!at<void *>(self, 0x30)) {
        *result = 0xcffe0100;
        return result;
    }
    u32 property[5] = {0x14, 0x10, 0, 0, 0x20};   // DIPROPDWORD
    *result = com(at<u8 *>(self, 0x1c), 0x18, 1u, property);
    return result;
}

u8 *__fastcall KeyFilter_DeletingDtor(u8 *self, void *, u32 flags) {
    KeyFilter_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall CharFilter_DeletingDtor(u8 *self, void *, u32 flags) {
    CharFilter_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u32 __cdecl MessageSource_Constant1() { return 0x00707c50; }
u32 __cdecl MessageSource_Constant2() { return 0x0067a460; }

void __fastcall ChainFilter_Attach(u8 *self, void *, u8 *object) { vcall<void>(object, 0x14, self); }
void __fastcall ChainFilter_Detach(u8 *self, void *, u8 *object) { vcall<void>(object, 0x18, self); }

u32 __cdecl MouseFilter_Constant1() { return 0x00707c40; }
u32 __cdecl MouseFilter_Constant2() { return 0x00679440; }

}  // extern "C"
