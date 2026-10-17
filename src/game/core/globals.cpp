#include "globals.h"

#include <cstdint>
#include <initializer_list>

#include "../render/engineerrors.h"
#include "../world/object.h"
#include "accessors.h"
#include "crt.h"
#include "gameheap.h"
#include "string.h"
#include "vcall.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Global8bb628_SetOne() {
    global<u32>(0x008bb628) = 0x3f800000;
    return 0x3f800000;
}

u32 __cdecl Global8bba78_SetOne() {
    global<u32>(0x008bba78) = 0x3f800000;
    return 0x3f800000;
}

u32 __cdecl Globals8bcb80_SetOne() {
    global<u32>(0x008bcb8c) = 0x3f800000;
    global<u32>(0x008bcb84) = 1;
    global<u32>(0x008bcb80) = 1;
    global<u32>(0x008bcb88) = 1;
    return 1;
}

u32 __fastcall Id_Get(u32 *id) {
    if (*id == 0) {
        const u32 next = global<u32>(0x00fb1dd4) + 1;
        global<u32>(0x00fb1dd4) = next;
        *id = next;
    }
    return *id;
}

void __cdecl Global7ced44_Release() {
    u8 *shared = global<u8 *>(0x007ced44);
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count != 0xffffffffu)
        at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0)
        vcall<u32>(shared, 0, 1u);
    global<u32>(0x007ced44) = 0;
}

void __cdecl Globalf827dc_Release() {
    u8 *shared = global<u8 *>(0x00f827dc);
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count != 0xffffffffu)
        at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0)
        vcall<u32>(shared, 0, 1u);
    global<u32>(0x00f827dc) = 0;
}

u32 __cdecl Global879a24_Free() {
    u8 *object = global<u8 *>(0x00879a24);
    if (!object)
        return 0;
    Obj_ReleaseSevenComMembers(object);
    object = global<u8 *>(0x00879a24);
    if (object) {
        Obj_ReleaseSevenComMembers(object);
        GameHeap_Free(object);
    }
    global<u32>(0x00879a24) = 0;
    return 1;
}

void __cdecl Globals816d40_Free() {
    if (!global<u32>(0x00816d40))
        return;
    global<u32>(0x0080ced0) = 0;
    Mem_FreeChecked(global<void *>(0x0080cef8));
    const u32 other = global<u32>(0x00816d38);
    global<u32>(0x00816d40) = 0;
    Mem_FreeChecked((void *)(uintptr_t)other);
}

void __fastcall Obj_ApplyToGlobalObject(u8 *object) {
    u8 *game = global<u8 *>(0x007c2658);
    const u32 returned = Set198_4e21e0(game, nullptr, at<u32>(object, 0x28));
    const u32 product = (u32)(u16)(s16)((s32)at<s8>(object, 0x24) * 0x32);
    Object_SetQualityOfGoods(game, nullptr, (returned & 0xffff0000u) | product);
    Obj_Set194(game, nullptr, at<u32>(object, 0x2c));
}

void __cdecl Table879ad0_Clear() {
    for (u32 address = 0x00879ad0, i = 0; i < 0x40; ++i, address += 8) {
        global<u32>(address) = 0;
        global<u32>(address + 4) = 0;
    }
    global<u32>(0x00879cd0) = 0;
}

u32 __cdecl Global81ae2c_Equals(u32 value) { return global<u32>(0x0081ae2c) == value; }

u32 __cdecl Global81b0cc_IfNoFlag() { return global<u32>(0x008187f8) ? 0 : global<u32>(0x0081b0cc); }

u32 __cdecl Global81ecac_IfNoFlag() { return global<u32>(0x008187f8) ? 0 : global<u32>(0x0081ecac); }

u32 __cdecl Global81af4c_Add(u32 amount) {
    global<u32>(0x0081af4c) += amount;
    return amount;
}

u32 __cdecl Global879a24_Get4c4() { return at<u32>(reinterpret_cast<const u8 *>(global<u32>(0x00879a24)), 0x4c4); }

u32 __cdecl Global786b80_SetAndMode3() {
    const u32 target = global<u32>(0x00f82884);
    global<u32>(0x00786b80) = 1;
    at<u32>(reinterpret_cast<const u8 *>(target), 0x14) = 3;
    return target;
}

u8 __stdcall Bytes785058_Get(u32 index) {
    switch (index) {
    case 0: return *reinterpret_cast<u8 *>(0x00785058);
    case 1: return *reinterpret_cast<u8 *>(0x0078505c);
    case 2: return *reinterpret_cast<u8 *>(0x00785060);
    default: return 0;
    }
}

u32 __cdecl Global7b4b88_Get_47eaa0() {
    return global<u32>(0x007b4b88);
}

u32 __cdecl Global7cc2f0_Get_4b4480() { return global<u32>(0x007cc2f0); }

u32 __cdecl Global785584_Get_519410() {
    return global<u32>(0x00785584);
}

u32 __cdecl Global785588_Get_519420() {
    return global<u32>(0x00785588);
}

u32 __cdecl Global78558c_Get_519430() {
    return global<u32>(0x0078558c);
}

u32 __cdecl Global785590_Get_519440() {
    return global<u32>(0x00785590);
}

u32 __cdecl Global81d7a4_Get_52f1f0() {
    return global<u32>(0x0081d7a4);
}

u32 __cdecl Global81ae2c_Get_52f840() {
    return global<u32>(0x0081ae2c);
}

u32 __cdecl Global81af64_Get_52f850() {
    return global<u32>(0x0081af64);
}

u32 __cdecl Global81b6dc_Set_52fb10(u32 v0) {
    global<u32>(0x0081b6dc) = v0;
    return v0;
}

u32 __cdecl Global81b6dc_Get_52fb20() {
    return global<u32>(0x0081b6dc);
}

u32 __cdecl Global81ea14_Set_52fb30(u32 v0) {
    global<u32>(0x0081ea14) = v0;
    return v0;
}

u32 __cdecl Global81ea14_Get_52fb40() {
    return global<u32>(0x0081ea14);
}

u32 __cdecl Global81aebc_Set_52fb50(u32 v0) {
    global<u32>(0x0081aebc) = v0;
    return v0;
}

u32 __cdecl Global81aebc_Get_52fb60() {
    return global<u32>(0x0081aebc);
}

u32 __cdecl Global81b43c_Set_52fb70(u32 v0) {
    global<u32>(0x0081b43c) = v0;
    return v0;
}

u32 __cdecl Global81b43c_Get_52fb80() {
    return global<u32>(0x0081b43c);
}

u32 __cdecl Global81af4c_Set_52fb90(u32 v0) {
    global<u32>(0x0081af4c) = v0;
    return v0;
}

u32 __cdecl Global81af4c_Get_52fbc0() {
    return global<u32>(0x0081af4c);
}

u32 __cdecl Global820a8c_Get_52fbe0() {
    return global<u32>(0x00820a8c);
}

u32 __cdecl Global879088_Set_5508b0(u32 v0) {
    global<u32>(0x00879088) = v0;
    return v0;
}

u32 __cdecl Global781fc8_Get_691530() {
    return global<u32>(0x00781fc8);
}

u32 __cdecl Globalfb2708_Set_6d1bc0(u32 v0) {
    global<u32>(0x00fb2708) = v0;
    return v0;
}

u32 __cdecl FreeGlobal879a20_561a00() {
    const u32 p = global<u32>(0x00879a20); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x00879a20) = 0; return 1;
}

u32 __cdecl CreateGlobal879d54_56b6c0() {
    u8 *&block = *reinterpret_cast<u8 **>((uintptr_t)0x00879d54);
    if (block) return 0;
    u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0x2d8));
    if (p) {
        for (const u32 offset : {0x30u, 0x34u, 0x3cu, 0x40u, 0x44u, 0x48u, 0x4cu, 0x50u, 0x54u, 0x58u, 0x5cu, 0x60u, 0x6cu, 0xc0u, 0xc4u, 0xc8u, 0x1ccu,
                                 0x1d0u})
            *reinterpret_cast<u32 *>(p + offset) = 0;
    }
    block = p;
    if (!p) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __cdecl FreeGlobal879d54_56b760() {
    const u32 p = global<u32>(0x00879d54); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x00879d54) = 0; return 1;
}

u32 __cdecl FreeGlobal87a248_5740e0() {
    const u32 p = global<u32>(0x0087a248); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x0087a248) = 0; return 1;
}

u32 __cdecl FreeGlobal8bcbb8_580500() {
    const u32 p = global<u32>(0x008bcbb8); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x008bcbb8) = 0; return 1;
}

u32 __cdecl FreeGlobal8c47d4_58b040() {
    const u32 p = global<u32>(0x008c47d4); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x008c47d4) = 0; return 1;
}
}  // extern "C"
