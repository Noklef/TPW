// The thread waiting on event handles: see waitthread.h.
#include "waitthread.h"

#include <cstdint>

#include "ime.h"
#include "result.h"
#include "seqs.h"
#include "vcall.h"

namespace {

AddrCalls g_calls;

constexpr u32 kRangeError = 0x00632a40;

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

template <typename F> F import(u32 slot) { return *reinterpret_cast<F const *>(static_cast<uintptr_t>(slot)); }

s32 setEvent(u32 event) { return import<s32(__stdcall *)(u32)>(0x006fd124)(event); }

u32 createEvent() { return import<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd0d4)(0, 0, 0, 0); }

// vector::erase of one element: those after it moved down, the end back.
void eraseAt(u8 *vector, u32 *position) {
    const u32 *const end = at<u32 *>(vector, 8);
    for (u32 *p = position + 1; p != end; ++p) p[-1] = *p;
    at<u32 *>(vector, 8) = at<u32 *>(vector, 8) - 1;
}

// The handles' count: the byte span shifted signed; no block, 0.
s32 handleCount(u8 *self) {
    const u32 begin = at<u32>(self, 0x28);
    return begin ? static_cast<s32>(at<u32>(self, 0x2c) - begin) >> 2 : 0;
}

u32 *lastError(u32 *out) { return reinterpret_cast<u32 *>(Result_FromLastError(reinterpret_cast<s32 *>(out))); }

}  // namespace

void WaitThread_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __stdcall WaitThread_Proc(u8 *self) { return WaitThread_Loop(self); }

u32 __fastcall WaitThread_Loop(u8 *self) {
    while (!at<u32>(self, 0x10)) {
        const s32 which = import<s32(__stdcall *)(s32, u32 *, u32, u32)>(0x006fd160)(handleCount(self), at<u32 *>(self, 0x28), 0, 0xffffffff);   // WaitForMultipleObjects
        if (which == -1) {
            at<u32 *>(self, 0x3c) = Dwords_Copy(at<u32 *>(self, 0x3c), at<u32 *>(self, 0x3c), at<u32 *>(self, 0x38));
            at<u32 *>(self, 0x2c) = at<u32 *>(self, 0x28) + 1;
            continue;
        }
        if (which < 0 || static_cast<u32>(which) >= static_cast<u32>(handleCount(self))) continue;
        if (which == 0) {
            WaitThread_RunCommand(self);
            continue;
        }
        vcall<void>(reinterpret_cast<u8 *>(static_cast<uintptr_t>(at<u32 *>(self, 0x38)[which - 1])), 0xc);
        if (static_cast<u32>(which) >= static_cast<u32>(handleCount(self) - 1)) continue;
        u32 *const handles = at<u32 *>(self, 0x28);
        const u32 handle = handles[which];
        handles[which] = handles[which + 1];
        handles[which + 1] = handle;
        u32 *const objects = at<u32 *>(self, 0x38);
        const u32 object = objects[which - 1];
        objects[which - 1] = objects[which];
        objects[which] = object;
    }
    return 0;
}

void __fastcall WaitThread_RunCommand(u8 *self) {
    u32 result;
    switch (at<u32>(self, 4)) {
    case 1:
        at<u32>(self, 0xc) = *WaitThread_DoAdd(self, nullptr, &result);
        break;
    case 2:
        at<u32>(self, 0xc) = *WaitThread_DoRemove(self, nullptr, &result);
        break;
    case 3:
        at<u32>(self, 0x10) = 1;
        at<u32>(self, 0xc) = 0;
        break;
    default:
        at<u32>(self, 0xc) = 0;
        break;
    }
    setEvent(at<u32>(self, 0x1c));
}

u32 *__fastcall WaitThread_OpenEvents(u8 *self, void *, u32 *out) {
    if (!at<u32>(self, 0x18)) {
        at<u32>(self, 0x18) = createEvent();
        if (!at<u32>(self, 0x18)) return lastError(out);
        ImeVector_InsertN(self + 0x24, nullptr, at<u32 *>(self, 0x28), 1, &at<u32>(self, 0x18));
    }
    if (!at<u32>(self, 0x1c)) {
        at<u32>(self, 0x1c) = createEvent();
        if (!at<u32>(self, 0x1c)) return lastError(out);
    }
    *out = 0;
    return out;
}

void __fastcall WaitThread_CloseEvents(u8 *self) {
    const auto close = import<s32(__stdcall *)(u32)>(0x006fd13c);
    if (at<u32>(self, 0x18)) {
        close(at<u32>(self, 0x18));
        at<u32>(self, 0x18) = 0;
    }
    if (!at<u32>(self, 0x1c)) return;
    close(at<u32>(self, 0x1c));
    at<u32>(self, 0x1c) = 0;
    eraseAt(self + 0x24, at<u32 *>(self, 0x28));
}

u32 *__fastcall WaitThread_Send(u8 *self, void *, u32 *out) {
    if (!at<u32>(self, 4)) {
        *out = 0xffffffff;
        return out;
    }
    if (!setEvent(at<u32>(self, 0x18))) return lastError(out);
    if (import<u32(__stdcall *)(u32, u32)>(0x006fd128)(at<u32>(self, 0x1c), 0xffffffff)) return lastError(out);
    *out = at<u32>(self, 0xc);
    return out;
}

u32 *__fastcall WaitThread_Add(u8 *self, void *, u32 *out, u32 object) {
    at<u32>(self, 4) = 1;
    at<u32>(self, 8) = object;
    WaitThread_Send(self, nullptr, out);
    return out;
}

u32 *__fastcall WaitThread_Remove(u8 *self, void *, u32 *out, u32 object) {
    at<u32>(self, 4) = 2;
    at<u32>(self, 8) = object;
    WaitThread_Send(self, nullptr, out);
    return out;
}

u32 *__fastcall WaitThread_DoAdd(u8 *self, void *, u32 *out) {
    u32 object = at<u32>(self, 8);
    if (!object) {
        *out = 0x80004003;
        return out;
    }
    WaitThread_InsertObjects(self + 0x34, nullptr, at<u32 *>(self, 0x3c), 1, &object);
    u32 handle = *reinterpret_cast<const u32 *>(static_cast<uintptr_t>(object + 8));
    ImeVector_InsertN(self + 0x24, nullptr, at<u32 *>(self, 0x2c), 1, &handle);
    *out = 0;
    return out;
}

u32 *__fastcall WaitThread_DoRemove(u8 *self, void *, u32 *out) {
    const u32 object = at<u32>(self, 8);
    if (!object) {
        *out = 0x80004003;
        return out;
    }
    u32 *const begin = at<u32 *>(self, 0x38);
    u32 *const end = at<u32 *>(self, 0x3c);
    u32 *found = begin;
    while (found != end && *found != object) ++found;
    if (found == end) {
        *out = 0xcffe0101;
        return out;
    }
    const u32 index = static_cast<u32>(found - begin) + 1;
    u8 *const handles = self + 0x24;
    const u32 size = at<u32 *>(handles, 4) ? static_cast<u32>(static_cast<s32>(at<u32>(handles, 8) - at<u32>(handles, 4)) >> 2) : 0;
    if (size <= index) g_calls.Th(kRangeError, static_cast<u32>(reinterpret_cast<uintptr_t>(handles)));
    eraseAt(handles, at<u32 *>(handles, 4) + index);
    eraseAt(self + 0x34, found);
    *out = 0;
    return out;
}

void __fastcall WaitThread_InsertObjects(u8 *vector, void *, u32 *position, u32 count, const u32 *value) { ImeVector_InsertN(vector, nullptr, position, count, value); }

}  // extern "C"
