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

// The cell a direction pair's step (the two ints at pair) from a cell, scale 1: Map_StepCell's 0x44-byte one, Map_StepCell28's entry.
u8 *step44(const u8 *cell, u32 pair) {
    return Map_StepCell(reinterpret_cast<const u16 *>(cell), nullptr, global<s32>(pair), global<s32>(pair + 4), 1);
}

u8 *step28(const u8 *cell, u32 pair) {
    return Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, global<s32>(pair), global<s32>(pair + 4), 1);
}

// A cell track cannot cross: type 4, 7 / 0x1e, 0xa, 9 or 0x15.
bool unfit(const u8 *cell) {
    return MapCell_IsType4(cell) || MapCell_IsType7Or1e(cell) || MapCell_IsTypeA(cell) || MapCell_IsType9(cell) || type(cell) == 0x15;
}

// A footprint cell's object priority (object_priority) against 0x818c10: false when 0x818c10 is above it (signed); true for other cells.
bool priority_ok(const u8 *cell) {
    if (!footprint(type(cell))) return true;
    object_priority(cell);
    return global<s32>(0x00818c10) <= global<s32>(0x00818c14);
}

// An entry beside the cell that is (or whose parent is) a track type (0xb, 0xd, 0x10, 0x12, 0x19) owned by another player: blocked, and
// 0x816d50 set. A type 0 entry is passed over.
void claim(const u8 *entry, u32 &blocked) {
    if (type(entry) == 0) return;
    if (type(entry) == 0xc || type(entry) == 0x11) entry = parent_entry(at<u16>(entry, 0x10));
    const u32 t = type(entry);
    if ((t == 0xb || t == 0xd || t == 0x10 || t == 0x12 || t == 0x19) && !owned(entry)) {
        blocked = 1;
        global<u32>(0x00816d50) = 1;
    }
}

// Two side neighbours across from each other (1 and 0x10, or 4 and 0x40).
bool straight(const u8 *cell) {
    const u8 nb = MapCell_Neighbours(cell);
    return MapCell_SideNeighbourCount(cell) == 2 && (((nb & 1) && (nb & 0x10)) || ((nb & 4) && (nb & 0x40)));
}

// Two side neighbours at a corner (1 or 0x10 with 4 or 0x40).
bool corner(const u8 *cell) {
    const u8 nb = MapCell_Neighbours(cell);
    return MapCell_SideNeighbourCount(cell) == 2 && (((nb & 1) && (nb & 4)) || ((nb & 1) && (nb & 0x40)) || ((nb & 0x10) && (nb & 4)) ||
                                                     ((nb & 0x10) && (nb & 0x40)));
}

// More than two side neighbours (and some neighbour bit).
bool spread(const u8 *cell) {
    const u8 nb = MapCell_Neighbours(cell);
    return (s32)MapCell_SideNeighbourCount(cell) > 2 && nb != 0;
}

// The direction's other form (Nibble_Scale; its low byte).
u8 turned(const u8 *cell) { return (u8)Nibble_Scale(cell[0xd]); }

// A refusal: 0x816d58 set unless alone; 1.
u32 taken(u32 alone) {
    if (alone == 0) global<u32>(0x00816d58) = 1;
    return 1;
}

// The test of a track cell of the kind's own type facing another way (0xb on 0xb, 0x10 on 0x10): -1 when it does not apply.
s32 facing(const u8 *cell, u8 dir, u32 alone) {
    if (dir == turned(cell) || !owned(cell)) return -1;
    const u8 f = cell[0xe];
    if ((f & 0x10) && dir != cell[0xd]) return (s32)taken(alone);
    if ((f & 0xf) == 0 || dir == cell[0xd]) return 0;
    return (s32)taken(alone);
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

u32 __fastcall MapCell_CanBuildTrack(u8 *cell, void *, u32 kind, u32 alone, u32 direction) {
    const u8 dir = (u8)direction;
    u32 blocked = 0, price = 0;
    if (kind == 0x39) return global<u32>(0x00816d50) != 0 ? 1 : 0;
    u8 *here = MapHandle_Cell44(reinterpret_cast<const u16 *>(cell));
    u8 *n1 = step44(here, 0x007cdbc8), *n2 = step44(here, 0x007cdbb8), *n3 = step44(here, 0x007cdba8);
    if (unfit(here) || unfit(n1) || unfit(n2) || unfit(n3)) blocked = 1;
    if (MapCell_HasFlags(here, nullptr, 0x40) || MapCell_HasFlags(n1, nullptr, 0x40) || MapCell_HasFlags(n2, nullptr, 0x40) ||
        MapCell_HasFlags(n3, nullptr, 0x40))
        return 1;
    if (!priority_ok(here) || !priority_ok(n1) || !priority_ok(n2) || !priority_ok(n2)) return 0;  // (sic: n2 twice, n3 never)
    if (kind == 0xb) price = Global81aebc_Get_52fb60();
    if (kind == 0x10) price = Global81b43c_Get_52fb80();
    if (kind == 0xb || kind == 0x10) {
        const u8 *e1 = step28(cell, 0x007cdbc8), *e2 = step28(cell, 0x007cdbb8), *e3 = step28(cell, 0x007cdba8);
        claim(e1, blocked);
        claim(e2, blocked);
        claim(e3, blocked);
    }
    if (global<u32>(0x00816d58) != 0) global<u32>(0x00816d50) = 1;
    if (alone == 0 && global<u32>(0x00816d50) != 0) return 1;
    u32 t = type(cell);
    if (t == 0x19) {
        if (kind == 0x19) {  // a hoarding on a hoarding: only at either of the two points (0x7639f0, 0x7639f8)
            const u32 index = (u32)at<u16>(cell, 0) - 1;
            const u32 x = index & 0x7f, y = index >> 7;
            if (x == global<u32>(0x007639f0) && y == global<u32>(0x007639f4)) return 1;
            if (x == global<u32>(0x007639f8) && y == global<u32>(0x007639fc)) return 1;
            return 0;
        }
    } else if (kind == 0x19) {  // (the original's test of a type 0x19 cell's direction here is never reached)
        if (t == 0 || t == 7) return 0;
        global<u32>(0x00816d50) = 1;
        return 1;
    }
    if (kind == t && owned(cell)) {
        if (global<u32>(0x00816d54) == 0 || MapCell_NeighbourCount(cell) == 1 || MapCell_NeighbourCount(cell) == 3) global<u32>(0x00816d54) = 1;
    }
    if (kind == type(cell) && global<u32>(0x00820ac8) == 0 && global<u32>(0x00820ad0) == 0) {
        if (!straight(cell) && MapCell_NeighbourCount(cell) != 1) return taken(alone);
    }
    t = type(cell);  // (the original also looks up MapHandle_Cell44 of the cell here and drops it)
    bool add;
    if (t == 0x19 || t == 7 || (cell[0xe] & 0x40))
        add = t != kind;
    else
        add = (kind == 0 || t == 0) && t != kind;
    if (add && global<u8>(0x008186d4) == 0) Global81af4c_Add(price);
    if (global<u32>(0x00785a2c) == 0 && kind != 0xe && kind != 0x13) {
        const s32 have = (s32)money();
        if ((s32)Global81af4c_Get_52fbc0() > have) {
            if (alone == 0) {
                global<u32>(0x00816d50) = 1;
                Global816d48_Set(1);
            }
            global<u32>(0x00816d5c) = 1;
            return 1;
        }
    }
    if (global<u32>(0x00820ad0) != 0 && type(cell) != 0xb && type(cell) != 0x10 && (kind == 0xb || kind == 0x10)) {
        if (MapCell_IsType3Or9(here) || MapCell_IsType3Or9(n1) || MapCell_IsType3Or9(n2) || MapCell_IsType3Or9(n3) ||
            MapCell_HasFlags(here, nullptr, 0x20) || MapCell_HasFlags(n1, nullptr, 0x20) || MapCell_HasFlags(n2, nullptr, 0x20) ||
            MapCell_HasFlags(n3, nullptr, 0x20))
            return taken(alone);
    }
    t = type(cell);
    if (t == 0 && kind != 0xe && kind != 0x13 && blocked == 0) {
        if (alone == 0) global<u32>(0x00816d54) = 0;
        return 0;
    }
    if (kind == t || (kind == 0xb && t == 0xd) || (kind == 0x10 && t == 0x12)) {
        const u32 joined = global<u32>(0x00820ac8);
        if ((joined != 0 && global<u32>(0x00818688) != 0) || global<u32>(0x00820ad0) != 0) {
            if (t == 0xb || t == 0x10) {
                if (dir != cell[0xd] && MapCell_NeighbourCount(cell) == 2) return taken(alone);
                if (corner(cell)) return taken(alone);
            }
            t = type(cell);
            if ((t == 0xd || t == 0x12) && dir != cell[0xd]) return taken(alone);
            if ((cell[0xe] & 0xf) && dir != 0 && dir != cell[0xd] && (global<u32>(0x00820ad0) != 0 || joined != 0)) return taken(alone);
            if (dir != turned(cell) && owned(cell) && (cell[0xe] & 0x10) && dir != cell[0xd]) return taken(alone);
            global<u32>(0x00816d4c) = 1;
            return 0xb;
        }
        if ((cell[0xe] & 0xf) && dir != 0 && dir != cell[0xd] && joined != 0) return taken(alone);
    }
    if (t == 0xb && kind == 0xb) {
        const s32 answer = facing(cell, dir, alone);
        if (answer >= 0) return (u32)answer;
    }
    t = type(cell);
    if (t == 0x12 && kind == 0x10 && dir == cell[0xd]) {
        if (alone == 0) global<u32>(0x00816d58) = 1;
        return 0;
    }
    if (t == 0x10 && kind == 0x10) {
        const s32 answer = facing(cell, dir, alone);
        if (answer >= 0) return (u32)answer;
    }
    t = type(cell);
    if (t == 0xd && kind == 0xb && dir == cell[0xd]) {
        if (alone == 0) global<u32>(0x00816d58) = 1;
        return 0;
    }
    if (blocked != 0 && (cell[0xe] & 0x10) == 0) return taken(alone);
    if (kind == 0xf) {
        if ((t == 0xb || t == 0x10) && owned(cell)) return 0xd;
        return t == 0x19 ? 0xd : 1;
    }
    if (t == 0xb && (kind == 0xe || kind == 0x13) && owned(cell) && !spread(cell)) return 9;
    if (type(cell) == 0x10 && (kind == 0x13 || kind == 0xe) && owned(cell) && !spread(cell)) return 9;
    bool refuse = corner(cell) && global<u32>(0x008186d0) == 0;
    if (!refuse && (cell[0xe] & 0xf)) {
        if (global<u32>(0x00820ac8) == 0)
            refuse = true;
        else if (dir != cell[0xd] && dir != turned(cell))
            refuse = true;
    }
    if (refuse) {
        if (alone == 0) global<u32>(0x00816d50) = 1;
        return 1;
    }
    if (MapCell_NeighbourCount(cell) == 2 && dir != cell[0xd] && dir != turned(cell)) global<u32>(0x00816d54) = 0;
    if (kind == type(cell) && owned(cell) && MapCell_NeighbourCount(cell) == 3) return 0;
    return 1;
}
}
