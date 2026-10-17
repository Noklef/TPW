// A straight run of map cells: see mapline.h.
#include "mapline.h"

#include <cstdint>

#include "mapcell.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const MapLineEnv kGameEnv = {
    reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, s32, s32)>((uintptr_t)0x005370e0),
};
const MapLineEnv *g_env = &kGameEnv;

u32 &gLineFirst() { return global<u32>(0x00820ac8); }
u32 &gLineLast() { return global<u32>(0x00820ad0); }

s32 abs32(u32 v) {
    const u32 s = static_cast<u32>(static_cast<s32>(v) >> 31);
    return static_cast<s32>((v ^ s) - s);
}

u8 *cell(s32 x, s32 y) {
    const u32 id = ((static_cast<u32>(y) << 7) + static_cast<u32>(x) + 1) & 0xffff;
    return reinterpret_cast<u8 *>((uintptr_t)(global<u32>(0x007cf83c) + (id + 0x6cde) * 0x28));
}

// The end pulled in to an even distance from `from` (written back), and the step toward it: -2, 0 or 2.
s32 pull_end(s32 from, s32 *end) {
    u32 d = static_cast<u32>(from) - static_cast<u32>(*end);
    if (static_cast<s32>(d) > 0 && (d & 1)) ++d;
    const s32 e = static_cast<s32>(static_cast<u32>(from) - static_cast<u32>(static_cast<s32>(d) / 2) * 2);
    *end = e;
    return from > e ? -2 : from == e ? 0 : 2;
}

// LinkCell's link of the cell to the one two away by the direction pair at `pair`: both of a linking type (only 0x19 when `only19`), each with
// fewer than 4 neighbour bits, the other's owner (+0x10) the focus cell's id or 0.
void link(u8 *cell, u32 pair, u32 selfBit, u32 otherBit, bool only19) {
    u8 *const next = Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, global<s32>(pair), global<s32>(pair + 4), 2);
    if (!next) return;
    const u32 type = *reinterpret_cast<const u32 *>(next + 8);
    if (only19 ? type != 0x19 : type != 0xb && type != 0xd && type != 0x10 && type != 0x12 && type != 0x19) return;
    if (static_cast<s32>(MapCell_NeighbourCount(cell)) >= 4 || static_cast<s32>(MapCell_NeighbourCount(next)) >= 4) return;
    const u16 owner = *reinterpret_cast<const u16 *>(next + 0x10);
    if (owner != global<u16>(0x00818698) && owner != 0) return;
    MapCell_AddNeighbours(cell, nullptr, selfBit);
    MapCell_AddNeighbours(next, nullptr, otherBit);
}

}  // namespace

void MapLine_SetEnv(const MapLineEnv *e) { g_env = e ? e : &kGameEnv; }
const MapLineEnv *MapLine_GetEnv() { return g_env; }

extern "C" {

u32 __cdecl Map_ApplyLine(u32 arg, s32 x, s32 y, s32 *xEnd, s32 *yEnd) {
    gLineFirst() = 1;
    gLineLast() = 0;
    const s32 dx = abs32(static_cast<u32>(*xEnd) - static_cast<u32>(x));
    const s32 dy = abs32(static_cast<u32>(*yEnd) - static_cast<u32>(y));
    s32 stepX = 0, stepY = 0;
    s32 *pos, *end;
    if (dx > dy) {
        stepX = pull_end(x, xEnd);
        pos = &x;
        end = xEnd;
    } else {
        stepY = pull_end(y, yEnd);
        pos = &y;
        end = yEnd;
    }
    const s32 step = stepX | stepY;
    if (*pos != *end) {
        do {
            if (Map_InBounds(x, y)) {
                const u32 r = g_env->cellOp(cell(x, y), nullptr, arg, stepX, stepY);
                gLineFirst() = 0;
                if (r == 0) return 0;
            }
            *pos = static_cast<s32>(static_cast<u32>(*pos) + static_cast<u32>(step));
        } while (*pos != *end);
    }
    gLineLast() = 1;
    if (!Map_InBounds(x, y)) return 1;
    const u32 r = g_env->cellOp(cell(x, y), nullptr, arg, stepX, stepY);
    gLineFirst() = 0;
    gLineLast() = 0;
    return r != 0 ? 1 : 0;
}

void __fastcall MapLine_LinkCell(u8 *cell, void *, u32 kind, s32 stepX, s32 stepY) {
    s32 sx = stepX, sy = stepY;
    if (gLineFirst()) {
        sx = global<s32>(0x00763b00);
        sy = global<s32>(0x00763b04);
        if (sx == 0x80 || sy == 0x80) sx = sy = 0;
    }
    if (kind == 0xb || kind == 0xd || kind == 0x10 || kind == 0x12 || kind == 0x19) {
        const bool only19 = kind == 0x19;
        if (sy == 2) link(cell, 0x007cdba0, 0x01, 0x10, only19);
        if (sx == -2) link(cell, 0x007cdbc8, 0x04, 0x40, only19);
        if (sy == -2) link(cell, 0x007cdba8, 0x10, 0x01, only19);
        if (sx == 2) link(cell, 0x007cdbd0, 0x40, 0x04, only19);
    }
    global<s32>(0x00763b00) = stepX;
    global<s32>(0x00763b04) = stepY;
}

}  // extern "C"
