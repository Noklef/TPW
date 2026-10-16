#include "dynfaces.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

constexpr u32 kFaces = 0x00820b00, kCount = 0x00871ee0, kTags = 0x00872168;

void Finish(u32 n, u32 tag) { 
    global<u32>(kTags + 4 * n) = (n << 5) | (tag & 0x1f);
    global<u32>(kCount) = n + 1;
}

}  // namespace

extern "C" {

void __cdecl DynFaces_AddSquare(u32 a, u32 b, u32 c, u8 d, u32 tag, u32 e, u32 f, u32 g, u8 h, u32 i, u32 j) {
    const s32 n = global<s32>(kCount);
    if (n >= 0x806) return;  // (the original prints that there are too many)
    if (tag == 1 && global<u8>(0x00763c98) == 0) return;
    u8 *const face = reinterpret_cast<u8 *>(kFaces + n * 0xa0);
    at<u32>(face, 0) = a;
    at<u32>(face, 4) = b;
    at<u32>(face, 8) = c;
    at<u8>(face, 0xc) = d;
    at<u32>(face, 0x10) = e;
    at<u32>(face, 0x14) = f;
    at<u32>(face, 0x18) = g;
    at<u8>(face, 0x1c) = h;
    at<u8>(face, 0x9c) = (u8)tag;
    at<u32>(face, 0x94) = 1;
    at<u32>(face, 0x98) = global<u32>(0x00874fc4) == 0 ? i : 1;
    at<u32>(face, 0x90) = j;
    Finish((u32)n, tag);
}

void __cdecl DynFaces_AddPoly(const u32 *corners, u32 tag, const u32 *uvs, const u32 *colours, u32 user) {
    const s32 n = global<s32>(kCount);
    if (n >= 0x806) return;
    u8 *const face = reinterpret_cast<u8 *>(kFaces + n * 0xa0);
    for (u32 k = 0; k < 4; ++k) {
        at<u32>(face, 12 * k + 0) = corners[3 * k];
        at<u32>(face, 12 * k + 4) = corners[3 * k + 1];
        at<u32>(face, 12 * k + 8) = corners[3 * k + 2];
        at<u32>(face, 0x30 + 12 * k) = uvs[3 * k];
        at<u32>(face, 0x34 + 12 * k) = uvs[3 * k + 1];
        at<u32>(face, 0x60 + 12 * k) = colours[3 * k];
        at<u32>(face, 0x64 + 12 * k) = colours[3 * k + 1];
        at<u32>(face, 0x68 + 12 * k) = colours[3 * k + 2];
    }
    at<u8>(face, 0x9c) = (u8)tag;
    at<u32>(face, 0x94) = 0;
    at<u32>(face, 0x98) = 0;
    at<u32>(face, 0x90) = user;
    Finish((u32)n, tag);
}

}  // extern "C"
