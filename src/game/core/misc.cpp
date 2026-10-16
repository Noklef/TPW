#include "misc.h"

#include <cstdint>
#include <initializer_list>

#include "../sprite/script.h"
#include "accessors.h"
#include "tiny_c.h"
#include "tiny_d.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_AdvanceUnless30(u8 *object) {
    u32 result = Obj_Get30(object);
    if (!result) {
        result = at<u32>(object, 0x40);
        at<u32>(object, 0x3c) += result;
    }
    return result;
}

u8 *__fastcall Obj_CtorOne(u8 *object) {
    Init_402f30(object);
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0x24) = 0x3ff00000;
    return object;
}

u8 *__fastcall Obj_CtorValue(u8 *object, void *, u32 value) {
    Fn_657d30(object);
    at<u32>(object, 0) = 0x006ff058;
    at<u32>(object, 0x14) = value;
    return object;
}

void __fastcall Obj_KillSlot(u8 *object) {
    SpriteScript_Kill((s32)at<u32>(object, 0));
    at<u32>(object, 0) = 0;
}

u32 __fastcall Obj_StampAt30(u8 *object, void *, u32 word) {
    at<u16>(object, 0x30) = (u16)word;
    const u32 stamp = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(object, 0x34) = stamp;
    return stamp;
}

u32 __fastcall Obj_StampAt5e(u8 *object, void *, u32 word) {
    at<u16>(object, 0x5e) = (u16)word;
    const u32 stamp = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(object, 0x60) = stamp;
    return stamp;
}

u32 __cdecl Dword_ClearBit0(u32 *p) {
    *p &= 0xfffffffe;
    return 1;
}

u32 __fastcall Obj_Clear18_0c(u8 *object) {
    at<u32>(object, 0x18) = 0;
    at<u16>(object, 0xc) = 0;
    return 0;
}

u32 __fastcall Obj_Diff180_184(const u8 *object) { return at<u32>(object, 0x180) - at<u32>(object, 0x184); }

void __fastcall Vtbl7099ec_SetVirtual(u8 *sub) {
    const u32 base = at<u32>(at<u8 *>(sub, (u32)-4), 4);
    at<u32>(sub, base - 4) = 0x007099ec;
}

u32 __fastcall Wstreambuf_InitPointers(u8 *object) {
    static const u32 kSlots[6][2] = {{0xc, 0x4}, {0x10, 0x8}, {0x1c, 0x14}, {0x20, 0x18}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots)
        at<u32>(object, slot[0]) = (u32)(uintptr_t)object + slot[1];
    for (u32 offset : {0x8u, 0x18u, 0x28u, 0x4u, 0x14u, 0x24u})
        at<u32>(object, offset) = 0;
    return (u32)(uintptr_t)object + 0x24;
}

u32 *__fastcall Wstreambuf_Setg(u8 *object, void *, u32 first, u32 next, u32 last) {
    *at<u32 *>(object, 0xc) = first;
    *at<u32 *>(object, 0x1c) = next;
    u32 *count = at<u32 *>(object, 0x2c);
    *count = (u32)((s32)(last - next) >> 1);
    return count;
}

u32 *__fastcall Wstreambuf_Setp(u8 *object, void *, u32 first, u32 last) {
    *at<u32 *>(object, 0x10) = first;
    *at<u32 *>(object, 0x20) = first;
    u32 *count = at<u32 *>(object, 0x30);
    *count = (u32)((s32)(last - first) >> 1);
    return count;
}

u32 __stdcall SoftRenderer_PixelAt(s32 x, s32 y, u32 *out) {
    const u32 *buffer = at<const u32 *>(global<u8 *>(0x00879dac), 8);
    *out = buffer[(u32)y * global<u32>(0x008bcbd0) + (u32)x];
    return 1;
}

u32 __cdecl Table879d08_Fill() {
    for (u32 i = 0; i < 16; i++)
        global<u32>(0x00879d08 + 4 * i) = 16 - i;
    return 0;
}

u32 __fastcall Obj_Set30_34(u8 *object, void *, u32 a, u32 b) {
    at<u32>(object, 0x30) = a;
    at<u32>(object, 0x34) = b;
    return a;
}
}
