#include "demolish.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../core/soundplay.h"
#include "../math/x87.h"
#include "cellkind.h"
#include "cellrun.h"
#include "gamemode.h"
#include "mapcell.h"
#include "money.h"
#include "object.h"
#include "things.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *cellAt(u32 id) { return global<u8 *>(0x007cf83c) + 0x294 + (id & 0xffff) * 0x44; }
bool lineType(s32 type) { return type == 4 || type == 9 || type == 10; }

template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
const DemolishEnv kGameEnv = {&Path_CheckDeadEnds,
                              &MapCell_Clear,
                              game<void(__cdecl *)(u8 *)>(0x00530120),
                              &GameMode_Set,
                              &GameMode_SetOnly,
                              game<u32(__cdecl *)(u8 *, u32)>(0x0052fe50),
                              reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32)>(&TrackCell_Remove),
                              game<void(__cdecl *)(u32)>(0x00545610)};
const DemolishEnv *g_env = &kGameEnv;

s32 Ftol(double v) { return (s32)(long long)v; }   // (__ftol)

// A footprint cell (column, row) from (x, y), turned by the angle.
void place(s32 angle, s32 x, s32 y, s32 col, s32 row, s32 *px, s32 *py) {
    switch (angle) {
    case 0x5a: *px = x + row; *py = y - col; break;
    case 0xb4: *px = x - col; *py = y - row; break;
    case 0x10e: *px = x - row; *py = y + col; break;
    default: *px = x + col; *py = y + row; break;
    }
}

// The path cell next to a demolished queue cell (from it the direction d): a type 1 one with flag 0x20 loses it when at most one of the
// cells about it (round from d by Byte_ShiftLog2(b, 4), among its neighbour bits) is of type 9 or 0xa and its byte +0x26 lacks bit 3.
void unflag(u8 *next, u8 d) {
    if (at<u32>(next, 8) != 1 || !MapCell_HasFlags(next, nullptr, 0x20)) return;
    s32 queues = 0;
    u8 b = d;
    do {
        if (MapCell_Neighbours(next) & b) {
            const u8 *const other = Map_StepCellInDirection(reinterpret_cast<const u16 *>(next), nullptr, b);
            if (other && (MapCell_IsType9(other) || MapCell_IsTypeA(other))) ++queues;
        }
        b = (u8)Byte_ShiftLog2(b, 4);
    } while (b != d);
    if (queues <= 1 && !(next[0x26] & 8)) MapCell_ClearFlags(next, nullptr, 0x20);
}

u8 *money() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c))); }

}  // namespace

extern "C" {

void Demolish_SetEnv(const DemolishEnv *env) { g_env = env ? env : &kGameEnv; }

u8 __cdecl Map_DemolishRect(s32 x0, s32 y0, s32 x1, s32 y1) {
    const s32 minX = x0 < x1 ? x0 : x1;
    const s32 minY = y0 < y1 ? y0 : y1;
    const s32 maxX = x0 > x1 ? x0 : x1;
    const s32 maxY = y0 > y1 ? y0 : y1;
    for (s32 y = minY; y <= maxY; ++y) {
        const u32 row = ((u32)y << 7) + 1;
        for (s32 x = minX; x <= maxX; ++x) {
            const u8 *const cell = cellAt(row + (u32)x);
            const s32 type = at<s32>(cell, 8);
            if (!(type == 4 || (type > 8 && type <= 10))) continue;
            const u16 owner = at<u16>(cell, 0x10);
            bool ok = true;
            const u8 *c;
            s32 i = x;
            do {   // (right)
                c = cellAt(row + (u32)i);
                if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                if (i > (s32)((u32)maxX + 1)) ok = false;
                ++i;
            } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(i, y));
            if (ok) {
                i = x;
                do {   // (left)
                    c = cellAt(row + (u32)i);
                    if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                    if (i < (s32)((u32)minX - 1)) ok = false;
                    --i;
                } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(i, y));
            }
            if (ok) {
                i = y;
                do {   // (down)
                    c = cellAt(((u32)i << 7) + (u32)x + 1);
                    if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                    if (i > (s32)((u32)maxY + 1)) ok = false;
                    ++i;
                } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(x, i));
            }
            if (ok) {
                i = y;
                do {   // (up)
                    c = cellAt(((u32)i << 7) + (u32)x + 1);
                    if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                    if (i < (s32)((u32)minY - 1)) ok = false;
                    --i;
                } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(x, i));
                if (ok) g_env->demolish(x, y, 0);
            }
        }
    }
    for (s32 y = minY; y <= maxY; ++y) {
        for (s32 x = minX; x <= maxX; ++x) {
            u8 *const c = cellAt(((u32)y << 7) + 1 + (u32)x);
            if (MapCell_IsType1(c)) g_env->cell5367a0(c, nullptr, 0, 0);
        }
    }
    return 1;
}

void __cdecl Path_CheckDeadEnds(s32 x, s32 y, u32 flag) {
    const u32 saved = Global81af64_Get_52f850();
    s32 cx = x, cy = y;
    u16 handle;
    Map_ObjectAtCell(&handle, &cx, &cy);
    if (!handle) return;
    u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
    if (!thing) return;
    const u32 scrap = Object_ScrapValue(thing);
    const s32 angle = at<s32>(thing, 0x10);
    const u8 *const def = Object_GetDef(thing);
    const s32 *const shape = at<const s32 *>(def, 0x18);
    if (at<u32>(def, 0x40)) {
        g_env->thing530120(thing);
        g_env->f52f200(3, 0);
        global<u32>(0x0081d7a8) = 1;
        if (at<s32>(thing, 0x40) > 0) {
            while (g_env->step52fe50(thing, 1)) {}
            const s32 amount = (s32)(Global81ea14_Get_52fb40() * scrap) / 100;
            Money_Debit(money(), nullptr, amount);
        } else {
            Fn_52fbd0();
        }
        global<u32>(0x0081d7a8) = 0;
        const u32 kind = at<u32>(def, 0x9c);
        if (kind != 0) {
            u32 rate = 0;
            if (kind == 1 || kind == 2) {
                const u32 piece = kind == 1 ? 0xb : 0x10;
                TrackRun_TraceA(thing);
                g_env->f52f580(piece, 0);
                global<u32>(0x0081b46c) = 1;
                while (g_env->step52fe50(thing, 0)) {}
                global<u32>(0x0081b46c) = 0;
                TrackRun_TraceB(thing);
                g_env->f52f580(piece, 0);
                global<u32>(0x0081b46c) = 1;
                while (g_env->step52fe50(thing, 0)) {}
                global<u32>(0x0081b46c) = 0;
                rate = (kind == 1 ? Global81aebc_Get_52fb60() : Global81b43c_Get_52fb80()) << 1;
            }
            const s32 amount = (s32)(scrap * rate) / 100;
            Money_Debit(money(), nullptr, amount);
        }
    }
    if (!flag) g_env->f52f200(saved, 0);
    const u8 *const cells = reinterpret_cast<const u8 *>(shape) + 8;
    s32 count = 0;
    for (s32 row = 0; row < shape[1]; ++row) {
        for (s32 col = 0; col < shape[0]; ++col) {
            if (!at<u32>(cells, row * 0xa0 + col * 8)) continue;
            s32 px, py;
            place(angle, cx, cy, col, row, &px, &py);
            if (!Map_InBounds(px, py)) continue;
            u8 *const cell = cellAt(((u32)py << 7) + (u32)px + 1);
            if (MapCell_IsType4(cell)) ++count;
            if (MapCell_IsType9(cell)) {
                const u8 d = MapCell_Direction(cell);
                unflag(Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, Nibble_Scale(d)), d);
            }
            if (MapCell_IsTypeA(cell)) {
                const u8 d = MapCell_Direction(cell);
                unflag(Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, d), d);
            }
        }
    }
    s32 col = x;   // (sic: the sound's x is x as given when there are no rows)
    s32 row;
    for (row = 0; row < shape[1]; ++row) {
        for (col = 0; col < shape[0]; ++col) {
            if (!at<u32>(cells, row * 0xa0 + col * 8)) continue;
            s32 px, py;
            place(angle, cx, cy, col, row, &px, &py);
            if (!Map_InBounds(px, py)) continue;
            u8 *const cell = cellAt(((u32)py << 7) + (u32)px + 1);
            if (MapCell_HasFlags(cell, nullptr, 0x20)) MapCell_ClearFlags(cell, nullptr, 0x20);
            g_env->cell5367a0(cell, nullptr, 0, 0);
            u8 *const entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
            if (flag)
                MapCell_ClearFlags(entry, nullptr, 0x10);
            else
                g_env->entry53b280(entry, nullptr, 0, 0, 1);
        }
    }
    if (!flag && at<u32>(thing, 0x28)) g_env->f545610(at<u32>(thing, 0x28));
    Thing_Delete(thing);
    if (global<u32>(0x00785a2c)) return;
    const float scale = global<float>(0x00700cd8);
    const s32 z = Ftol(X87_MulIntDouble(row, scale));
    const s32 across = Ftol(X87_MulIntDouble(col, scale));
    const u32 sound = count < 4 ? 0x96 : count < 8 ? 0x97 : count < 16 ? 0x98 : 0x99;
    Sound_PlayAt(0, global<u32>(0x00803a2c), sound, across, 0, z);
}
}
