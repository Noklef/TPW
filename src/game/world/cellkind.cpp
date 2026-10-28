#include "cellkind.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../files/wrapsources.h"
#include "../terrain/land.h"
#include "cellmodel.h"
#include "mapcell.h"
#include "money.h"
#include "object.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const CellKindEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x004de1f0),
                              reinterpret_cast<u32(__cdecl *)(s32, s32, u32)>((uintptr_t)0x00527ee0)};
const CellKindEnv *g_env = &kGameEnv;

u8 *World() { return global<u8 *>(0x007cf83c); }
u8 *OwnerCell(const u8 *cell) { return World() + 0x294 + (u32)at<u16>(cell, 0x10) * 0x44; }
u8 *ThingAt(u32 handle) { return global<u8 *>(0x007cfb90 + 20 * (handle & 0xffff)); }
u8 *WorldMoney() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(World())); }
u32 WithLowByte(u32 slot, u8 low) { return (slot & 0xffffff00) | low; }

// The directions among the cell's neighbour bits, from 1 round to 1 again by Byte_ShiftLog2 (d, step): each dropped from the cell and the
// next cell that way (when there is one) losing the reverse and getting its model. visit (dir, next) after each. slot is the argument's
// stack slot the direction lives in (its low byte).
template <typename F> void DropNeighbours(u8 *cell, u32 &slot, u32 step, F visit) {
    u8 dir = 1;
    slot = WithLowByte(slot, dir);
    do {
        const u8 bits = MapCell_Neighbours(cell);
        const u32 d = slot;
        if (dir & bits) {
            MapCell_RemoveNeighbours(cell, nullptr, d);
            u8 *const next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, d);
            if (next) {
                MapCell_RemoveNeighbours(next, nullptr, Nibble_Scale(d));
                MapCell_ApplyPathModel(next);
                visit(dir, next);
            }
        }
        dir = (u8)Byte_ShiftLog2(d, step);
        slot = WithLowByte(slot, dir);
    } while (dir != 1);
}

// A type 3 cell's object scrapped: its owner's cell's first object (out to the slot's low word) credited its scrap value times 0x81ea14 / 100.
void Scrap(const u8 *cell, u32 &slot) {
    const u8 *const owner = OwnerCell(cell);
    if (at<u32>(owner, 8) == 0) return;
    Cell_FirstObject(owner, nullptr, reinterpret_cast<u16 *>(&slot));   // (the result not looked at: the slot's word is read)
    const u32 value = Object_ScrapValue(ThingAt(slot));
    const u32 rate = Global81ea14_Get_52fb40();
    Money_Credit(WorldMoney(), nullptr, (s32)(rate * value / 100));
}

// The pair of ints at table (a direction's step: 0x7cdba0...), as IntPair_Copy reads it.
s32 PairX(u32 table) { return global<s32>(table); }
s32 PairY(u32 table) { return global<s32>(table + 4); }

bool PathOrEmpty(const u8 *cell) { return at<u32>(cell, 8) == 1 || at<u32>(cell, 8) == 0; }

// A track cell emptied: no neighbours, direction or flags, kind 0 (MapCell_SetTrackKind), no owner.
void EmptyTrackCell(u8 *cell) {
    MapCell_RemoveNeighbours(cell, nullptr, 0xff);
    MapCell_SetDirection(cell, nullptr, 0);
    at<u16>(cell, 0xe) = 0;
    MapCell_SetTrackKind(cell, nullptr, 0);
    MapCell_SetParent(cell, nullptr, 0);
}

bool LineKind(u32 kind) { return kind == 0xb || kind == 0xd || kind == 0x10 || kind == 0x12 || kind == 0x19; }
u8 *WorldCell28(u16 id) { return World() + ((u32)id + 0x6cde) * 0x28; }

// next (a line cell's neighbour) of kind 0xd / 0x12, or of 0xc / 0x11 with its owner's world cell of those, and of the cell's owner.
bool JoinsLine(const u8 *cell, const u8 *next) {
    const u32 kind = at<u32>(next, 8);
    bool line = false;
    if (kind == 0xc || kind == 0x11) {
        const u8 *const owner = WorldCell28(at<u16>(next, 0x10));
        line = owner && (at<u32>(owner, 8) == 0xd || at<u32>(owner, 8) == 0x12);
    }
    if (!line) line = kind == 0xd || kind == 0x12;
    return line && at<u16>(next, 0x10) == at<u16>(cell, 0x10);
}

// The cell a pair away (Map_StepCell28 (.., 1)) whose owner is the cell emptied; its model when models.
void ClearOwned(const u8 *cell, u32 table, u8 models) {
    u8 *const next = Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, PairX(table), PairY(table), 1);
    if (!next || at<u16>(next, 0x10) != at<u16>(cell, 0)) return;
    MapCell_RemoveNeighbours(next, nullptr, 0xff);
    MapCell_SetDirection(next, nullptr, 0);
    at<u16>(next, 0xe) = 0;
    MapCell_SetTrackKind(next, nullptr, 0);
    MapCell_SetOverlap(next, nullptr, 0);
    if (at<u32>(cell, 8) != 0x19) MapCell_SetParent(next, nullptr, 0);
    if (models) MapCell_ApplyModel(next);
}

// The count's step by (a, b) and the sides or direction dropped; false when the cell is left for the check without its model.
bool CountDown(u8 *cell, s32 a, s32 b, u16 start, u16 end, u32 &slot) {
    if (a == 0 && b == 0 && at<u16>(cell, 0) != start) {
        at<u16>(cell, 0x20) = 0xffff;
        return false;
    }
    at<u16>(cell, 0x20)--;
    if (a == -1 && b == -1) {
        switch (MapCell_Direction(cell)) {   // (the jump tables 0x53bae0 / 0x53bacc)
        case 1:
        case 0x10:
            MapCell_RemoveNeighbours(cell, nullptr, 4);
            MapCell_RemoveNeighbours(cell, nullptr, 0x40);
            break;
        case 4:
        case 0x40:
            MapCell_RemoveNeighbours(cell, nullptr, 1);
            MapCell_RemoveNeighbours(cell, nullptr, 0x10);
            break;
        }
        return true;
    }
    if ((s32)MapCell_SideDirectionCount(cell) >= 2) {
        if (a == -2) MapCell_ClearDirection(cell, nullptr, 4);
        if (a == 2) MapCell_ClearDirection(cell, nullptr, 0x40);
        if (b == -2) MapCell_ClearDirection(cell, nullptr, 0x10);
        if (b == 2) MapCell_ClearDirection(cell, nullptr, 1);
        if (a == 2 || a == -2) {
            MapCell_RemoveNeighbours(cell, nullptr, 4);
            MapCell_RemoveNeighbours(cell, nullptr, 0x40);
        }
        if (b == 2 || b == -2) {
            MapCell_RemoveNeighbours(cell, nullptr, 0x10);
            MapCell_RemoveNeighbours(cell, nullptr, 1);
        }
        return true;
    }
    if (at<u16>(cell, 0) == start || at<u16>(cell, 0) == end) return false;
    slot = WithLowByte(slot, 0);
    const u8 bits = MapCell_Neighbours(cell);
    const bool corner = MapCell_SideNeighbourCount(cell) == 2 && ((bits & 1) || (bits & 0x10)) && ((bits & 4) || (bits & 0x40));
    if (a == -2) MapCell_RemoveNeighbours(cell, nullptr, 4);
    if (a == 2) MapCell_RemoveNeighbours(cell, nullptr, 0x40);
    if (b == -2) MapCell_RemoveNeighbours(cell, nullptr, 0x10);
    if (b == 2) MapCell_RemoveNeighbours(cell, nullptr, 1);
    if (corner && MapCell_NeighbourCount(cell) == 1) MapCell_SetDirection(cell, nullptr, Nibble_Scale(MapCell_Neighbours(cell)));
    return true;
}

// Emptied (the count 0): the scrap credit, the neighbours dropped both ways, the env's 0x527ee0 for flag 0x10, then cleared, its owned cells too.
void Removed(u8 *cell, u32 &slot, u32 price, u8 models) {
    at<u16>(cell, 0x20) = 0;
    if (at<u32>(cell, 8) != 0x19 && !global<u32>(0x00785a2c)) {
        const u8 *const c44 = MapHandle_Cell44(reinterpret_cast<const u16 *>(WorldCell28(at<u16>(cell, 0x10))));
        Cell_FirstObject(c44, nullptr, reinterpret_cast<u16 *>(&slot));   // (the slot's word read, found or not)
        const u32 value = Object_ScrapValue(ThingAt(slot));
        Money_Credit(WorldMoney(), nullptr, (s32)(value * price / 100));
    }
    u8 d = 1;
    slot = WithLowByte(slot, d);
    do {
        const u8 bits = MapCell_Neighbours(cell);
        const u32 dd = slot;
        if (d & bits) {
            MapCell_RemoveNeighbours(cell, nullptr, dd);
            u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, dd, 2);
            if (next) {
                MapCell_RemoveNeighbours(next, nullptr, Nibble_Scale(dd));
                MapCell_ApplyModel(next);
                if (MapCell_NeighbourCount(next) == 1 && MapCell_Direction(next) != MapCell_Neighbours(next) &&
                    MapCell_Direction(next) != (u8)Nibble_Scale(MapCell_Neighbours(next)))
                    MapCell_SetDirection(next, nullptr, Nibble_Scale(MapCell_Neighbours(next)));
            }
        }
        d = (u8)Byte_ShiftLog2(dd, 4);
        slot = WithLowByte(slot, d);
    } while (d != 1);
    if (cell[0xe] & 0x10) {
        slot = (slot & 0xffff0000) | at<u16>(cell, 0);
        const u32 index = (u32)(u16)slot - 1;
        g_env->deadEnds527ee0((s32)(index & 0x7f), (s32)(index >> 7), 1);
    }
    MapCell_SetDirection(cell, nullptr, 0);
    at<u16>(cell, 0xe) = 0;
    MapCell_SetTrackKind(cell, nullptr, 0);
    if (at<u32>(cell, 8) != 0x19) {
        MapCell_SetParent(cell, nullptr, 0);
        at<u16>(cell, 0x24) = 0xffff;
    }
    MapCell_ApplyModel(cell);
    ClearOwned(cell, 0x007cdba8, models);
    ClearOwned(cell, 0x007cdbc8, models);
    ClearOwned(cell, 0x007cdbb8, models);
}

// The line's start cell: each direction not among its bits whose cell two steps out joins the line (JoinsLine) faces it and joins its
// bits (a model), and one not of the line kinds is dropped (a model); a cell left with one neighbour then faces those that join.
void Rejoin(u8 *cell, u32 &slot) {
    u8 d = 1;
    slot = WithLowByte(slot, d);
    do {
        const u8 bits = MapCell_Neighbours(cell);
        const u32 dd = slot;
        if (!(d & bits)) {
            const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, dd, 2);
            if (next) {
                if (JoinsLine(cell, next)) {
                    MapCell_SetDirection(cell, nullptr, Nibble_Scale(dd));
                    MapCell_AddNeighbours(cell, nullptr, dd);
                    MapCell_ApplyModel(cell);
                }
                if (!LineKind(at<u32>(next, 8))) {
                    MapCell_RemoveNeighbours(cell, nullptr, dd);
                    MapCell_ApplyModel(cell);
                }
            }
        }
        d = (u8)Byte_ShiftLog2(dd, 4);
        slot = WithLowByte(slot, d);
    } while (d != 1);
    if (MapCell_NeighbourCount(cell) != 1) return;
    MapCell_ClearDirection(cell, nullptr, 0xff);
    do {
        const u8 bits = MapCell_Neighbours(cell);
        const u32 dd = slot;
        if (d & bits) {
            const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, dd, 2);
            if (next && JoinsLine(cell, next)) MapCell_SetDirection(cell, nullptr, Nibble_Scale(dd));
        }
        d = (u8)Byte_ShiftLog2(dd, 4);
        slot = WithLowByte(slot, d);
    } while (d != 1);
}

}  // namespace

extern "C" {

u32 __fastcall MapCell_SetTrackKind(u8 *cell, void *, u32 kind) {
    u32 price = 0;
    if (kind == 0xb && !global<u8>(0x008186d4)) price = Global81aebc_Get_52fb60();
    if (kind == 0x10 && !global<u8>(0x008186d4)) price = Global81b43c_Get_52fb80();
    MapHandle_Cell44(reinterpret_cast<const u16 *>(cell));   // (its answer dropped)
    const u32 mine = at<u32>(cell, 8);
    if (mine != 0x19 && mine != 7 && !(cell[0xe] & 0x40) && kind != 0 && mine != 0 && kind != mine) return kind == 0x19 ? 1 : 0;
    if (mine == kind) {
        at<u16>(cell, 0x20)++;
        return 1;
    }
    bool affordable = true;
    if (!global<u32>(0x00785a2c))
        affordable = (s32)(Source_Field0c(reinterpret_cast<const TrackedSource *>(WorldMoney())) - price) >= 0;
    if (kind != 0x19 && !affordable && kind != 0 && kind != 7) return 0;
    at<u32>(cell, 8) = kind;
    const u32 count = Counter7cdb98_Inc();
    Map_SetBlockValue(World() + 0x2d8, nullptr, count, at<u16>(cell, 0));   // (the high half: a local's unset stack bytes; the callee reads the low)
    const u32 now = at<u32>(cell, 8);
    if ((now == 0xb || now == 0x10) && global<u32>(0x00820ad0)) {
        u8 *const here = MapHandle_Cell44(reinterpret_cast<const u16 *>(cell));
        if (here) {
            u8 *const east = Map_StepCell(reinterpret_cast<const u16 *>(here), nullptr, PairX(0x007cdbc8), PairY(0x007cdbc8), 1);
            if (east) {
                u8 *const corner = Map_StepCell(reinterpret_cast<const u16 *>(here), nullptr, PairX(0x007cdbb8), PairY(0x007cdbb8), 1);
                if (corner) {
                    u8 *const south = Map_StepCell(reinterpret_cast<const u16 *>(here), nullptr, PairX(0x007cdba8), PairY(0x007cdba8), 1);
                    if (south && PathOrEmpty(here) && PathOrEmpty(east) && PathOrEmpty(corner) && PathOrEmpty(south)) {
                        MapCell_Clear(here, nullptr, 0, 0);
                        MapCell_Clear(east, nullptr, 0, 0);
                        MapCell_Clear(corner, nullptr, 0, 0);
                        MapCell_Clear(south, nullptr, 0, 0);
                    }
                }
            }
        }
    }
    if (kind != 0 && kind != 7 && !global<u8>(0x008186d4)) Money_Debit(WorldMoney(), nullptr, (s32)price);
    return 1;
}

void __fastcall TrackCell_ClearBlock(u8 *cell) {
    EmptyTrackCell(cell);
    static const u32 kSteps[] = {0x007cdba8u, 0x007cdbc8u, 0x007cdbb8u};   // (south, east, south-east)
    for (const u32 table : kSteps) {
        u8 *const next = Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, PairX(table), PairY(table), 1);
        if (next) EmptyTrackCell(next);
    }
}

void __fastcall TrackCell_Reset(u8 *cell) {
    const u32 index = (u32)at<u16>(cell, 0) - 1;
    MapCell_SetTrackKind(cell, nullptr, Terrain_IsParkLand(index & 0x7f, index >> 7) ? 0 : 7);
    MapCell_RemoveNeighbours(cell, nullptr, 0xff);
    global<u8>(0x0081b4cc) = 1;   // (the hoarding neighbours too)
    MapCell_RemoveNeighbours(cell, nullptr, 0xff);
    global<u8>(0x0081b4cc) = 0;
    MapCell_SetDirection(cell, nullptr, 0);
    at<u16>(cell, 0xe) = 0;
    MapCell_SetParent(cell, nullptr, 0);   // (the original's slot: the low word 0 over a stale dword; the callee reads the low)
    at<u32>(cell, 4) = 0;
    at<u16>(cell, 0x20) = 0;
    at<u32>(cell, 0x14) = 0;
    at<u32>(cell, 0x18) = 0;
    at<u32>(cell, 0x1c) = 0;
    at<u16>(cell, 0x24) = 0xffff;
}

void __fastcall TrackCell_Remove(u8 *cell, void *, s32 a, s32 b, u32 models) {
    u32 slotA = (u32)a, slotM = models;   // (a's and models' stack slots, reused)
    u16 start = 0, end = 0;
    u32 price = 0;
    if (at<u32>(cell, 8) == 0xb)
        price = Global81aebc_Get_52fb60();
    else if (at<u32>(cell, 8) == 0x10)
        price = Global81b43c_Get_52fb80();
    if (at<u32>(cell, 8) == 0xc || at<u32>(cell, 8) == 0x11) {
        TrackCell_Remove(WorldCell28(at<u16>(cell, 0x10)), nullptr, a, b, 1);
    } else {
        if (Map_InBounds(global<s32>(0x007639e0), global<s32>(0x007639e4)))
            start = (u16)(((u32)global<s32>(0x007639e4) << 7) + (u32)global<s32>(0x007639e0) + 1);
        if (Map_InBounds(global<s32>(0x007639e8), global<s32>(0x007639ec)))
            end = (u16)(((u32)global<s32>(0x007639ec) << 7) + (u32)global<s32>(0x007639e8) + 1);
        if (LineKind(at<u32>(cell, 8))) {   // (the jump tables 0x53babc / 0x53baa4)
            if (CountDown(cell, a, b, start, end, slotA)) MapCell_ApplyModel(cell);
            const s16 count = at<s16>(cell, 0x20);
            const u16 id = at<u16>(cell, 0);
            const bool clear = (count < 0 && id != start && id != end) || (count < 0 && global<u32>(0x0081b46c)) || global<u32>(0x0081d7a8);
            if (clear) Removed(cell, slotA, price, (u8)models);
        }
    }
    if (at<u16>(cell, 0) == start && LineKind(at<u32>(cell, 8)) && !global<u32>(0x0081b46c)) Rejoin(cell, slotM);
    if (at<u32>(cell, 8) != 0 && !at<u32>(cell, 4)) MapCell_ApplyModel(cell);
}

void CellKind_SetEnv(const CellKindEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __fastcall MapCell_SetKind(u8 *cell, void *, u32 kind) {
    u32 slot = kind;   // (the argument's stack slot, reused)
    bool track = false;
    u32 price = 0;
    if (!Kinds_Compatible(cell, nullptr, kind)) return 0;
    if (kind != 0 && at<u32>(cell, 8) == kind) {
        at<u16>(cell, 0x20)++;
        return 1;
    }
    if (kind == 1) {
        if (at<u32>(cell, 8) == 3) track = true;
        if (!global<u8>(0x008186d4)) price = Global81b6dc_Get_52fb20();
    }
    if (kind == 3) {
        if (!global<u8>(0x008186d4)) price = Global81ea14_Get_52fb40();
        if (at<u32>(cell, 8) == 1) {
            global<u32>(0x0081d7a8) = 1;
            MapCell_Clear(cell, nullptr, 0, 0);
            global<u32>(0x0081d7a8) = 0;
        }
    }
    bool affordable = true;
    if (!global<u32>(0x00785a2c))
        affordable = (s32)(Source_Field0c(reinterpret_cast<const TrackedSource *>(WorldMoney())) - price) >= 0;
    if ((kind == 1 || kind == 3) && !affordable) return 0;
    at<u32>(cell, 8) = kind;
    const u32 count = Counter7cdb98_Inc();
    slot = (slot & 0xffff0000) | at<u16>(cell, 0);
    Map_SetBlockValue(World() + 0x2d8, nullptr, count, slot);
    if (track) {
        const u8 *const owner = OwnerCell(cell);
        const u32 type = at<u32>(owner, 8);
        if (type == 4 || type == 9 || type == 0xa) {
            const u16 handle = *Cell_FirstObject(owner, nullptr, reinterpret_cast<u16 *>(&slot));
            if (handle) g_env->thing4de1f0(ThingAt(handle));
        }
    }
    if (kind == 1 && !global<u8>(0x008186d4)) Money_Debit(WorldMoney(), nullptr, (s32)price);
    if (kind == 3 && !global<u8>(0x008186d4) && at<u32>(cell, 8) != 1) Money_Debit(WorldMoney(), nullptr, (s32)price);
    return 1;
}

void __fastcall MapCell_Clear(u8 *cell, void *, u32 a, u32 b) {
    u32 slot = a;   // (a's stack slot, reused for the direction and an out word)
    if (at<u32>(cell, 8) != 1 || (cell[0xe] & 0x20)) {
        if (at<u32>(cell, 8) == 1 && MapCell_NeighbourCount(cell) == 0) at<u16>(cell, 0xe) &= 0xffdf;
        if ((cell[0xe] & 0x20) && !global<u32>(0x0081d7a8)) return;
    }
    s16 &count = at<s16>(cell, 0x20);
    switch (at<u32>(cell, 8)) {
    case 1:
    case 0xa:
        if (a == 0 && b == 0)
            count = -1;
        else
            count--;
        if (count >= 0 && !global<u32>(0x0081d7a8)) return;
        count = 0;
        DropNeighbours(cell, slot, 2, [cell](u8 dir, u8 *next) {
            if (at<u32>(cell, 8) != 1 || at<u32>(next, 8) != 9) return;
            if (dir != 1 && dir != 4 && dir != 0x10 && dir != 0x40) return;
            if (!at<u16>(next, 0x10)) return;
            u16 out;   // (b's stack slot; nothing reads it after)
            const u16 handle = *Cell_FirstObject(OwnerCell(next), nullptr, &out);
            if (!handle) return;
            u8 *const thing = ThingAt(handle);
            if (thing[2] == 3) g_env->thing4de1f0(thing);
        });
        break;
    case 3:
    case 9: {
        bool reset = a == 0 && b == 0;
        if (!reset) {
            const u8 bits = MapCell_Neighbours(cell);
            if (MapCell_SideNeighbourCount(cell) == 2)
                reset = ((bits & 1) || (bits & 0x10)) && ((bits & 4) || (bits & 0x40));
        }
        if (reset)
            count = -1;
        else
            count--;
        if (global<u32>(0x0081d7a8)) {
            count = 0;
            if (at<u32>(cell, 8) != 9) Scrap(cell, slot);
            break;
        }
        if (count >= 0) return;
        count = 0;
        if (at<u32>(cell, 8) != 9) Scrap(cell, slot);
        DropNeighbours(cell, slot, 4, [](u8, u8 *) {});
        break;
    }
    case 4:
        break;
    case 0x15:
        at<u32>(MapHandle_Entry28(reinterpret_cast<const u16 *>(cell)), 4) = 0;
        break;
    default:
        return;
    }
    MapCell_RemoveNeighbours(cell, nullptr, 0xff);
    MapCell_SetDirection(cell, nullptr, 0);
    MapCell_SetParent(cell, nullptr, 0);
    at<u16>(cell, 0xe) = 0;
    MapCell_SetKind(cell, nullptr, 0);
    MapCell_ApplyPathModel(cell);
}
}
