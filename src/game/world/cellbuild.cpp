#include "cellbuild.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../files/wrapsources.h"
#include "mapcell.h"
#include "object.h"
#include "objectdefs.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 type(const u8 *cell) { return at<u32>(cell, 8); }

bool footprint(u32 t) { return t == 4 || t == 9 || t == 0xa; }

bool owned(const u8 *cell) { return at<u16>(cell, 0x10) == global<u16>(0x00818698); }

// The world's 0x28-byte entry of a parent id.
u8 *parent_entry(u32 id) { return reinterpret_cast<u8 *>(global<u32>(0x007cf83c) + (id + 0x6cde) * 0x28); }

// A type 4 / 9 / 0xa cell's object: the first thing of the world's cell its +0x10 names; one of kind 3 sets 0x818c14 to its definition's
// +0x54 (the original's Debug_Printf assert on the thing table is left out; the slot is read whether set or not).
void object_priority(const u8 *cell) {
    const u8 *owner = reinterpret_cast<const u8 *>(global<u32>(0x007cf83c) + (u32)at<u16>(cell, 0x10) * 0x44 + 0x294);
    u16 handle;
    Cell_FirstObject(owner, nullptr, &handle);
    const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
    if (thing[2] == 3)
        global<u32>(0x00818c14) = at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x54);
}

// The money the world's table entry holds (Source_Field0c).
u32 money() {
    return Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c))));
}

// A track entry (type 0xb, 0xd, 0x10, 0x12 or 0x19) for the cell, as the cell's own entry or its parent's names it; a 0xc / 0x11 entry
// with no parent is none.
u32 track_entry(const u8 *cell) {
    const u8 *entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
    if (MapCell_IsTypeCOr11(entry)) {
        const u16 parent = at<u16>(entry, 0x10);
        if (parent == 0) return 0;
        entry = parent_entry(parent);
    }
    return MapCell_IsTypeBDOr10Or12Or19(entry);
}

}  // namespace

extern "C" {

u32 __fastcall MapCell_CanBuild(u8 *cell, void *, u32 kind, u32 alone) {
    u32 price = 0;
    if (kind == 0x1f) return 2;
    if (kind == 0x3a) return 0xd;
    if (cell[0xe] & 0x20) {
        if (kind == 4 || kind == 9 || kind == 0xa) return 1;
        if (kind == 0x18 && type(cell) == 1) return 0;
    }
    if (kind == 0x18) kind = 1;
    if (kind == 0x36) return global<u32>(0x00816d50) != 0 ? 1 : 0;
    if (footprint(type(cell))) object_priority(cell);
    const u32 blocked = global<u32>(0x00816d50);
    if (blocked == 0 && kind == 0x15 && type(cell) == 0x15 && (cell[0xf] & 1) && owned(cell)) {
        global<u32>(0x00816d4c) = 1;
        return 0xb;
    }
    if (cell[0xe] & 0x40) return 1;
    if (footprint(type(cell))) {
        if (global<s32>(0x00818c10) > global<s32>(0x00818c14) && kind != 0x15 && kind != 0x16 && global<u32>(0x00816d58) == 0 && blocked == 0)
            return 0;
        global<u32>(0x00816d50) = 1;
        return 1;
    }
    if (kind == 1) price = Global81b6dc_Get_52fb20();
    if (kind == 3) price = Global81ea14_Get_52fb40();
    u8 *entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
    if (MapCell_IsTypeCOr11(entry)) entry = parent_entry(at<u16>(entry, 0x10));
    if (kind == 0xb && MapCell_IsTypeBDOr10Or12Or19(entry) && global<u32>(0x008187f8) == 0) {
        global<u32>(0x00816d50) = 1;
        return 1;
    }
    if (kind == 0x10 && MapCell_IsTypeBDOr10Or12Or19(entry) && global<u32>(0x008187f8) == 0) {
        global<u32>(0x00816d50) = 1;
        return 1;
    }
    if (global<u32>(0x00816d58) != 0) global<u32>(0x00816d50) = 1;
    if (alone == 0 && global<u32>(0x00816d50) != 0) return 1;
    if (type(entry) == 0x19) {
        global<u32>(0x00816d50) = 1;
        return 1;
    }
    if (Kinds_Compatible(cell, nullptr, kind) && type(cell) != kind && global<u8>(0x008186d4) == 0 && !(kind == 3 && type(cell) == 1))
        Global81af4c_Add(price);
    if (global<u32>(0x00785a2c) == 0 && Global81af4c_Get_52fbc0() != 0) {
        const s32 have = (s32)money();
        if ((s32)Global81af4c_Get_52fbc0() > have) {
            global<u32>(0x00816d50) = 1;
            Global816d48_Set(1);
            global<u32>(0x00816d5c) = 1;
            return 1;
        }
    }
    if (global<u32>(0x008187f8) != 0 && (kind == 0xb || kind == 0x10)) {
        if ((cell[0xe] & 0x10) == 0 && MapCell_IsTypeBDOr10Or12Or19(entry) && !MapCell_CornerNeighbours(entry) &&
            !MapCell_JunctionNeighbours(entry) && at<u16>(entry, 0x10) == global<u16>(0x00818698) && type(cell) != 3)
            return 0;
        return 1;
    }
    if (kind == 0x15 && track_entry(cell)) return 1;
    if (kind == 0x16 && track_entry(cell)) return 1;
    if (kind == 0x14) {
        if (type(cell) != 3) return 1;
        return owned(cell) ? 0 : 1;
    }
    if (kind == 3 && type(cell) == 3) {
        if (MapCell_NeighbourCount(cell) == 2 && global<u32>(0x008186d0) == 0) {
            global<u32>(0x00816d50) = 1;
            return 1;
        }
        if (!owned(cell)) {
            global<u32>(0x00816d50) = 1;
            return 1;
        }
        if (global<u32>(0x00820ab8) == 0) return 0;
        global<u32>(0x00816d4c) = 1;
        return 0xb;
    }
    u32 t = type(cell);
    if (kind == t && ((global<u32>(0x00820aa4) != 0 && global<u32>(0x00818688) != 0) || global<u32>(0x00820ab8) != 0)) {
        if (t == 3 && MapCell_NeighbourCount(cell) == 2) return 1;
        global<u32>(0x00816d4c) = 1;
        return 0xb;
    }
    const bool lane = kind == 0x15 || kind == 0x16;
    if (lane && (cell[0xe] & 0x20) == 0 && t == 1) return 0;
    if (t == 0x15) {
        if (lane && owned(cell)) return 0;
        global<u32>(0x00816d58) = 1;
        return 1;
    }
    if (kind == 4 && MapCell_IsTypeBDOr10Or12Or19(entry)) return global<u32>(0x008187f8) == 0 ? 1 : 0;
    if (MapCell_CornerNeighbours(entry) || MapCell_JunctionNeighbours(entry)) {
        global<u32>(0x00816d58) = 1;
        return 1;
    }
    if (kind == 1 && type(cell) == 1) return 0;
    if (kind == 0x15 && type(cell) == 0x15) return 0;
    if (kind == 4 && type(cell) == 4) return 1;
    t = type(cell);
    if (kind == t && at<u16>(cell, 0x10) == *global<u16 *>(0x008186c0)) return 0;
    if (t == 0) return 0;
    if (kind == 3) {
        if (t == 1) {
            if (global<u32>(0x00820ab8) != 0 && global<u32>(0x00816d50) == 0) {
                global<u32>(0x00816d60) = 1;
                global<u32>(0x00816d58) = 1;
                return 8;
            }
            if (cell[0xe] & 0x20) {
                global<u32>(0x00816d58) = 1;
                return 1;
            }
            return 0;
        }
        if (t == 4) {
            global<u32>(0x00816d58) = 1;
            return 1;
        }
    }
    if (kind == 1) {
        if (t == 4 || t == 9) {
            global<u32>(0x00816d58) = 1;
            return 1;
        }
        if (t == 3) {
            if (MapCell_NeighbourCount(cell) == 1) {
                const u8 *next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, MapCell_Neighbours(cell));
                if (type(next) == 3) {  // (a null step is read: sic)
                    global<u32>(0x00816d60) = 1;
                    return 8;
                }
            }
            global<u32>(0x00816d50) = 1;
            return 1;
        }
    }
    if ((kind == 4 || kind == 0xb || kind == 0x10) && t == 1) return 0;
    return 1;
}
}
