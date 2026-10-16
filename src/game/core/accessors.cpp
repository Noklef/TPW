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

u32 __cdecl Table74fb50_Get(u32 i) { return reinterpret_cast<u32 *>(0x0074fb50)[i]; }

u32 __cdecl Counter7cdb98_Inc() { return ++global<u32>(0x007cdb98); }

u32 __cdecl Nibble_Scale(u32 v) {
    const u8 low = (u8)v;
    if (low == 0)
        return v;
    return low < 0x10 ? (u32)low << 4 : (u32)low >> 4;
}

u32 __cdecl Byte_ShiftLog2(u32 a, u32 b) {
    u8 reduced = (u8)a;
    u32 shift = 0;
    if (reduced > 1) {
        do {
            reduced >>= 1;
            shift++;
        } while (reduced > 1);
    }
    const u32 scaled = (b & 0xff) << shift;
    return (s32)scaled > 0x80 ? scaled >> 8 : scaled;
}

u32 __fastcall MeshPlus60_Ready(u8 *sub) {
    if (!(at<u32>(sub, (u32)-0x10) & 0x10000000)) {
        at<u32>(sub, (u32)-8) = global<u32>(0x00879dec);
        global<u32>(0x00879dec) = (u32)(uintptr_t)(sub - 0x60);
    }
    return 1;
}

u8 *__fastcall Plus28_IfKind2(u8 *sub) {
    return (at<u32>(sub, (u32)-0x10) & 0xf) == 2 ? sub - 0x28 : nullptr;
}

u32 __cdecl Globals7c264c_Load() {
    global<u32>(0x007c264c) = global<u32>(0x007b05a8);
    global<u32>(0x007c2650) = global<u32>(0x007b05ac);
    global<u32>(0x007c2654) = global<u32>(0x007b05b0);
    return global<u32>(0x007b05b0);
}

u8 *__fastcall Class492d80_Ctor(u8 *object) {
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0) = 0x00492d80;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0xffffffff;
    at<u32>(object, 0xc) = 0;
    return object;
}

s32 __cdecl Global7cb28c_Select(s32 which, u32 v) {
    switch (which) {
    case 1:
        global<u32>(0x007cb28c) = v;
        return 0;
    case 2:
        global<u32>(0x007cb2a0) = v;
        return 0;
    case 3:
        global<u32>(0x007cb290) = v;
        return (s32)v;
    default:
        return which - 3;
    }
}
}
