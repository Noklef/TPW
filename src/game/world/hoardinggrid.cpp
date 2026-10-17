#include "hoardinggrid.h"

#include <cstdint>

#include "mapcell.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The world's 0x28-byte cell of an id.
const u8 *world_cell(u32 id) { return reinterpret_cast<const u8 *>(global<u32>(0x007cf83c) + (id + 0x6cde) * 0x28); }

}  // namespace

extern "C" {

void __cdecl HoardingGrid_Fill() {
    for (s32 y = 0; y < global<s32>(0x0081b1b0); y++) {
        for (s32 x = 0; x < global<s32>(0x0081d740); x++) {
            u8 *c = reinterpret_cast<u8 *>((uintptr_t)(0x0081eeb0 + (y >> 1) + (x >> 1) * 0x42 + (x >> 1)));
            *c = ' ';
            if (!Map_InBounds(x, y)) continue;
            const u8 *cell = world_cell((u16)((y << 7) + x + 1));
            if (MapCell_IsTypeCOr11(cell))
                cell = world_cell(*reinterpret_cast<const u16 *>(cell + 0x10));
            const u8 *cell44 = MapHandle_Cell44(reinterpret_cast<const u16 *>(cell));
            if (MapCell_IsType19(cell))
                *c = '@';
            else if (!MapCell_HasFlags(cell44, nullptr, 0x40) && *c != '@')  // (sic: never '@' here)
                *c = '#';
        }
    }
}
}
