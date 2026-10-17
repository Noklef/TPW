#include "options.h"

#include <cstdint>

#include "../render/mode.h"
#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void copy(u8 *dst, const u8 *src) {
    for (u32 i = 0; i < 0x3c; i += 4) at<u32>(dst, i) = at<u32>(src, i);
}

void set_changed(u8 *options, u32 offset, u32 value) {
    if (at<u32>(options, offset) != value) at<u32>(options, offset) = value;
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Options_Revert(u8 *options) {
    copy(options, options + 0x3c);
    at<u32>(options, 0x94) = 0;
    at<u32>(options, 0x98) &= ~7u;
}

void __fastcall Options_Keep(u8 *options) {
    copy(options + 0x3c, options);
    if (at<s32>(options, 0x94) < 0) at<s32>(options, 0x94) = 0;
    at<u32>(options, 0x98) &= ~7u;
}

void __fastcall Options_Set30(u8 *options, void *, u32 value) { at<u32>(options, 0x30) = value; }
void __fastcall Options_Set4(u8 *options, void *, u32 value) { set_changed(options, 4, value); }
void __fastcall Options_Set8(u8 *options, void *, u32 value) { set_changed(options, 8, value); }
void __fastcall Options_SetC(u8 *options, void *, u32 value) { set_changed(options, 0xc, value); }

void __fastcall Setting_SetMode(u8 *object, void *, u32 mode) {
    at<u32>(object, 0) = mode;
    u32 &flags = global<u32>(0x007a1a8c);
    if (mode == 0)
        flags = (flags & 0xfcffffff) | 0x800000;
    else if (mode == 1)
        flags = (flags & 0xfd7fffff) | 0x1000000;
    else if (mode == 2)
        flags = (flags & 0xfe7fffff) | 0x2000000;
}

u8 *__fastcall Setting_ReadMode(u8 *object) {
    u32 &flags = global<u32>(0x007a1a8c);
    if (flags & 0x2000000) {
        at<u32>(object, 0) = 2;
        flags = (flags & 0xfe7fffff) | 0x2000000;
    } else if (flags & 0x1000000) {
        at<u32>(object, 0) = 1;
        flags = (flags & 0xfd7fffff) | 0x1000000;
    } else {
        at<u32>(object, 0) = 0;
        flags = (flags & 0xfcffffff) | 0x800000;
    }
    return object;
}

u32 __cdecl Graphics_Level() {
    if (RenderMode_Size() == 0x4000) {
        const u32 depth = RenderMode_Depth();
        if (depth == 8) return 8;
        if (depth == 0x10) return 6;
        return depth == 0x20 ? 4 : 0;
    }
    switch (*reinterpret_cast<u32 *>(0x007858d0)) {
    case 0: return 2;
    case 1: return 4;
    case 2: return 6;
    default: return 8;
    }
}
}
