#include "placeobject.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/soundplay.h"
#include "../core/thunks.h"
#include "../math/intpair.h"
#include "../math/x87.h"
#include "cellkind.h"
#include "focuscell.h"
#include "mapcell.h"
#include "objectdefs.h"
#include "pathlink.h"
#include "things.h"
#include "trackrides.h"
#include "tracksquare.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const PlaceObjectEnv kGameEnv = {reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>((uintptr_t)0x004db090)};
const PlaceObjectEnv *g_env = &kGameEnv;

// fild n; fmul [0x700cd0]; fmul [0x700cd8]; __ftol.
s32 scaled(s32 n) {
    const double v = (X87Reg(X87_MulIntDouble(n, global<float>(0x00700cd0))) * global<float>(0x00700cd8)).v;
    return (s32)(long long)v;
}

}  // namespace

extern "C" {

void PlaceObject_SetEnv(const PlaceObjectEnv *env) { g_env = env ? env : &kGameEnv; }

u8 *__cdecl PlaceObject_Create(s32 angle) {
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, global<u16>(0x00818c38));
    u32 ride = 0;
    const s32 kind = at<s32>(def, 0xa0);
    if (kind) {
        const u32 cell = (u32)global<u16>(0x00818c3c) - 1;
        s32 x = (s32)(cell & 0x7f), y = (s32)(cell >> 7);
        u32 code;
        switch (angle) {
        case 0: x += at<s32>(def, 0xa4); y += at<s32>(def, 0xa8); code = 0; break;
        case 0x5a: x += at<s32>(def, 0xac); y += at<s32>(def, 0xb0); code = 5; break;
        case 0xb4: x += at<s32>(def, 0xb4); y += at<s32>(def, 0xb8); code = 6; break;
        case 0x10e: x += at<s32>(def, 0xbc); y += at<s32>(def, 0xc0); code = 1; break;
        default: code = (u32)angle; break;
        }
        switch (angle) {   // (Position_StepA's steps, inlined)
        case 0: x += 4; y += 1; break;
        case 0x5a: x += 1; y -= 5; break;
        case 0xb4: x -= 5; y -= 2; break;
        case 0x10e: x -= 2; y += 4; break;
        default: break;
        }
        const s32 ty = scaled(y);
        const s32 tx = scaled(x);
        ride = TrakRide_Create(kind, tx, ty, code, 0, global<u16>(0x00818c38));
    }
    u32 pair[2];
    IntPair_Rotated(reinterpret_cast<const u32 *>(def + 0x4b0), nullptr, pair, angle);
    const u16 cell = (u16)((u16)(pair[1] << 7) + global<u16>(0x00818c3c) + (u16)pair[0]);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x450));
    u8 *const object = block ? g_env->construct(block, nullptr, cell, global<u16>(0x00818c38), angle, ride) : nullptr;
    if (!object) return nullptr;
    u16 handle = 0;
    if (global<u32>(0x008187f8) && at<u32>(def, 0x288) != 2) Thing_GetHandle(reinterpret_cast<const u8 *>((uintptr_t)Global8186ac_Get()), nullptr, &handle);
    Fn_4e0de0(object, nullptr, handle);
    Wrap_Global7c2654_Set_4815b0((u32)(uintptr_t)object);
    return object;
}
}

namespace {

template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
const PlaceFootprintEnv kFootprintEnv = {game<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>(0x00532fc0),
                                         &MapCell_Clear,
                                         game<void(__fastcall *)(u8 *)>(0x00536780),
                                         game<void(__fastcall *)(u8 *)>(0x004de1f0),
                                         &MapCell_LinkPath,
                                         &PlaceObject_Create,
                                         &FocusCell_Update,
                                         &Sound_PlayAt};
const PlaceFootprintEnv *g_fp = &kFootprintEnv;

u32 cellId(s32 x, s32 y) { return (((u32)y << 7) + (u32)x + 1) & 0xffff; }
u8 *cell44(s32 x, s32 y) { return global<u8 *>(0x007cf83c) + 0x294 + cellId(x, y) * 0x44; }
u8 *entry28(s32 x, s32 y) { return global<u8 *>(0x007cf83c) + 0x1102b0 + cellId(x, y) * 0x28; }   // ((id + 0x6cde) * 0x28)
u32 test(u8 *cell, u32 action, u32 b = 0, u32 c = 0) { return g_fp->test532fc0(cell, nullptr, action, b, c); }
u8 shift(u32 a, u32 b) { return (u8)Byte_ShiftLog2(a, b); }

// The footprint's frame: the corner (x, y) and the offsets the turned cells are taken from.
struct Frame {
    s32 x, y, angle;
    s32 o3c, o40, o44, o48, o64, o68;
};

// A footprint cell (column, row) on the map, turned by the angle.
void spot(const Frame &f, s32 col, s32 row, s32 *px, s32 *py) {
    switch (f.angle) {
    case 0x5a: *px = f.o44 + row + f.x; *py = f.o48 - col + f.y + 5; break;
    case 0xb4: *px = f.o40 - col + f.x + 5; *py = f.o68 - row + f.y + 5; break;
    case 0x10e: *px = f.o64 - row + f.x + 5; *py = f.o3c + col + f.y; break;
    default: *px = col + f.x; *py = row + f.y; break;
    }
}

// A path cell linked (MapCell_LinkPath (1, 0, 0, 1)) and marked (0x81).
void link(u8 *cell) {
    g_fp->linkPath(cell, nullptr, 1, 0, 0, 1);
    test(cell, 0x81);
}

// The path cells about a cell (its neighbour bits, from 1 round by Byte_ShiftLog2(d, 4)), those of type 1 linked.
void linkAround(u8 *cell) {
    u8 d = 1;
    do {
        if (MapCell_Neighbours(cell) & d) {
            u8 *const next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, d);
            if (next && at<s32>(next, 8) == 1) link(next);
        }
        d = shift(d, 4);
    } while (d != 1);
}

// A path cell joined as a path: 0x87 unless of type 1, 1, linked, 0x85, 0x86, flag 0x20 and the path cells about it.
void joinPath(u8 *cell) {
    if (at<s32>(cell, 8) != 1) test(cell, 0x87);
    test(cell, 1);
    link(cell);
    test(cell, 0x85);
    test(cell, 0x86);
    MapCell_SetFlags(cell, nullptr, 0x20);
    linkAround(cell);
}

}  // namespace

extern "C" {

void PlaceFootprint_SetEnv(const PlaceFootprintEnv *env) { g_fp = env ? env : &kFootprintEnv; }

s32 *__cdecl PlaceObject_Footprint(s32 x, s32 y, u32, s32 angle, u32 place, u32 check) {
    u16 objectCell = 0;
    s32 count = 0;
    u8 failed = 0, seen9 = 0, seenA = 0, dir9 = 0, dirA = 0;
    s32 x9 = 0, y9 = 0, xA = 0, yA = 0;   // (the cell behind the type 9 cell, the one ahead of the type 0xa cell)
    u8 dir = 0;                           // (unset in the original until a pass sets it)
    global<u8>(0x00818c50) = 0;
    global<u8>(0x00818c30) = 0;
    const u16 id = global<u16>(0x008186e0);
    global<u16>(0x00818c38) = id;
    global<s32>(0x007639e0) = -1;
    global<s32>(0x007639e4) = -1;
    global<s32>(0x00818c28) = -1;
    global<s32>(0x00818c2c) = -1;
    global<s32>(0x00818c20) = -1;
    global<s32>(0x00818c24) = -1;
    u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, id);
    global<u8 *>(0x00818c40) = def;
    const s32 *const shape = at<s32 *>(def, 0x18);
    global<const s32 *>(0x00818c18) = shape;
    const s32 cols = shape[0], rows = shape[1];
    const s32 h = (5 - cols) / 2, v = (5 - rows) / 2;
    Frame f;
    f.angle = angle;
    if (!global<u8>(0x0081af7c)) {
        x -= cols / 2;
        y -= rows / 2;
        f.o40 = -1 - 2 * h;
        f.o44 = v - h;
        f.o68 = -1 - 2 * v;
        f.o48 = f.o64 = -1 - h - v;
        f.o3c = h - v;
    } else {
        f.o3c = f.o44 = 0;
        f.o40 = f.o48 = f.o68 = -5;
        f.o64 = -1 - rows;
    }
    f.x = x;
    f.y = y;
    global<s32>(0x00818c44) = x - h;
    global<s32>(0x00818c48) = y - v;
    global<s32>(0x00818c4c) = angle;
    const auto type = [&](s32 col, s32 row) { return shape[2 + 2 * (row * 0x14 + col)]; };
    const auto dirByte = [&](s32 col, s32 row) { return reinterpret_cast<const u8 *>(shape)[0xc + 8 * (row * 0x14 + col)]; };
    if (place) {
        for (s32 row = 0; row < rows; ++row) {
            for (s32 col = 0; col < cols; ++col) {
                s32 px, py;
                spot(f, col, row, &px, &py);
                if (!Map_InBounds(px, py)) continue;
                const s32 t = type(col, row);
                if (t == 4) ++count;
                if (t) test(cell44(px, py), 0x87);
            }
        }
        if (rows > 0) dir = (u8)(f.o68 + y + 5 - rows);   // (sic: the pass's row coordinate, kept where the direction goes)
        s32 ax, ay;
        spot(f, 0, 0, &ax, &ay);
        global<s32>(0x00818c28) = ax;
        global<s32>(0x00818c2c) = ay;
        global<u16>(0x00818c3c) = (u16)cellId(ax, ay);
        const u8 *const object = g_fp->create(angle);
        if (!global<u32>(0x00785a2c)) {
            const float scale = global<float>(0x00700cd8);
            const s32 z = (s32)(long long)X87_MulIntDouble(y, scale);   // (__ftol)
            const s32 across = (s32)(long long)X87_MulIntDouble(x, scale);
            const u32 sound = count < 4 ? 0x8e : count < 8 ? 0x8d : count < 0x10 ? 0x8f : 0x90;
            g_fp->playAt(0, global<u32>(0x00803a2c), sound, across, 0, z);
        }
        objectCell = (u16)(object[7] * 0x80 + object[5] + 1);
    }
    if (!global<u32>(0x008187f8)) g_fp->focusUpdate(0);
    for (s32 row = 0; row < rows; ++row) {
        for (s32 col = 0; col < cols; ++col) {
            s32 px, py;
            spot(f, col, row, &px, &py);
            dir = angle == 0x5a ? 0x40 : angle == 0xb4 ? 0x10 : angle == 0x10e ? 4 : 1;
            if (!Map_InBounds(px, py)) continue;
            u8 *const cell = cell44(px, py);
            u8 *const entry = entry28(px, py);
            const s32 t = type(col, row);
            const u8 b = dirByte(col, row);
            if (place && t) {
                const u32 as = t == 0xb || t == 0x10 || t == 0x17 ? 4 : (u32)t;
                test(cell, 0x87);
                g_fp->clear(cell, nullptr, 0, 0);
                test(cell, as);
                test(cell, 0x81);
                MapCell_SetParent(cell, nullptr, objectCell);
                g_fp->entry536780(entry);
                MapCell_ClearFlags(entry, nullptr, 0xf);
            }
            // (a soft test: 0x104 (6, the direction) on the first row, else 0x104; a refusal sets the failed flag)
            const auto soft = [&]() {
                if (!(row ? test(cell, 0x104) : test(cell, 0x104, 6, dir))) failed = 1;
            };
            switch ((u32)t) {
            case 0: break;
            case 1:
                if (place) {
                    MapCell_SetFlags(cell, nullptr, 0x20);
                    link(cell);
                    linkAround(cell);
                } else if (check) {
                    if (!test(cell, 0x204)) return nullptr;
                } else if (!test(cell, 0x104)) {
                    failed = 1;
                }
                break;
            case 9: {
                seen9 = 1;
                dir9 = shift(dir, b);
                switch (dir9) {
                case 1: x9 = px; y9 = py + 1; break;
                case 4: x9 = px - 1; y9 = py; break;
                case 0x10: x9 = px; y9 = py - 1; break;
                case 0x40: x9 = px + 1; y9 = py; break;
                default: break;
                }
                if (place) {
                    global<s32>(0x00818c20) = px;
                    global<s32>(0x00818c24) = py;
                    const u8 d = shift(dir, b);
                    global<u8>(0x00818c30) = d;
                    MapCell_SetDirection(cell, nullptr, d);
                    u8 *const next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, Nibble_Scale(MapCell_Direction(cell)));
                    if (next && at<s32>(next, 8) == 1) link(next);
                } else if (check) {
                    if (!test(cell, 0x204, 3, shift(dir, b))) return nullptr;
                } else {
                    soft();
                }
                break;
            }
            case 0xa:
                seenA = 1;
                dirA = shift(dir, b);
                switch (dirA) {
                case 1: xA = px; yA = py - 1; break;
                case 4: xA = px + 1; yA = py; break;
                case 0x10: xA = px; yA = py + 1; break;
                case 0x40: xA = px - 1; yA = py; break;
                default: break;
                }
                if (place) {
                    MapCell_SetDirection(cell, nullptr, shift(dir, b));
                    u8 *const next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, MapCell_Direction(cell));
                    if (next && at<s32>(next, 8) == 1) link(next);
                } else if (check) {
                    if (!test(cell, 0x204, 0, shift(dir, b))) return nullptr;
                } else {
                    soft();
                }
                break;
            case 0xb:
            case 0x10:
                if (place) {
                    MapCell_SetFlags(cell, nullptr, 0x10);
                    MapCell_SetFlags(entry, nullptr, 0x10);
                } else if (check) {
                    if (!test(cell, t == 0xb ? 0x20b : 0x210)) return nullptr;
                    if (MapCell_IsType4(cell)) return nullptr;
                } else if (!test(cell, t == 0xb ? 0x10b : 0x110)) {
                    failed = 1;
                }
                break;
            case 0x17: {
                if (b) {
                    if (check) {
                        if (!test(cell, 0x204)) return nullptr;
                    } else if (!place) {
                        test(cell, 0x104, 0, shift(dir, 0x10));
                    }
                }
                u8 *const next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, shift(dir, b));
                if (check) {
                    if (!next || !test(next, 0x204)) return nullptr;
                } else if (place) {   // (sic: on the cell it faces, none or not)
                    test(next, 0x87);
                    test(next, 0x15);
                    test(next, 0x83);
                    test(next, 0x84);
                    test(next, 0x81);
                } else if (next) {
                    test(next, 0x104, 5, shift(dir, 0x10));
                } else {
                    global<u32>(0x00816d50) = 1;
                }
                break;
            }
            default:
                if (place) break;
                if (check) {
                    if (!test(cell, 0x204)) return nullptr;
                } else {
                    soft();
                }
                break;
            }
        }
    }
    if (!global<u32>(0x008187f8)) g_fp->focusUpdate(0);
    const u8 out9 = (u8)Nibble_Scale(dir9), outA = (u8)Nibble_Scale(dirA);
    if (seen9 || seenA) {
        if (failed) global<u32>(0x00816d50) = 1;
        if (!Map_InBounds(x9, y9)) return nullptr;
        u8 *const behind = cell44(x9, y9);
        if (!Global81b0cc_IfNoFlag()) {
            if (check) {
                if (!test(behind, 0x201, 7, 0)) return nullptr;
            } else if (!place) {
                test(behind, 0x118, 7, out9);
            } else {
                joinPath(behind);
            }
        } else if (check) {
            if (!test(behind, 0x204, 3, out9)) return nullptr;
        } else if (!place) {
            test(behind, 0x104, 3, out9);
        } else {
            u8 *const end = cell44(global<s32>(0x00818c20), global<s32>(0x00818c24));
            MapCell_AddNeighbours(end, nullptr, out9);
            MapCell_SetDirection(end, nullptr, out9);
            test(behind, 0x87);
            test(behind, 3);
            MapCell_AddNeighbours(behind, nullptr, Nibble_Scale(out9));
            MapCell_SetDirection(behind, nullptr, Nibble_Scale(out9));
            MapCell_SetParent(behind, nullptr, at<u16>(end, 0x10));
            MapCell_SetFlags(behind, nullptr, 0x20);
            test(behind, 0x81);
            test(behind, 0x85);
            test(behind, 0x86);
            g_fp->thing4de1f0(global<u8 *>(0x008186ac));
        }
        if (Global81b0cc_IfNoFlag()) {
            if (!Map_InBounds(xA, yA)) return nullptr;
            u8 *const ahead = cell44(xA, yA);
            if (check) {
                if (!test(ahead, 0x201, 0, outA)) return nullptr;
            } else if (place) {
                joinPath(ahead);
            } else {
                test(ahead, 0x118, 4, outA);
            }
        }
    }
    const s32 kind = (s32)Global81ecac_IfNoFlag();
    if (kind > 0 && kind <= 2) {
        const u8 *const d = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, global<u16>(0x008186e0));
        s32 x1, y1, x2, y2;
        switch (angle) {
        case 0x5a: {
            const s32 c = at<s32>(d, 0xbc) - h + v + x, t = y + at<s32>(d, 0xc0) - h - v;
            x1 = c; y1 = t + 1; x2 = c - 1; y2 = t + 6;
            break;
        }
        case 0xb4: {
            const s32 s = at<s32>(d, 0xb4) - 2 * h + x, t = y + at<s32>(d, 0xb8) - 2 * v;
            x1 = s - 5; y1 = t + 1; x2 = s; y2 = t + 3;
            break;
        }
        case 0x10e: {
            const s32 e = x + (at<s32>(d, 0xac) - h - v);
            x1 = e + 1; x2 = e + 3; y1 = at<s32>(d, 0xb0) - v + h + y; y2 = y1 - 4;
            break;
        }
        default:
            x1 = at<s32>(d, 0xa4) + x; y1 = at<s32>(d, 0xa8) + y; x2 = x1 + 8; y2 = y1 - 1;
            break;
        }
        if (!TrackSquare_Check(x1, y1, (u32)angle, place, check, dir)) return nullptr;
        switch (angle) {   // (the original also takes Nibble_Scale of 1 / 4 / 0x40 / 0x10, or x, and drops it)
        case 0x5a: ++x2; ++y2; break;
        case 0: x2 -= 2; ++y2; break;
        case 0xb4: ++x2; y2 -= 2; break;
        case 0x10e: x2 -= 2; y2 -= 2; break;
        default: break;
        }
        if (!TrackSquare_Check(x2, y2, (u32)angle, place, check, dir)) return nullptr;
    }
    return reinterpret_cast<s32 *>(0x00818c20);
}
}
