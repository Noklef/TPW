#include "texformat.h"

#include <cstdint>

#include "../gfx/texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall TexLoad_DriverFlags(const u8 *object, void *, const u8 *request, const u8 *texture) {
    const u32 scene = *reinterpret_cast<u32 *>(0x008bcbc8);
    u32 bits = 0;
    if (scene & 0x2000)
        bits = 0x100000;
    if (scene & 0x400)
        bits |= 0x80000;
    const u32 flags = at<u32>(at<const u8 *>(object, 4), 0);
    if (flags & 2)
        bits |= 0x800;
    if (flags & 0x2000000)
        bits |= 0x5156;
    if (flags & 0x8000000)
        bits |= 0x20000;
    const u32 first = at<u32>(request, 0);
    if (at<u8>(request, 0xc) & 2)
        bits |= 0x800000;
    const u32 more = at<u32>(object, 8);
    if (more & 0x10000000)
        bits |= 0x10000000;
    if (more & 0x4000000)
        bits |= 0x30000000;
    if (more & 0x8000000)
        bits |= 0x50000000;
    if (!first || !texture)
        return bits | 0x1256;
    if (at<u8>(reinterpret_cast<const u8 *>(first), 1) & 8)
        bits |= 0x1256;
    else
        bits |= Texture_FormatBits(texture);
    const u32 tex = at<u32>(texture, 8);
    if (tex & 0x100000)
        bits |= 0x40000;
    if (tex & 0x30000)
        bits |= 0x400000;
    if ((tex & 0x400) && at<u32>(texture, 0x3c) == 0)
        bits |= 0x80;
    return bits;
}

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
