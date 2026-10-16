#include "texformat.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall TexFormat_SetArgb32(u8 *settings) {
    at<u32>(settings, 0x6c) = 0x41;
    at<u32>(settings, 0x68) = 0x20;
    at<u32>(settings, 0x70) = 0;
    at<u32>(settings, 0x74) = 0x20;
    at<u32>(settings, 0x78) = 0xff0000;
    at<u32>(settings, 0x7c) = 0xff00;
    at<u32>(settings, 0x80) = 0xff;
    at<u32>(settings, 0x84) = 0xff000000;
}

u8 *__fastcall TexFormat_Table(u8 *settings) { return settings + 0x88 + (at<u32>(settings, 4) << 12); }
}
