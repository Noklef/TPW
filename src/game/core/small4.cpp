#include "small4.h"

#include <cstdint>

#include "../world/staff.h"
#include "methods.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Obj_AddrOf4(u8 *object) { return object + 4; }

void __fastcall Vtbl6fd800_Set(u8 *object) { at<u32>(object, 0) = 0x006fd800; }

void __fastcall Vtbl6fdce8_At60_Set(u8 *object) { at<u32>(object, 0x60) = 0x006fdce8; }

u32 __fastcall Obj_GetCOr6fd6c4(const u8 *object) {
    const u32 value = at<u32>(object, 0xc);
    return value ? value : 0x006fd6c4;
}

u32 __fastcall VirtualBase_SetVtbl70994c(u8 *object) {
    const u32 vtable = at<u32>(object, (u32)-8);
    at<u32>(object, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 8) = 0x0070994c;
    return vtable;
}

u8 *__fastcall Array36_At(const u8 *object, void *, u32 index) { return at<u8 *>(object, 4) + 36 * index; }

s32 __cdecl Const_MinusOne_564a50() { return -1; }

s32 __cdecl Const_MinusOne_5ceaa0() { return -1; }

s32 __stdcall Const_MinusOne_5cea90(u32) { return -1; }

s32 __stdcall Const_MinusOne_6bab20(u32, u32, u32, u32) { return -1; }

u32 __cdecl Global786b80_SetAndMode3() {
    const u32 target = global<u32>(0x00f82884);
    global<u32>(0x00786b80) = 1;
    at<u32>(reinterpret_cast<const u8 *>(target), 0x14) = 3;
    return target;
}

s32 __fastcall Staff_WageForRecord(const u8 *record) { return Staff_WageFor(at<u32>(record, 0), at<u8>(record, 8)); }

u32 __cdecl Text_NarrowToGlobalBuffer(const u16 *text) {
    if (const u32 table = global<u32>(0x0078ba80)) {
        u32 status;
        Narrow_FromWide(reinterpret_cast<const u8 *>(table), nullptr, &status, reinterpret_cast<u8 *>(0x007b5378), text, 0x3fff);
    }
    return 0x007b5378;
}

}  // extern "C"
