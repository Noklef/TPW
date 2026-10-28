// A map cell's messages: see cellmessage.h.
#include "cellmessage.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../math/x87.h"
#include "../render/dynfaces.h"
#include "../terrain/land.h"
#include "cellbuild.h"
#include "cellkind.h"
#include "cellmodel.h"
#include "cellrun.h"
#include "demolish.h"
#include "gamemode.h"
#include "lineends.h"
#include "mapcell.h"
#include "pathlink.h"
#include "thingscan.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const CellMessageEnv kGameEnv = {&Path_CheckDeadEnds, &GameMode_SetOnly};
const CellMessageEnv *g_env = &kGameEnv;

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

s32 ftol(double x) { return (s32)(long long)x; }   // (__ftol)

// The ground at (x, y) rounded up to tens: ((__ftol(height) + 9) / 10, signed) times the float 0x700d54 (fild, fmul).
float rounded(s32 x, s32 y) {
    const s32 tens = (s32)((u32)ftol(Terrain_SurfaceHeight(x, y, nullptr)) + 9) / 10;
    return (float)(X87Reg((double)tens) * global<float>(0x00700d54));
}

// A cell's height: the rounded ground when its entry's type is 0xb, 0xc, 0x10, 0x11 or 0x19, or its own type 7, 0x1e, 4, 9 or 0xa; else 0.
float height_of(const u8 *cell, s32 x, s32 y) {
    float h = 0.0f;
    const u32 kind = at<u32>(MapHandle_Entry28(reinterpret_cast<const u16 *>(cell)), 8);
    if (kind == 0xb || kind == 0xc || kind == 0x10 || kind == 0x11 || kind == 0x19) h = rounded(x, y);
    const u32 type = at<u32>(cell, 8);
    if (type == 7 || type == 0x1e || type == 4 || type == 9 || type == 0xa) h = rounded(x, y);
    return h;
}

// The squares down each side whose next cell stands lower: built (the answer 1) or not, with the tag and the mode flag of the other way.
void walls(const u8 *cell, float top, u32 x10, u32 y10, bool built, u32 tag, u32 flag) {
    static const u32 kSides[4][2] = {{0x007cdba0, 1}, {0x007cdbc8, 4}, {0x007cdba8, 0x10}, {0x007cdbd0, 0x40}};
    for (const auto &side : kSides) {
        const s32 *const step = reinterpret_cast<const s32 *>((uintptr_t)side[0]);
        const u8 *const next = Map_StepCell(reinterpret_cast<const u16 *>(cell), nullptr, step[0], step[1], 1);
        if (!next) continue;
        const u32 id = (u32)*reinterpret_cast<const u16 *>(next) - 1;
        const float h = height_of(next, (s32)(id & 0x7f), (s32)(id >> 7));
        if (top == h || top != top || h != h) continue;   // (fcomp: equal or unordered)
        const s32 high = ftol(top);
        for (s32 i = ftol(h); i < high; i = (s32)((u32)i + 10)) {
            const float z = (float)(X87Reg((double)i) - global<float>(0x00700d5c));
            if (built) {
                DynFaces_AddSquare(x10, bits(z), y10, (u8)side[1], 1, 0, 0x3f800000, 0, 0, 0, 0);
            } else {
                global<u32>(0x00816d58) = 0;
                global<u32>(0x00816d50) = 0;
                DynFaces_AddSquare(x10, bits(z), y10, (u8)side[1], tag, 0, 0x3f800000, 0, 0, flag, 0);
            }
        }
    }
}

u32 draw_walls(u8 *cell, u32 code, u32 a, u32 b) {
    const u32 id = (u32)*reinterpret_cast<const u16 *>(cell) - 1;
    const s32 x = (s32)(id & 0x7f), y = (s32)(id >> 7);
    const u32 flag = Global81ae2c_Get_52f840() != 4;
    MapCell_IsTypeCOr11(MapHandle_Entry28(reinterpret_cast<const u16 *>(cell)));   // (the answer dropped)
    u32 kept = 0, after = 1;
    if (code == 4 || code == 0x18 || (code == 0xb && a == 5) || (code == 0x10 && a == 5)) {
        kept = a;
        after = b;
    }
    const float top = height_of(cell, x, y);
    const u32 y10 = bits((float)(s32)((u32)y * 10)), x10 = bits((float)(s32)((u32)x * 10));
    u32 answer = global<u32>(0x0081b604) ? 1 : MapCell_CanBuild(cell, nullptr, code, 0);
    const u32 z = bits((float)(X87Reg(top) - global<float>(0x00700d5c)));
    if (answer == 1) {
        MapCell_CanBuild(cell, nullptr, code, 1);
        Global816d48_Set(1);
        DynFaces_AddSquare(x10, z, y10, 0, 1, 1, 0x3f800000, 0, 0, 0, 0);
        walls(cell, top, x10, y10, true, 0, 0);
        return 1;
    }
    if (!answer) answer = kept;
    DynFaces_AddSquare(x10, z, y10, 0, answer, after, 0x3f800000, 0, 0, 0, 0);
    walls(cell, top, x10, y10, false, answer == 2 ? 2 : 0, flag);
    return 1;
}

// The cell's entry, or for a type 0xc / 0x11 one its parent ([0x7cf83c] + (its word +0x10 + 0x6cde) * 0x28).
u8 *entry_of(const u8 *cell) {
    u8 *entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
    if (MapCell_IsTypeCOr11(entry)) entry = global<u8 *>(0x007cf83c) + ((u32)at<u16>(entry, 0x10) + 0x6cde) * 0x28;
    return entry;
}

bool type1or3(const u8 *cell) { return at<u32>(cell, 8) == 1 || at<u32>(cell, 8) == 3; }

void reset_ends(u8 *cell, u32 a, u32 b) {
    MapCell_Clear(cell, nullptr, a, b);
    u8 *const entry = entry_of(cell);
    if (!MapCell_IsTypeBDOr10Or12Or19(entry) || !MapCell_HasLowFlags(entry)) return;
    const u8 *const cell44 = MapHandle_Cell44(reinterpret_cast<const u16 *>(entry));
    s32 count = type1or3(cell44) ? 1 : 0;
    static const u32 kPairs[3] = {0x007cdbc8, 0x007cdba8, 0x007cdbb8};
    for (const u32 pair : kPairs) {
        const s32 *const step = reinterpret_cast<const s32 *>((uintptr_t)pair);
        const u8 *const next = Map_StepCell(reinterpret_cast<const u16 *>(cell44), nullptr, step[0], step[1], 1);
        if (next && type1or3(next)) ++count;
    }
    if (count) return;
    const u8 direction = MapCell_Direction(entry);
    u32 first, second;
    if (direction == 1 || direction == 0x10) {
        first = 0x007cdba8;
        second = 0x007cdba0;
    } else if (direction == 4 || direction == 0x40) {
        first = 0x007cdbd0;
        second = 0x007cdbc8;
    } else {
        return;
    }
    if (!MapCell_HasLowFlags(entry)) return;
    const s32 *const p = reinterpret_cast<const s32 *>((uintptr_t)first);
    const s32 *const q = reinterpret_cast<const s32 *>((uintptr_t)second);
    TrackCell_ResetEnds(entry, p[0], p[1], q[0], q[1]);
}

}  // namespace

void CellMessage_SetEnv(const CellMessageEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall MapCell_HandleMessage(u8 *cell, void *, u32 message, u32 a, u32 b) {
    const u32 code = message & 0xff;
    if (message & 0x200) {
        Global81ae2c_Get_52f840();
        return MapCell_CanBuild(cell, nullptr, code, 0) != 1;
    }
    if (message & 0x100) return draw_walls(cell, code, a, b);
    switch (code) {
    case 0x32:
        reset_ends(cell, a, b);
        return 1;
    case 0x80:
        MapCell_LinkPath(cell, nullptr, global<u32>(0x00816d64), (s32)a, (s32)b, 1);
        return 1;
    case 0x81:
        MapCell_ApplyPathModel(cell);
        global<u32>(0x00816d64) = 0;
        if (!global<u32>(0x00820ab8) || !global<u32>(0x00820abc)) return 1;
        global<u32>(0x00820abc) = 0;
        return 0;
    case 0x82:
        if (MapCell_Direction(cell)) return 1;
        if (a == 0xffffffffu) MapCell_SetDirection(cell, nullptr, 4);
        if (a == 1) MapCell_SetDirection(cell, nullptr, 0x40);
        if (b == 0xffffffffu) MapCell_SetDirection(cell, nullptr, 0x10);
        if (b == 1) MapCell_SetDirection(cell, nullptr, 1);
        return 1;
    case 0x83:
        at<u16>(cell, 0x10) = global<u16>(0x00818698);
        return 1;
    case 0x84:
        at<u16>(cell, 0x40) = global<u16>(0x008186bc);
        ++global<u32>(0x008186bc);
        return 1;
    case 0x85: {
        u8 *const entry = entry_of(cell);
        if (!MapCell_IsTypeBDOr10Or12Or19(entry) || at<u32>(entry, 8) == 0x19 || MapCell_CornerNeighbours(entry) ||
            MapCell_JunctionNeighbours(entry) || MapCell_HasLowFlags(entry))
            return 1;
        MapCell_ClearFlags(entry, nullptr, 0xf);
        MapCell_SetFlags(entry, nullptr, 1);
        return 1;
    }
    case 0x86: {
        const u8 *const entry = entry_of(cell);
        if (!MapCell_IsTypeBDOr10Or12Or19(entry) || MapCell_IsType19(entry)) return 1;
        const u32 parent = at<u16>(entry, 0x10);
        if (!parent) return 1;
        u16 handle = (u16)b;
        Cell_FirstObject(global<u8 *>(0x007cf83c) + 0x294 + parent * 0x44, nullptr, &handle);
        if (!handle) return 1;
        u8 *const thing = global<u8 *>(0x007cfb90 + (u32)handle * 20);
        const u32 slot = (a & 0xffff0000u) | (handle & 0xff00u) | (at<u32>(thing, 0x2c) != 0);   // (a's slot, sic)
        if (!TrackRun_UpdateA(thing, 1, 1, slot)) TrackRun_UpdateB(thing, 0, 1, slot);
        return 1;
    }
    case 0x87: {
        if (global<s32>(0x00818c10) <= global<s32>(0x00818c14)) return 1;
        const u32 id = (u32)*reinterpret_cast<const u16 *>(cell) - 1;
        const u32 saved8 = global<u32>(0x0081ede8), saved4 = global<u32>(0x0081ede4);
        const u32 mode = Global81ae2c_Get_52f840();
        if (at<u32>(cell, 8) == 1 && mode != 3 && mode != 1)
            MapCell_Clear(cell, nullptr, 0, 0);
        else
            g_env->deadEnds((s32)(id & 0x7f), (s32)(id >> 7), 0);
        global<u32>(0x0081ede4) = saved4;
        global<u32>(0x0081ede8) = saved8;
        g_env->setMode(mode, 0);
        return 1;
    }
    default:
        break;
    }
    global<u32>(0x00820abc) = 0;
    if (global<u32>(0x00820ab8) && message != 0x15 && (at<u32>(cell, 8) == 3 || at<u32>(cell, 8) == 1)) global<u32>(0x00820abc) = 1;
    if (message == 0x15 && ((at<u8>(cell, 0xf) & 1) || at<u32>(cell, 8) == 4 || at<u32>(cell, 8) == 9 || at<u32>(cell, 8) == 0xa))
        global<u32>(0x00820abc) = 1;
    if (!MapCell_SetKind(cell, nullptr, message)) return 0;
    global<u32>(0x00816d64) = message;
    return 1;
}
}

namespace {

u8 *map_cell(s32 x, s32 y) {
    return global<u8 *>(0x007cf83c) + 0x294 + (((u32)y << 7) + (u32)x + 1 & 0xffff) * 0x44;
}

}   // namespace

extern "C" {

void __cdecl Map_SetRideCell(u32 unused, s32 x, s32 y, u32 ride, u32 flagged) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= global<s32>(0x007a0314)) x = global<s32>(0x007a0314) - 1;
    if (y >= global<s32>(0x007a0374)) y = global<s32>(0x007a0374) - 1;
    if (!Map_InBounds(x, y)) return;
    u8 *const cell = map_cell(x, y);
    if (!cell) return;
    u8 *const entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
    const u32 slot = (unused & 0xffff0000u) | at<u16>(cell, 0x10);   // (the first argument's slot, its low word the owner's)
    MapCell_Clear(cell, nullptr, 0, 0);
    MapCell_HandleMessage(cell, nullptr, 0x15, 0, 0);
    MapCell_SetParent(cell, nullptr, slot);
    at<u32>(entry, 4) = ride;
    const u32 flags = MapCell_Flags(cell);
    MapCell_SetFlags(cell, nullptr, flagged ? flags | 0x100 : flags & 0xfeff);
}

u32 __cdecl Map_SendLine(u32 message, s32 x0, s32 y0, const s32 *x1, const s32 *y1) {
    global<u32>(0x00820aa4) = 1;
    global<u32>(0x00820ab8) = 0;
    const s32 ex = *x1, ey = *y1;
    const s32 dx = (s32)((u32)ex - (u32)x0), dy = (s32)((u32)ey - (u32)y0);
    const bool along_x = (dx < 0 ? (s32)(0u - (u32)dx) : dx) > (dy < 0 ? (s32)(0u - (u32)dy) : dy);
    s32 x = x0, y = y0;
    s32 &walk = along_x ? x : y;
    const s32 step = along_x ? (x0 <= ex ? 1 : -1) : (y0 <= ey ? 1 : -1);
    const s32 *const end = along_x ? x1 : y1;
    if (walk != (along_x ? ex : ey)) {
        do {
            if (Map_InBounds(x, y)) {
                const u32 answer = along_x ? MapCell_HandleMessage(map_cell(x, y), nullptr, message, (u32)step, 0)
                                           : MapCell_HandleMessage(map_cell(x, y), nullptr, message, 0, (u32)step);
                global<u32>(0x00820aa4) = 0;
                if (!answer) return 0;
            }
            walk = (s32)((u32)walk + (u32)step);
        } while (walk != *end);
    }
    global<u32>(0x00820ab8) = 1;
    if (!Map_InBounds(x, y)) return 1;
    const u32 answer = along_x ? MapCell_HandleMessage(map_cell(x, y), nullptr, message, (u32)step, 0)
                               : MapCell_HandleMessage(map_cell(x, y), nullptr, message, 0, (u32)step);
    global<u32>(0x00820aa4) = 0;
    global<u32>(0x00820ab8) = 0;
    return answer ? 1 : 0;
}
}
