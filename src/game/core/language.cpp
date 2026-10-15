#include "language.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Language_SetFontSet(u8 *lang, void *, s32 width) {
    at<u32>(lang, 4) = width <= 0x200 ? 0 : width <= 0x280 ? 1 : width <= 0x320 ? 2 : 3;
}

const char *__fastcall Language_Buffer(const u8 *) { return reinterpret_cast<const char *>(0x00f7f230); }

u32 __fastcall Language_Id(const u8 *lang) { return at<u32>(lang, 8); }

void __fastcall Language_SetId(u8 *lang, void *, u32 id) { at<u32>(lang, 8) = id; }

u32 __fastcall Language_IsHebrew(const u8 *lang) { return at<u32>(lang, 8) == 0x40d; }
}
