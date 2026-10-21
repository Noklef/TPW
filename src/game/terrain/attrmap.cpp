#include "attrmap.h"

#include <cstdint>

namespace {
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}  // namespace

extern "C" {

u32 __fastcall AttrMapHandler_MatchTag(u8 *handler, void *, u32 tag) { return tag == at<u32>(handler, 4) ? 1 : 0; }

void __fastcall AttrMap_ReadMapChunk(u8 *handler, void *, const u8 *chunk, u32) {
    const s32 w = at<s32>(chunk, 0), h = at<s32>(chunk, 4);
    const u8 *data = chunk + 0x1c;
    const s32 x0 = *reinterpret_cast<const s32 *>(0x00785584), y0 = *reinterpret_cast<const s32 *>(0x00785588);
    for (s32 r = 0; r < h; ++r) {
        const s32 x = r - x0;
        for (s32 c = 0; c < w; ++c, ++data) {
            const s32 y = c - y0;
            if (x < 0 || x >= 0x80 || y < 0 || y >= 0x80) continue;
            const u32 id = (u16)((y << 7) + x + 1);
            at<u8>(at<u8 *>(handler, 8), id * 0x44 - 0x1e) = *data;
        }
    }
}
}
