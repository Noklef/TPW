// A track cell's messages: see trackmessage.h.
#include "trackmessage.h"

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
#include "mapcell.h"
#include "mapline.h"
#include "thingscan.h"
#include "cellbuild.h"
#include "mapclick.h"
#include "focuscell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TrackMessageEnv kGameEnv = {&MapCell_CanBuildTrack,  &DynFaces_AddSquare, &TrackCell_Remove, &MapLine_LinkCell, &MapCell_SetStepDirection,
                                  &MapCell_ApplyPathModel, &MapCell_ApplyModel, &MapCell_SetTrackKind, &TrackRun_UpdateA, &TrackRun_UpdateB,
                                  &Path_CheckDeadEnds,     &MapCell_Clear,      &GameMode_SetOnly};
const TrackMessageEnv *g_env = &kGameEnv;

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

s32 ftol(double x) { return (s32)(long long)x; }   // (__ftol)

const u16 *handle(const u8 *cell) { return reinterpret_cast<const u16 *>(cell); }

// The (dx, dy) pair at an address of the step table (0x7cdba0 on).
const s32 *pair(u32 address) { return reinterpret_cast<const s32 *>((uintptr_t)address); }

// A track cell's height at (x, y): see trackmessage.h.
float height_of(const u8 *cell, s32 x, s32 y) {
    float h = 0.0f;
    const u32 type = at<u32>(cell, 8);
    if (type == 0xb || type == 0x10 || type == 0x19 || type == 7 || type == 4 || type == 9 || type == 0xa) {
        const s32 tens = (s32)((u32)ftol(Terrain_SurfaceHeight(x, y, nullptr)) + 9) / 10;
        h = (float)(X87Reg((double)tens) * global<float>(0x00700d94));
    }
    return Terrain_RoundedCellHeight(MapHandle_Cell44(handle(cell)), h, x, y);
}

// The squares down each side whose cell two steps away stands lower (ordered and not equal), with the tag and the flag (argument i).
void sides(const u8 *cell, float top, u32 x10, u32 y10, u32 tag, u32 flag) {
    static const u32 kSides[4][2] = {{0x007cdba0, 1}, {0x007cdbc8, 4}, {0x007cdba8, 0x10}, {0x007cdbd0, 0x40}};
    for (const auto &side : kSides) {
        const s32 *const step = pair(side[0]);
        const u8 *const next = Map_StepCell28(handle(cell), nullptr, step[0], step[1], 2);
        if (!next) continue;
        const u32 id = (u32)*handle(next) - 1;
        const float h = height_of(next, (s32)(id & 0x7f), (s32)(id >> 7));
        if (top == h || top != top || h != h) continue;   // (fcomp: equal or unordered)
        const s32 high = ftol(top);
        for (s32 i = ftol(h); i < high; i = (s32)((u32)i + 10)) {
            const float z = (float)(X87Reg((double)i) - global<float>(0x00700d9c));
            g_env->addSquare(x10, bits(z), y10, (u8)side[1], tag, 0, 0, 1, 0, flag, 0);
        }
    }
}

u32 walls(u8 *cell, u32 code, u32 a, u32 b) {
    const u32 id = (u32)*handle(cell) - 1;
    const s32 x = (s32)(id & 0x7f), y = (s32)(id >> 7);
    u32 kept = 0, after = 1;
    if (code == 4 || (code == 0xb && a == 5) || (code == 0x10 && a == 5)) {
        kept = a;
        after = b;
    }
    const float top = height_of(cell, x, y);
    const u32 x10 = bits((float)(s32)((u32)x * 10)), y10 = bits((float)(s32)((u32)y * 10));
    u8 direction = 0;
    if (a == 0xfffffffeu) direction = 0x40;
    if (a == 2) direction = 4;
    if (b == 0xfffffffeu) direction = 1;
    if (b == 2) direction = 0x10;
    const u32 answer = g_env->canBuild(cell, nullptr, code, 0, direction);
    const u32 z = bits((float)(X87Reg(top) - global<float>(0x00700d9c)));
    if (answer == 1) {
        g_env->canBuild(cell, nullptr, code, 1, direction);
        Global816d48_Set(1);
        g_env->addSquare(x10, z, y10, 0, 1, 1, 0, 1, 0, 0, 0);
        sides(cell, top, x10, y10, 1, 0);
        return 1;
    }
    g_env->addSquare(x10, z, y10, 0, answer ? answer : kept, after, 0, 1, 0, 0, 0);
    sides(cell, top, x10, y10, 0, answer == 0xd);
    return 1;
}

// Two side neighbours (MapCell_SideNeighbourCount 2) facing each other / at a right angle; more than two (signed) with any bit.
bool straight(const u8 *cell) {
    const u8 n = MapCell_Neighbours(cell);
    return MapCell_SideNeighbourCount(cell) == 2 && ((n & 1 && n & 0x10) || (n & 4 && n & 0x40));
}

bool corner(const u8 *cell) {
    const u8 n = MapCell_Neighbours(cell);
    return MapCell_SideNeighbourCount(cell) == 2 && ((n & 1 && n & 4) || (n & 1 && n & 0x40) || (n & 0x10 && n & 4) || (n & 0x10 && n & 0x40));
}

bool junction(const u8 *cell) {
    const u8 n = MapCell_Neighbours(cell);
    return (s32)MapCell_SideNeighbourCount(cell) > 2 && n;
}

// With 0x81b4cc's byte set: a cell of type 0xb, 0xc, 0x10 or 0x11, or one whose 0x44-byte cell is of type 0x15.
bool held(const u8 *cell) {
    if (!global<u8>(0x0081b4cc)) return false;
    const u32 type = at<u32>(cell, 8);
    if (type == 0xb || type == 0xc || type == 0x10 || type == 0x11) return true;
    return at<u32>(MapHandle_Cell44(handle(cell)), 8) == 0x15;
}

void flag40(u8 *target, const u8 *from, bool clear) {
    if (clear)
        MapCell_ClearFlags(target, nullptr, 0x40);
    else
        MapCell_SetFlags(target, nullptr, (u16)(MapCell_Flags(from) | 0x40));
}

// A hoarding's 0x44-byte cell and the three it steps to (pairs 0x7cdbc8, 0x7cdba8, 0x7cdbb8): flag 0x40 cleared (a to d) or set, by
// the hoarding's direction: at an end (straight between two side neighbours, or the cell at either point 0x7639f0 / 0x7639f8) 1: b, d;
// 4: c, d; 0x10: a, c; 0x40: a, b. At a corner: all four when the direction and the neighbour bits match (1 with 0x41 or 0x50, 4 with
// 5 or 0x41, 0x10 with 0x14 or 5, 0x40 with 0x50 or 0x14), else 1: b; 4: d; 0x10: c; 0x40: a. Each then MapCell_ApplyPathModel.
void hoarding_flags(const u8 *cell) {
    u8 *const c44 = MapHandle_Cell44(handle(cell));
    const u8 direction = MapCell_Direction(cell);
    const u8 n = MapCell_Neighbours(cell);
    const bool match = (direction == 1 && n == 0x41) || (direction == 0x10 && n == 0x14) || (direction == 4 && n == 5) ||
                       (direction == 0x40 && (n == 0x50 || n == 0x14)) || (direction == 1 && n == 0x50) || (direction == 4 && n == 0x41) ||
                       (direction == 0x10 && n == 5);
    const u32 id = (u32)*handle(c44) - 1;
    const u32 x = id & 0x7f, y = id >> 7;
    bool a = false, b = false, c = false, d = false;
    if (direction == 1 || direction == 4 || direction == 0x10 || direction == 0x40) {
        if (straight(cell) || (x == global<u32>(0x007639f0) && y == global<u32>(0x007639f4)) ||
            (x == global<u32>(0x007639f8) && y == global<u32>(0x007639fc))) {
            if (direction == 1) b = d = true;
            else if (direction == 4) c = d = true;
            else if (direction == 0x10) a = c = true;
            else a = b = true;
        } else if (corner(cell)) {
            if (match) a = b = c = d = true;
            else if (direction == 1) b = true;
            else if (direction == 4) d = true;
            else if (direction == 0x10) c = true;
            else a = true;
        }
    }
    flag40(c44, c44, a);
    g_env->pathModel(c44);
    static const u32 kPairs[3] = {0x007cdbc8, 0x007cdba8, 0x007cdbb8};
    const bool clear[3] = {b, c, d};
    for (int k = 0; k < 3; ++k) {
        const s32 *const step = pair(kPairs[k]);
        u8 *const next = Map_StepCell(handle(c44), nullptr, step[0], step[1], 1);
        if (!next) continue;
        flag40(next, c44, clear[k]);   // (set from the hoarding's own cell's flags, sic)
        g_env->pathModel(next);
    }
}

u32 model(u8 *cell) {
    if (held(cell)) return 1;
    if (at<u32>(cell, 8) == 0x19)
        hoarding_flags(cell);
    else if (corner(cell) || junction(cell))
        at<u16>(cell, 0xe) &= 0xfff0;
    global<u32>(0x00816d64) = 0;
    if (at<u32>(cell, 8) != 0xd && at<u32>(cell, 8) != 0x12) g_env->model(cell);
    if (global<u32>(0x00820ad0) && global<u32>(0x00820ae8)) {
        global<u32>(0x00820ae8) = 0;
        return 0;
    }
    return 1;
}

u32 owner(u8 *cell) {
    const u16 id = *handle(cell);
    if (held(cell) || !gFocuscell_8186c4) return 1;
    at<u16>(cell, 0x10) = global<u16>(0x00818698);
    static const u32 kPairs[3] = {0x007cdba8, 0x007cdbc8, 0x007cdbb8};
    for (const u32 p : kPairs) {
        u8 *const next = Map_StepCell28(handle(cell), nullptr, pair(p)[0], pair(p)[1], 1);
        if (!next) continue;
        at<u16>(next, 0x10) = id;
        g_env->setKind(next, nullptr, 0xc);
    }
    return 1;
}

bool type2or7(const u8 *cell) { return MapCell_IsType2(cell) || MapCell_IsType7Or1e(cell); }

u32 end_flag(u8 *cell) {
    const u8 *const c44 = MapHandle_Cell44(handle(cell));
    if (corner(cell) || junction(cell)) return 1;
    static const u32 kPairs[3] = {0x007cdba8, 0x007cdbc8, 0x007cdbb8};
    if (at<u32>(cell, 8) == 0x19) {
        if (type2or7(c44)) at<u16>(cell, 0xe) = 1;
        for (const u32 p : kPairs) {
            const u8 *const next = Map_StepCell(handle(c44), nullptr, pair(p)[0], pair(p)[1], 1);
            if (next && type2or7(next)) at<u16>(cell, 0xe) = 1;
        }
        return 1;
    }
    u32 type = at<u32>(c44, 8);
    if (type == 1 || type == 3) at<u16>(cell, 0xe) = 1;
    u8 empty = type == 0;
    for (const u32 p : kPairs) {
        const u8 *const next = Map_StepCell(handle(c44), nullptr, pair(p)[0], pair(p)[1], 1);
        if (!next) continue;
        type = at<u32>(next, 8);
        if (type == 1 || type == 3) at<u16>(cell, 0xe) = 1;
        if (type == 0) ++empty;
    }
    if (empty == 4) at<u16>(cell, 0xe) &= 0xfff0;
    return 1;
}

void run(const u8 *cell, u32 b) {
    const u32 type = at<u32>(cell, 8);
    if ((type != 0xb && type != 0xd && type != 0x10 && type != 0x12) || global<u8>(0x0081b4cc)) return;
    const u32 parent = at<u16>(cell, 0x10);
    if (!parent) return;
    u16 first = (u16)b;   // (b's slot is the out word)
    Cell_FirstObject(global<u8 *>(0x007cf83c) + 0x294 + parent * 0x44, nullptr, &first);
    if (!first) return;
    // (the debug check of the thing table, Debug_Printf, left out)
    u8 *const thing = global<u8 *>(0x007cfb90 + (u32)first * 20);
    if (!g_env->updateA(thing, 1, 0, 0)) g_env->updateB(thing, 0, 0, 0);
}

void dead_ends(const u8 *cell) {
    if (reinterpret_cast<s32 &>(gMapclick_818c10) <= gCellbuild_818c14) return;
    const u32 id = (u32)*handle(cell) - 1;
    const u32 x = id & 0x7f, y = id >> 7;
    const u32 saved4 = global<u32>(0x0081ede4), saved8 = global<u32>(0x0081ede8);
    const u32 mode = Global81ae2c_Get_52f840();
    const u32 type = at<u32>(cell, 8);
    if (type != 0xb && type != 0xd && type != 0x10 && type != 0x12 && type != 0x19) {
        g_env->deadEnds((s32)x, (s32)y, 0);
        g_env->deadEnds((s32)(x + 1), (s32)y, 0);
        g_env->deadEnds((s32)x, (s32)(y + 1), 0);
        g_env->deadEnds((s32)(x + 1), (s32)(y + 1), 0);
        if (global<u32>(0x00820ad0)) {
            u8 *const c44 = MapHandle_Cell44(handle(cell));
            g_env->clear(c44, nullptr, 0, 0);
            static const u32 kPairs[3] = {0x007cdbc8, 0x007cdba8, 0x007cdbb8};
            for (const u32 p : kPairs) g_env->clear(Map_StepCell(handle(c44), nullptr, pair(p)[0], pair(p)[1], 1), nullptr, 0, 0);   // (unchecked, sic)
        }
    }
    global<u32>(0x0081ede4) = saved4;
    global<u32>(0x0081ede8) = saved8;
    g_env->setOnly(mode, 0);
}

}  // namespace

void TrackMessage_SetEnv(const TrackMessageEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall MapCell_HandleTrackMessage(u8 *cell, void *, u32 message, u32 a, u32 b) {
    const u32 code = message & 0xff;
    if (message & 0x100) return walls(cell, code, a, b);
    switch (code) {
    case 0x32:
        g_env->remove(cell, nullptr, (s32)a, (s32)b, 1);
        return 1;
    case 0x80:
        g_env->link(cell, nullptr, global<u32>(0x00816d64), (s32)a, (s32)b);
        return 1;
    case 0x81:
        return model(cell);
    case 0x82:
        g_env->stepDirection(cell, nullptr, (s32)a, (s32)b);
        return 1;
    case 0x83:
        return owner(cell);
    case 0x85:
        return end_flag(cell);
    case 0x86:
        run(cell, b);
        return 1;
    case 0x87:
        dead_ends(cell);
        return 1;
    default:
        break;
    }
    const u32 type = at<u32>(cell, 8);
    global<u32>(0x00820ae8) = 0;
    if ((global<u32>(0x00820ad0) && (type == 0xd || type == 0x12 || type == 0xb || type == 0x10)) || type == 0x19) global<u32>(0x00820ae8) = 1;
    if (!g_env->setKind(cell, nullptr, message)) return message == 0x19;
    global<u32>(0x00816d64) = message;
    return 1;
}
}
