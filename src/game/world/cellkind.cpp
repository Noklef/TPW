#include "cellkind.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../files/wrapsources.h"
#include "cellmodel.h"
#include "mapcell.h"
#include "money.h"
#include "object.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const CellKindEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x004de1f0)};
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

}  // namespace

extern "C" {

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
