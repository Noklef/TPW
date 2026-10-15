#include "remap.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

void __cdecl Model_RemapTextures(u8 *model, const s32 *remap, u32 textures, s32 first) {
    auto renumber = [&](s32 v) { return v > 0 ? (u32)first + (u32)v - 1 : ~(u32)v; };  // -1 - v
    u8 *mesh = at<u8 *>(model, 0x70);
    for (u32 i = 0; i < at<u16>(model, 0x44); ++i, mesh += 0xa0) {
        u8 *face = at<u8 *>(mesh, 0x6c);
        for (u32 j = 0; j < at<u16>(mesh, 0x5a); ++j, face += 0x10) {
            const u32 texture = at<u32>(face, 0);
            if (texture) at<u32>(face, 0) = textures + 8 * renumber(remap[(texture - at<u32>(model, 0x50)) >> 3]);
        }
    }
    if (!at<const u8 *>(model, 0x6c)) return;
    for (u32 i = 0; i < at<u16>(at<const u8 *>(model, 0x6c), 6); ++i) {
        u16 &cell = at<u16>(at<const u8 *>(at<const u8 *>(model, 0x6c), 0x2c), 4 * i + 2);
        cell = (u16)renumber(remap[cell]);
    }
}
}
