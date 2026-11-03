#include "colourramps.h"

#include <cstdint>
#include <cstring>

#include "palette.h"

namespace {

const u8 *const kPalette = reinterpret_cast<const u8 *>(0x00fa71f8);

// One ramp's colour: argb, then the nearest entry.
void set(u8 *to, u32 red, u32 green, u32 blue) {
    const u32 argb = 0xff000000u | red << 16 | green << 8 | blue;
    std::memcpy(to, &argb, 4);
    to[4] = Palette_Nearest(kPalette, red, green, blue);
}

// The five bytes at +from to where the pointer at +pointer says.
void copy(u8 *object, u32 from, u32 pointer) {
    u8 *to;
    std::memcpy(&to, object + pointer, 4);
    std::memcpy(to, object + from, 4);
    to[4] = object[from + 4];
}

}  // namespace

extern "C" {

void __fastcall ColourRamps_Build(u8 *object) {
    for (u32 i = 0; i < 10; ++i) {
        const u32 v = i * 0xff / 9;
        u8 *const at = object + i * 5;
        set(at, v, 0, 0);
        set(at + 0x38, 0, v, 0);
        set(at + 0x70, 0, 0, v);
        set(at + 0xa8, v, v, 0);
        set(at + 0xe0, v, v, v);
    }
    copy(object, 0x19, 0x34);
    copy(object, 0x51, 0x6c);
    copy(object, 0x89, 0xa4);
    copy(object, 0xc1, 0xdc);
    copy(object, 0x10d, 0x114);
    copy(object, 0xe0, 0x118);
}
}
