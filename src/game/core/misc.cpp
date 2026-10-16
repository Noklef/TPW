#include "misc.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

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
