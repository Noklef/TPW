#include "selectors.h"

#include <cstdint>

#include "app.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_Get0Shr2(const u8 *object) { return at<u32>(object, 0) >> 2; }

u32 __fastcall Obj_Get0Bit24(const u8 *object) { return at<u32>(object, 0) & 0x1000000; }

bool __cdecl Word7cf4f4_NonZero() { return global<u16>(0x007cf4f4) != 0; }

u32 __fastcall Obj_Bit1At20_16Or8(const u8 *object) { return (at<u8>(object, 0x20) & 0x2) ? 16 : 8; }

u32 __fastcall Obj_Bit6At20_2Or1(const u8 *object) { return (at<u8>(object, 0x20) & 0x40) ? 2 : 1; }

u32 __fastcall Obj_NonZeroAt8_4Or5(const u8 *object) { return at<u32>(object, 8) ? 4 : 5; }

u32 __fastcall Obj_NonZeroAt8_14Or3(const u8 *object) { return at<u32>(object, 8) ? 14 : 3; }

u8 *__cdecl Global7cc53c_Pick7cc4f0() { return global<u8>(0x007cc53c) ? reinterpret_cast<u8 *>(0x007cc4f0) : nullptr; }

u32 __fastcall Obj_Inc30(u8 *object) { return ++at<u32>(object, 0x30); }

u32 __fastcall Obj_IncBefore4c(u8 *object) { return ++at<u32>(object, (u32)-0x4c); }

s32 __cdecl Global7852f8_Times78548c() { return (s32)((u32)global<s32>(0x007852f8) * (u32)global<s32>(0x0078548c)); }

u32 __fastcall Obj_Flags28_Bit1IfBit0(const u8 *object) {
    const u32 flags = at<u32>(object, 0x28);
    return (flags & 1) ? flags & 2 : 0;
}

u32 __stdcall Link_TakeNext(u32 *link) {
    const u32 *node = reinterpret_cast<const u32 *>(*link);
    *link = node[1];
    return node[0];
}

u32 __fastcall Obj_Get14OrInstance(const u8 *object) {
    const u32 value = at<u32>(object, 0x14);
    return value == 0xffffffff ? (u32)(uintptr_t)App_GetInstance() : value;
}

u32 __fastcall Obj_Get1cOrWindow(const u8 *object) {
    const u32 value = at<u32>(object, 0x1c);
    return value == 0xffffffff ? (u32)(uintptr_t)App_GetWindow() : value;
}

u32 __fastcall Obj_Get4Or6fd6c4(const u8 *object) {
    const u32 value = at<u32>(object, 4);
    return value ? value : 0x006fd6c4;
}

u32 __fastcall Obj_Get4Or7089cc(const u8 *object) {
    const u32 value = at<u32>(object, 4);
    return value ? value : 0x007089cc;
}

}  // extern "C"
