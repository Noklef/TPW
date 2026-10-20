#include "signstyle.h"

#include <cstdint>
#include "array3d.h"
#include "../core/streambuf.h"
#include "../core/vwrap.h"
#include "../files/vfs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// size bytes of the stream into the style at offset (0x5c4f60 with a count of 1).
void Take(u8 *style, u32 offset, u32 size, u8 *stream) { Object_CallSlot8Product((u32)(uintptr_t)(style + offset), size, 1, stream); }

// An effect's fields: four bytes, then four dwords.
void TakeEffect(u8 *style, u32 offset, u8 *stream) {
    for (u32 k = 0; k < 4; ++k)
        Take(style, offset + k, 1, stream);
    for (u32 k = 4; k < 0x14; k += 4)
        Take(style, offset + k, 4, stream);
}

}  // namespace

extern "C" {

u8 __fastcall SignStyle_Load(u8 *style, void *, const char *name) {
    u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(name));   // (0x74cfb8, "rb", pushed too)
    if (!stream)
        return 0;
    Take(style, 0, 4, stream);
    if (at<s32>(style, 0) < 100)
        return 0;   // the stream left open (sic)
    Take(style, 4, 4, stream);
    Take(style, 8, 1, stream);
    Take(style, 0xc, 4, stream);
    Take(style, 0x10, 4, stream);
    Take(style, 0x84, 0x40, stream);
    Take(style, 0xc4, 0x104, stream);
    Take(style, 0x40, 4, stream);
    Take(style, 0x44, 4, stream);
    Take(style, 0x48, 0x3c, stream);
    Eleven_Read(style + 0x358, nullptr, stream);
    Take(style, 0x214, 0x40, stream);
    Take(style, 0x254, 0x104, stream);
    Take(style, 0x1d0, 4, stream);
    Take(style, 0x1d4, 4, stream);
    Take(style, 0x1d8, 0x3c, stream);
    Eleven_Read(style + 0x3c4, nullptr, stream);
    if (at<u32>(style, 0xc) != 0)
        TakeEffect(style, 0x430, stream);
    if (at<u32>(style, 0x10) != 0)
        TakeEffect(style, 0x444, stream);
    Array3D_Read(style + 0x394, nullptr, stream);
    Array3D_Read(style + 0x400, nullptr, stream);
    if (style[8] != 0) {
        if (at<u32>(style, 0) == 100)
            Array3D_Read(style + 0x18, nullptr, stream);
        else
            Array3D_ReadCompressed(style + 0x18, nullptr, stream, at<u8 *>(style, 0x14));
    }
    return 1;   // the stream left open (sic)
}
}
