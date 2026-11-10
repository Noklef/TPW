// The track circuit walked: see trackcircuit.h.
#include "trackcircuit.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../model/parts.h"
#include "cellmodel.h"
#include "mapcell.h"
#include "object.h"
#include "thingscan.h"
#include "trackrides.h"
#include "focuscell.h"
#include "rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TrackCircuitEnv kGameEnv = {&TrakRide_Strip, &TrakRide_AddSection, &MapCell_ApplyModel};
const TrackCircuitEnv *g_env = &kGameEnv;

u8 *cell28(u32 id) { return global<u8 *>(0x007cf83c) + ((id & 0xffff) + 0x6cde) * 0x28; }
u8 *slot(u32 handle) { return global<u8 *>(0x007cfb90 + 20 * handle); }

// FILD, FMUL 3072.0f (0x700cdc), __ftol: the product is exact, its low dword returned.
s32 scaled(s32 v) { return (s32)((u32)v * 3072u); }

u8 *step(const u8 *cell, u32 direction) { return Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2); }

// A side of an upgrade: the cell two on a track cell with a thing, or of type 0xd or 0x12.
bool side(const u8 *cell) {
    if (cell == nullptr) return false;
    if (MapCell_IsTypeBOr10(cell) && at<u32>(cell, 4) != 0) return true;
    return at<u32>(cell, 8) == 0xd || at<u32>(cell, 8) == 0x12;
}

const u8 *holder(u32 thing) { return gRides_7a4610[thing]; }

// A part's flags: the model's face for it (the first +0x44 0xa0-byte faces at +0x70, the rest 0x58-byte at +0x74, from +0x46).
u32 &part_flags(u32 thing, u32 part) {
    const u8 *model = at<const u8 *>(at<const u8 *>(holder(thing), 8), 4);
    const u32 index = at<u16>(model, 0x46) + part, first = at<u16>(model, 0x44);
    return index < first ? at<u32>(at<u8 *>(model, 0x70), index * 0xa0) : at<u32>(at<u8 *>(model, 0x74), (index - first) * 0x58);
}

u32 find(u32 thing, u32 key) { return Model_FindPartByKey8(holder(thing), 1, key); }
void mark(u32 thing, u32 part) {
    if (part != 0xffffffff) part_flags(thing, part) |= 0x80000010;
}
void clear(u32 thing, u32 part) {
    if (part != 0xffffffff) part_flags(thing, part) &= ~0x10u;
}

// The second upgrade's parts keyed 1 and 0x10 by its sides.
void upgrade(u32 thing, u32 sides) {
    switch (sides) {
    case 0: {
        clear(thing, find(thing, 1));
        clear(thing, find(thing, 0x10));
        break;
    }
    case 1: {
        const u32 a = find(thing, 1), b = find(thing, 0x10);
        mark(thing, a);
        clear(thing, b);
        break;
    }
    case 0x10: {
        const u32 a = find(thing, 0x10), b = find(thing, 1);
        mark(thing, a);
        clear(thing, b);
        break;
    }
    case 0x11: {
        mark(thing, find(thing, 1));
        mark(thing, find(thing, 0x10));
        break;
    }
    default: break;
    }
}

u16 object_at(s32 x, s32 y) {
    u16 handle;
    Map_ObjectOrEntranceAt(&handle, x, y);
    return handle;
}

}  // namespace

extern "C" {

void TrackCircuit_SetEnv(const TrackCircuitEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __cdecl TrackCircuit_Walk(u32 check) {
    const TrackCircuitEnv *const E = g_env;
    const bool build = (u8)check == 0;
    u8 direction = gFocuscell_818c50;
    s32 upgrades = 0, index = 0;
    u32 mesh = 0, def = 0, result = 1, sides = 0;
    s32 x0 = global<s32>(0x007639e0), y0 = global<s32>(0x007639e4);
    switch (global<s32>(0x007639ac)) {
    case 0: x0 -= 4; break;
    case 0x5a: y0 += 4; break;
    case 0xb4: x0 += 4; break;
    case 0x10e: y0 -= 4; break;
    default: break;
    }
    s32 x = x0, y = y0;
    const u8 *start = cell28(((u32)y << 7) + (u32)x + 1);
    if (MapCell_IsTypeCOr11(start)) start = cell28(at<u16>(start, 0x10));
    if (!MapCell_IsTypeBDOr10Or12Or19(start)) return 0;
    u16 first;
    Cell_FirstObject(MapHandle_Cell44(reinterpret_cast<const u16 *>(cell28(at<u16>(start, 0x10)))), nullptr, &first);
    const u32 ride = at<u32>(slot(first), 0x28);   // (the original's Debug_Printf assert on the slot table left out)
    if (build) E->strip(ride);
    for (;;) {
        u8 *const cell = cell28(((u32)y << 7) + (u32)x + 1);   // (and MapHandle_Cell44 of it, unused)
        if (!MapCell_IsTypeBDOr10Or12Or19(cell)) {
            direction = 0;
        } else {
            if (MapCell_CornerNeighbours(cell)) {
                if (build) {
                    u32 piece = 0;
                    switch (MapCell_CornerNeighbours(cell)) {
                    case 5: piece = 5; break;
                    case 0x14: piece = 8; break;
                    case 0x41: piece = 6; break;
                    case 0x50: piece = 7; break;
                    default: break;
                    }
                    if (piece) {
                        const s32 sy = scaled(y), sx = scaled(x);
                        E->addSection(ride, sx, sy, piece, 0, 0, 0);
                    }
                    upgrades = 0;
                    mesh = 0;
                }
            } else if (MapCell_JunctionNeighbours(cell)) {
                if (build) {
                    const s32 sy = scaled(y), sx = scaled(x);
                    E->addSection(ride, sx, sy, 0xb, 0, 0, 0);
                    upgrades = 0;
                    mesh = 0;
                }
            } else if (MapCell_StraightNeighbours(cell)) {
                u32 piece = index < 2 ? 0x10000 : 0;
                if (MapCell_HasFlags(cell, nullptr, 0x10)) {
                    const u16 handle = object_at(x, y);
                    if (handle != 0) {
                        const u8 *object = slot(handle);
                        if (object != nullptr) {
                            mesh = at<u32>(object, 0x20);
                            def = at<u32>(Object_GetDef(object), 0x10);
                        }
                    }
                } else {
                    upgrades = 0;
                    mesh = 0;
                }
                if (build) {
                    const u8 bits = MapCell_StraightNeighbours(cell);
                    if (bits == 0x11 || bits == 0x44) {
                        const bool flagged = MapCell_HasFlags(cell, nullptr, 0x10) != 0;
                        piece |= bits == 0x11 ? (flagged ? 0xd : 0xa) : (flagged ? 0xc : 9);
                        const s32 sy = scaled(y), sx = scaled(x);
                        E->addSection(ride, sx, sy, piece, (u32)upgrades, mesh, def);
                    }
                }
            }
            if (!MapCell_HasFlags(cell, nullptr, 0x10)) {
                if (at<u32>(cell, 4) == 0 && MapCell_IsTypeBOr10(cell)) E->applyModel(cell);
            } else {
                const u32 own = (u32)at<u16>(cell, 0) - 1;
                if (++upgrades == 0) sides = 0;
                const u16 handle = object_at((s32)(own & 0x7f), (s32)(own >> 7));   // (and a Debug_Printf)
                const u8 *object = handle != 0 ? slot(handle) : nullptr;
                if (object != nullptr) {
                    const u32 thing = at<u32>(object, 0x20);
                    if (side(step(cell, MapCell_Direction(cell)))) sides |= 0x10;
                    if (side(step(cell, Nibble_Scale(MapCell_Direction(cell))))) sides |= 1;
                    if (upgrades == 2) upgrade(thing, sides);
                }
            }
            if (MapCell_CornerNeighbours(cell)) {
                u8 turn = (u8)Nibble_Scale(direction);
                if (!(turn & MapCell_Neighbours(cell))) turn = direction;
                direction = MapCell_Neighbours(cell) ^ turn;
                if (MapCell_Direction(cell) != direction) {
                    MapCell_SetDirection(cell, nullptr, direction);
                    E->applyModel(cell);
                }
            }
        }
        if (direction == 0 || (s32)MapCell_NeighbourCount(cell) < 2) result = 0;
        if (MapCell_NeighbourCount(cell) == 3) {
            if (MapCell_IsTypeBDOr10Or12Or19(step(cell, direction)) && (direction & (MapCell_Neighbours(step(cell, direction)) == 0 ? 1 : 0)))
                result = 0;
            if (at<u32>(step(cell, direction), 8) == 0) result = 0;
        }
        MapCell_SetWord24(cell, nullptr, (u32)index);
        ++index;
        switch (direction) {
        case 1: y -= 2; break;
        case 4: x += 2; break;
        case 0x10: y += 2; break;
        case 0x40: x -= 2; break;
        default: break;
        }
        if ((x == x0 && y == y0) || result == 0) return result;
    }
}

}  // extern "C"
