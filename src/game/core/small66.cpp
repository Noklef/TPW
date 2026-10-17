#include "small66.h"

#include <cstdint>

#include "../world/mapcell.h"
#include "accessors.h"
#include "small10.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *CellAt(s32 x, s32 y) {
    const u32 id = (((u32)y << 7) + (u32)x + 1) & 0xffff;
    return global<u8 *>(0x007cf83c) + 0x294 + id * 0x44;
}

}  // namespace

extern "C" {

u8 *__cdecl Cell_ExtendPath(const s32 *point, s32 *nextX, s32 *nextY) {
    if (!Map_InBounds(point[0], point[1]))
        return nullptr;
    const u8 direction = at<u8>(point, 0x10);
    MapCell_AddNeighbours(CellAt(point[0], point[1]), nullptr, Nibble_Scale(direction));
    *nextX = point[0];
    const s32 y = point[1];
    *nextY = y;
    switch (direction) {
    case 1: *nextY = y + 1; break;
    case 0x10: *nextY = y - 1; break;
    case 4: *nextX = *nextX - 1; break;
    case 0x40: *nextX = *nextX + 1; break;
    default: break;
    }
    if (!Map_InBounds(*nextX, *nextY))
        return nullptr;
    u8 *cell = CellAt(*nextX, *nextY);
    if (Kinds_Compatible(cell, nullptr, 3))
        MapCell_SetDirection(cell, nullptr, direction);
    return cell;
}

}  // extern "C"
