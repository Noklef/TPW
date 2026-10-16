#include "options.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void copy(u8 *dst, const u8 *src) {
    for (u32 i = 0; i < 0x3c; i += 4) at<u32>(dst, i) = at<u32>(src, i);
}

void set_changed(u8 *options, u32 offset, u32 value) {
    if (at<u32>(options, offset) != value) at<u32>(options, offset) = value;
}

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
}
