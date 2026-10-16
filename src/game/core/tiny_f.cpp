#include "tiny_f.h"

#include <cstdint>

#include "accessors.h"
#include "forwarders.h"
#include "misc.h"
#include "small.h"
#include "tiny_a.h"
#include "tiny_c.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Wrap_Obj_Get30_402df0(u8 *o) {
    return Obj_Get30(reinterpret_cast<const u8 *>(o));
}

u32 __cdecl Wrap_Flags7a1a14_Update_40f220() {
    return Flags7a1a14_Update(reinterpret_cast<const u8 *>(reinterpret_cast<u8 *>(0x00786b68)));
}

u32 __cdecl Wrap_Dword_ClearBit0_454540() {
    return Dword_ClearBit0(reinterpret_cast<u32 *>(0x74d008));
}

u32 __cdecl Wrap_Global7c2650_Get_481590() {
    return Global7c2650_Get();
}

u32 __cdecl Wrap_Global7c2654_Get_4815a0() {
    return Global7c2654_Get();
}

u32 __stdcall Wrap_Global7c2654_Set_4815b0(u32 v0) {
    return Global7c2654_Set(v0);
}

u32 __cdecl Wrap_Obj_Get64Plus10_5f5640() {
    return Obj_Get64Plus10(reinterpret_cast<const u8 *>(reinterpret_cast<u8 *>(0x00fa6e40)));
}

u32 __cdecl Wrap_Owner64_Copy4At34_5f5650(u32 v0) {
    return Owner64_Copy4At34(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fa6e40)), nullptr, reinterpret_cast<const u32 *>((uintptr_t)v0));
}

u8 * __cdecl Wrap_Init_691ee0_691ed0() {
    return Init_691ee0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ad0)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_691ee0_691f20() {
    return Init_691ee0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb19a8)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_691ee0_692100() {
    return Init_691ee0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1a38)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_692970_692960() {
    return Init_692970(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1c70)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_692970_6929b0() {
    return Init_692970(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1c18)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_692970_6929f0() {
    return Init_692970(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1d00)), nullptr, 0x0);
}

u32 __stdcall Wrap_Globalfb2708_Set_6d1bc0_6baa10(u32 v0) {
    return Globalfb2708_Set_6d1bc0(v0);
}

u8 * __cdecl Wrap_Init_6c4fb0_6c4900() {
    return Init_6c4fb0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb2038)));
}

}
