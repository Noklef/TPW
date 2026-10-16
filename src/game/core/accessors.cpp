#include "accessors.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_Get24(const u8 *object) { return at<u32>(object, 0x24); }

u32 __fastcall Obj_Get2c(const u8 *object) { return at<u32>(object, 0x2c); }

u32 __fastcall Obj_Get30(const u8 *object) { return at<u32>(object, 0x30); }

u32 __fastcall Obj_SetFirst(u8 *object, void *, u32 value) {
    at<u32>(object, 0) = value;
    return value;
}

u32 __fastcall Obj_Get188(const u8 *object) { return at<u32>(object, 0x188); }

u32 __fastcall Obj_Set188(u8 *object, void *, u32 value) {
    at<u32>(object, 0x188) = value;
    return value;
}

u32 __fastcall Obj_Get194(const u8 *object) { return at<u32>(object, 0x194); }

u32 __fastcall Obj_Set194(u8 *object, void *, u32 value) {
    at<u32>(object, 0x194) = value;
    return value;
}

u8 __fastcall Obj_GetByte18c(const u8 *object) { return at<u8>(object, 0x18c); }

void __fastcall Obj_Inc2b8(u8 *object) { at<u32>(object, 0x2b8)++; }

void __fastcall Vtbl6fd818_Set(u8 *object) { at<u32>(object, 0) = 0x006fd818; }

u32 __fastcall Obj_SetByte7(u8 *object, void *, u32, u32 value) {
    at<u8>(object, 7) = (u8)value;
    return value;
}

void __stdcall Stub_Ret16(u32, u32, u32, u32) {}

void __stdcall Stub_Ret4b(u32) {}

u32 __cdecl Global7ca1cc_Get() { return global<u32>(0x007ca1cc); }

u32 __cdecl Global7cb2e8_Get() { return global<u32>(0x007cb2e8); }

u32 __cdecl Global7c2650_Get() { return global<u32>(0x007c2650); }

u32 __cdecl Global7c2654_Get() { return global<u32>(0x007c2654); }

u32 __cdecl Global7c2654_Set(u32 value) {
    global<u32>(0x007c2654) = value;
    return value;
}

u32 __cdecl Global7c2650_Set(u32 value) {
    global<u32>(0x007c2650) = value;
    return value;
}

u32 __cdecl Global8186ac_Get() { return global<u32>(0x008186ac); }

u32 __cdecl Global816d48_Set(u32 value) {
    global<u32>(0x00816d48) = value;
    return value;
}

u32 __cdecl Global816d48_Get() { return global<u32>(0x00816d48); }

u32 __cdecl Global80239c_Get() { return global<u32>(0x0080239c); }

u32 __cdecl Global802ba8_SetMasked(u32 value) {
    global<u32>(0x00802ba8) = value & 0x3ff;
    return value & 0x3ff;
}

void __cdecl Global802bc0_Set1() { global<u32>(0x00802bc0) = 1; }

void __cdecl Global871ee0_Clear() { global<u32>(0x00871ee0) = 0; }

u8 __cdecl Global7c24d0_GetByte() { return global<u8>(0x007c24d0); }
}
