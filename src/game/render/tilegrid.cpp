#include "tilegrid.h"

#include <cstdint>

#include "../core/addrcalls.h"
#include "batchquad.h"
#include "../math/x87.h"
#include "../gfx/texcache.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TileGridEnv kPortEnv = {&Batch_DrawQuad};
const TileGridEnv *g_env = &kPortEnv;

// The constants, as the game's memory holds them: 1/64, 1, 0, -64, -0.03, 0.3.
const float &kStep = global<float>(0x0070186c);
const float &kOne = global<float>(0x00701870);
const float &kNone = global<float>(0x00701874);
const float &kLessCell = global<float>(0x00701878);
const float &kLessLift = global<float>(0x0070187c);
const float &kTrim = global<float>(0x00701880);

s32 Cell(float place) { return (s32)(long long)(X87Reg(place) * kStep).v; }

// The first and the last cell's part of a cell: where the rectangle cuts it.
X87Reg Cut(float place, s32 cell) { return (place - X87Reg((double)(s32)((u32)cell << 6))) * kStep; }

// One axis: each cell's part (a, b in `parts`) and its screen place at both ends (in `places`, negated for y).
void Axis(const u8 *object, u32 low, u32 high, u32 lowScreen, s32 first, s32 last, float slope, float *parts, float *places, bool negate) {
    X87Reg place = X87Reg(at<float>(object, lowScreen));
    for (s32 k = first; k <= last; ++k) {
        float *const part = parts + (k - first) * 2, *const out = places + (k - first) * 2;
        part[0] = k == first ? (float)Cut(at<float>(object, low), first) : kNone;
        const X87Reg end = k == last ? Cut(at<float>(object, high), last) : X87Reg(kOne);
        part[1] = (float)end;
        out[0] = (float)(negate ? X87Reg(-place.v) : place);
        place = place - (end - part[0]) * slope * kLessCell;
        out[1] = (float)(negate ? X87Reg(-place.v) : place);
    }
}

AddrCalls g_touchCalls;

}  // namespace

void TileGrid_SetEnv(const TileGridEnv *env) { g_env = env ? env : &kPortEnv; }
void TileGrid_SetCalls(const AddrCall *table, u32 count) { g_touchCalls.Set(table, count); }

u32 __fastcall TileGrid_TouchVisible(u8 *grid) {
    if (!at<u32>(grid, 0x1c)) return 0;
    const s32 left = Cell(at<float>(grid, 0x20));
    const s32 right = Cell(at<float>(grid, 0x28));
    const s32 last = Cell(at<float>(grid, 0x2c));
    for (s32 y = Cell(at<float>(grid, 0x24)); y <= last; y++)
        for (s32 x = left; x <= right; x++)
            g_touchCalls.Th(0x58c9f0, gTexcache_8c4b44, ((u32)y * at<u32>(grid, 0x10) + (u32)x + at<u32>(grid, 0x48)) * 0x60u + at<u32>(grid, 0x1c), 0u);
    return 1;
}

u32 __fastcall TileGrid_Draw(u8 *object, void *) {
    if (!at<u8 *>(object, 0x1c))
        return 0;
    const s32 x0 = Cell(at<float>(object, 0x20));
    const s32 x1 = Cell(at<float>(object, 0x28));
    const s32 y0 = Cell(at<float>(object, 0x24));
    const s32 y1 = Cell(at<float>(object, 0x2c));
    const float width = (float)(X87Reg(at<float>(object, 0x28)) - at<float>(object, 0x20));
    const float height = (float)(X87Reg(at<float>(object, 0x2c)) - at<float>(object, 0x24));
    const float slopeX = (float)((X87Reg(at<float>(object, 0x38)) - at<float>(object, 0x30)) / width);
    const float slopeY = (float)((X87Reg(at<float>(object, 0x3c)) - at<float>(object, 0x34)) / height);

    // As the original's frame: the x parts, the y parts, the v places, the u places, 32 pairs each (rows over 32 run on into the next, as there).
    // Sic, not kept: the original's last table ends at its return address. Column 33 (or row 65, through the u places) writes a float there, then the caller's frame, so its RET faults or
    // runs from that float's bits. Here the same writes land on whatever follows this array. Real data stays far below it: the only rectangle is ParkMap_LayOut's, kept inside a 512 x 512 2dmap.tga (8 by 8).
    float tables[0x100];
    float *const partsX = tables, *const partsY = tables + 0x40, *const placesY = tables + 0x80, *const placesX = tables + 0xc0;
    Axis(object, 0x20, 0x28, 0x30, x0, x1, slopeX, partsX, placesX, false);
    Axis(object, 0x24, 0x2c, 0x34, y0, y1, slopeY, partsY, placesY, true);

    // The spec, its points 0x40 on as in the original's frame.
    // Sic, not kept: the original never sets the polygon flags at +0, so they start as what its frame held, and the batch reads them. 0 here.
    u8 spec[0x80] = {};
    u8 *const points = spec + 0x40;
    at<u8 *>(spec, 4) = points;
    at<u32>(spec, 8) = 0xffffffff;
    at<u32>(spec, 0x28) = 2;
    X87Reg z = X87Reg(at<float>(object, 0x44));
    if (global<u32>(0x008bd524) == 4)
        z = z - kLessLift;
    for (u32 k = 0; k < 4; ++k) {
        at<float>(points, k * 0x10 + 8) = (float)z;
        at<u32>(points, k * 0x10 + 0xc) = 0x3f800000;
    }

    for (s32 y = y0; y <= y1; ++y) {
        const float *const partY = partsY + (y - y0) * 2, *const placeY = placesY + (y - y0) * 2;
        for (s32 x = x0; x <= x1; ++x) {
            const float *const partX = partsX + (x - x0) * 2, *const placeX = placesX + (x - x0) * 2;
            const u8 *const cell = at<u8 *>(object, 0x1c) + (u32)(y * at<s32>(object, 0x10) + x + at<s32>(object, 0x48)) * 0x60;
            const u32 flags = at<u32>(cell, 8);
            at<u32>(spec, 0x10) = flags & 0x1000 ? at<u32>(cell, 0x1c) : 0;
            if (at<u32>(cell, 8) & 0x100000)
                at<u32>(spec, 0) |= 4;

            u32 batchFlags = (at<u32>(object, 0x40) & 0xff000000) == 0xff000000 ? 0x1256 : 0x556;
            if (!at<u32>(object, 0x5c))
                batchFlags |= 0x20000;
            at<u32>(spec, 0xc) = batchFlags;

            at<float>(points, 0) = placeX[0];
            at<float>(points, 4) = placeY[1];
            at<float>(points, 0x10) = placeX[1];
            at<float>(points, 0x14) = placeY[1];
            at<float>(points, 0x20) = placeX[1];
            at<float>(points, 0x24) = placeY[0];
            at<float>(points, 0x30) = placeX[0];
            at<float>(points, 0x34) = placeY[0];

            at<float>(spec, 0x18) = (float)(X87Reg(at<float>(cell, 0x20)) * partX[0] + at<float>(cell, 0x28));
            at<float>(spec, 0x20) = (float)(X87Reg(at<float>(cell, 0x20)) * partX[1] + at<float>(cell, 0x28));
            at<float>(spec, 0x24) = (float)(X87Reg(at<float>(cell, 0x24)) * partY[0] + at<float>(cell, 0x2c));
            at<float>(spec, 0x1c) = (float)(X87Reg(at<float>(cell, 0x24)) * partY[1] + at<float>(cell, 0x2c));

            const u8 *texture = at<const u8 *>(cell, 0x18);
            if (texture)
                texture = at<const u8 *>(texture, 0);
            if (texture) {
                const X87Reg endU = X87Reg(at<float>(spec, 0x20));
                const X87Reg trimU = kTrim / X87Reg((double)at<s32>(texture, 8));
                const X87Reg startU = trimU + at<float>(spec, 0x18);
                at<float>(spec, 0x18) = (float)startU;
                at<float>(spec, 0x20) = (float)(endU - trimU);
                const X87Reg trim = X87Reg(global<u8>((u32)(uintptr_t)&gView_8bcbc8) & 2 ? kOne : kTrim);
                const X87Reg endV = X87Reg(at<float>(spec, 0x1c));
                const X87Reg trimV = trim / X87Reg((double)at<s32>(texture, 0xc));
                at<float>(spec, 0x1c) = (float)(endV - trimV);
                at<float>(spec, 0x24) = (float)(trimV + at<float>(spec, 0x24));
            }
            if (!g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, spec, 0xfffffffd))
                return 0;
        }
    }
    return 1;
}
