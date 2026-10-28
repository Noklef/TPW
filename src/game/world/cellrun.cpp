// Runs of type 0x19 map cells: see cellrun.h.
#include "cellrun.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../motion/animator.h"
#include "../model/highlight.h"
#include "cellmodel.h"
#include "dirstep.h"
#include "hoardinggrid.h"
#include "mapcell.h"
#include "mapline.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *cell_at(s32 x, s32 y) { return global<u8 *>(0x007cf83c) + ((((u32)y << 7) + (u32)x + 1) & 0xffff) * 0x28 + 0x6cde * 0x28; }

// The point list: (x, y) pairs at 0x81b740, the count at 0x820a8c.
void add_point(s32 x, s32 y) {
    s32 &count = global<s32>(0x00820a8c);
    if (count > 0x400) return;
    global<s32>(0x0081b740 + count * 8) = x;
    global<s32>(0x0081b744 + count * 8) = y;
    ++count;
}

// Two cells on in the direction (1 north, 4 east, 0x10 south, 0x40 west; others stay).
void move(u8 direction, s32 &x, s32 &y) {
    switch (direction) {
    case 0x01: y -= 2; break;
    case 0x04: x += 2; break;
    case 0x10: y += 2; break;
    case 0x40: x -= 2; break;
    default: break;
    }
}

// The run from the cell at 0x7639f0 / 0x7639f4 (left in x and y where it stops): visit(cell) on each cell, then turned() after a turn at it.
template <typename Visit, typename Turned> void run(s32 &x, s32 &y, Visit visit, Turned turned) {
    u8 direction = 4;
    u32 done = 0, count = 0;
    do {
        u8 *const cell = cell_at(x, y);
        visit(cell);
        if (at<u32>(cell, 8) == 0x19 && MapCell_CornerNeighbours(cell)) {
            direction = static_cast<u8>(Nibble_Scale(direction)) ^ MapCell_Neighbours(cell);
            turned();
        }
        const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2);
        if (next && at<u32>(next, 8) != 0x19) done = 1;
        if (MapCell_NeighbourCount(cell) == 1 && count != 0) done = 1;
        if (!done) move(direction, x, y);
        ++count;
    } while (!done);
}

// The track run from (x, y) shared by TraceA and TraceB (B: no move on reaching an end, and the cell `saved` reset).
void track_run(s32 x, s32 y, u8 direction, bool b, u16 saved, u32 done) {
    do {
        u8 *const cell = cell_at(x, y);
        if (MapCell_IsTypeBOr10(cell)) {
            if (MapCell_CornerNeighbours(cell)) {
                direction = static_cast<u8>(Nibble_Scale(direction)) ^ MapCell_Neighbours(cell);
                add_point(x, y);
                MapCell_SetOverlap(cell, nullptr, 1);
            } else if (MapCell_JunctionNeighbours(cell)) {
                MapCell_SetOverlap(cell, nullptr, 1);
            } else {
                MapCell_StraightNeighbours(cell);  // (its answer unused, sic)
                MapCell_SetOverlap(cell, nullptr, 0);
            }
            if (b && at<u16>(cell, 0) == saved) MapCell_SetOverlap(cell, nullptr, 0);
            if (!MapCell_CornerNeighbours(cell) && !(MapCell_Neighbours(cell) & direction)) done = 1;
            const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2);
            if (next && MapCell_IsTypeDOr12ViaParent(next) && at<u16>(next, 0x10) == at<u16>(cell, 0x10)) {
                const u8 bits = MapCell_Neighbours(next);
                if (static_cast<u8>(Nibble_Scale(direction)) & bits) {
                    done = 1;
                    if (!b) move(direction, x, y);
                }
            }
        }
        if (!done) move(direction, x, y);
    } while (!done);
    add_point(x, y);
    global<s32>(0x0081ede4) = x;
    global<s32>(0x0081ede8) = y;
}

// The animated run shared by AnimateA and AnimateB (see cellrun.h); start's low byte picks the bank. Its answer: the last direction less 1, the
// low byte cleared (the original's EAX).
u32 animate_run(s32 x, s32 y, u8 direction, u32 start) {
    bool more = true;
    do {
        u8 *const cell = cell_at(x, y);
        if (MapCell_IsTypeBDOr10Or12Or19(cell)) {
            if (MapCell_CornerNeighbours(cell)) {
                u8 turn = static_cast<u8>(Nibble_Scale(direction));
                if (!(turn & MapCell_Neighbours(cell))) turn = direction;
                direction = static_cast<u8>(MapCell_Neighbours(cell) ^ turn);
            }
            const u32 thing = at<u32>(cell, 4);
            if (thing != 0) Animator_Play(global<u8 *>(0x007a4610 + thing * 4), static_cast<u8>(start) ? 5 : 0xc, 0, 7, 1.0f, 0);
        } else {
            direction = 0;
        }
        if (!MapCell_CornerNeighbours(cell) && !(direction & MapCell_Neighbours(cell))) more = false;
        const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2);
        if (next && MapCell_IsTypeDOr12ViaParent(next) && at<u16>(next, 0x10) == at<u16>(cell, 0x10) &&
            (static_cast<u8>(Nibble_Scale(direction)) & MapCell_Neighbours(next)))
            more = false;
        move(direction, x, y);
    } while (more);
    return ((u32)direction - 1) & 0xffffff00;
}

const CellRunEnv kGameEnv = {&MapCell_ApplyModel,
                             reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)0x005370e0),
                             reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x00538fc0),
                             reinterpret_cast<void(__fastcall *)(u8 *, void *, s32, s32, u32)>((uintptr_t)0x0053b280),
                             reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)0x0053bc60),
                             reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)0x00545450)};
const CellRunEnv *g_env = &kGameEnv;

u32 has_low(const u8 *cell) { return MapCell_HasLowFlags(cell); }
u32 is_end(const u8 *cell) { return MapCell_IsTypeBDOr10Or12Or19(cell); }
u16 owner(const u8 *cell) { return at<u16>(cell, 0x10); }
u8 *thing_of(u32 thing) { return global<u8 *>(0x007a4610 + thing * 4); }
u8 *step(const u8 *cell, u32 direction) { return Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2); }

// The cell's low flags cleared (0xf) and then set to value; with of_a, value is (a is an end ? on : off), looked at after the clear.
void shape(u8 *cell, u32 value) {
    MapCell_ClearFlags(cell, nullptr, 0xf);
    MapCell_SetFlags(cell, nullptr, value);
}
void shape_by(u8 *cell, const u8 *a, u32 on, u32 off) {
    MapCell_ClearFlags(cell, nullptr, 0xf);
    MapCell_SetFlags(cell, nullptr, is_end(a) ? on : off);
}

// A rule's effect: the cell 8 or 2 (on and off: by a), then 4 when other has low flags.
void apply(u8 *cell, const u8 *a, const u8 *other, u32 on, u32 off) {
    if (has_low(cell)) shape_by(cell, a, on, off);
    if (has_low(other)) shape(cell, 4);
}

void play(const u8 *cell, u32 bank5) {
    const u32 thing = at<u32>(cell, 4);
    if (thing != 0) Animator_Play(thing_of(thing), static_cast<u8>(bank5) ? 5 : 0xc, 0, 7, 1.0f, 0);
}

// The turn at a corner (as AnimateA's).
void turn(const u8 *cell, u8 &direction) {
    u8 back = static_cast<u8>(Nibble_Scale(direction));
    if (!(back & MapCell_Neighbours(cell))) back = direction;
    direction = static_cast<u8>(MapCell_Neighbours(cell) ^ back);
}

// The end of a run at a cell not at a corner whose neighbour bits lack the direction: its thing flashes or stops flashing.
bool stop_here(const u8 *cell, u8 direction, u32 flash) {
    if (MapCell_CornerNeighbours(cell) || (MapCell_Neighbours(cell) & direction)) return false;
    const u32 thing = at<u32>(cell, 4);
    if (thing != 0) {
        u8 *const holder = thing_of(thing);
        const u32 flags = at<u32>(holder, 4);
        if (static_cast<u8>(flash)) {
            if (!(flags & 0x4000000)) Highlight_Flash(holder);
        } else if (flags & 0x4000000) {
            Highlight_Unflash(holder);
        }
    }
    return true;
}

// A next cell (two on) that is an end of the cell's +0x10 word with the reverse side.
bool end_ahead(const u8 *cell, u8 direction) {
    const u8 *const next = step(cell, direction);
    if (!next || !MapCell_IsTypeDOr12ViaParent(next) || owner(next) != owner(cell)) return false;
    const u8 bits = MapCell_Neighbours(next);
    return (static_cast<u8>(Nibble_Scale(direction)) & bits) != 0;
}

// A cell as CellRun_Connect takes it: one of type 0xc or 0x11 stands for the cell its +0x10 word names.
u8 *taken(u8 *cell) { return MapCell_IsTypeCOr11(cell) ? global<u8 *>(0x007cf83c) + ((u32)owner(cell) + 0x6cde) * 0x28 : cell; }

// The cell's (x, y) from its id less 1 (x the low 7 bits, y the rest).
void cell_xy(const u8 *cell, s32 &x, s32 &y) {
    const u32 id = (u32)at<u16>(cell, 0) - 1;
    x = static_cast<s32>(id & 0x7f);
    y = static_cast<s32>(id >> 7);
}

}  // namespace

extern "C" {

void __cdecl TrackRun_TraceA(const u8 *thing) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    const u8 direction = static_cast<u8>(Position_StepA(&x, &y, at<s32>(thing, 0x10)));
    global<u8>(0x00818c50) = direction;
    global<s32>(0x00820a8c) = 0;
    if (MapCell_IsTypeBOr10(cell_at(x, y))) add_point(x, y);
    track_run(x, y, direction, false, 0, 0);
}

void __cdecl TrackRun_TraceB(const u8 *thing) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    u16 saved = 0;
    const u8 direction = static_cast<u8>(Position_StepB(&x, &y, at<s32>(thing, 0x10)));
    global<u8>(0x00818c50) = direction;
    if (Map_InBounds(global<s32>(0x007639e0), global<s32>(0x007639e4)))
        saved = static_cast<u16>((static_cast<u32>(global<s32>(0x007639e4)) << 7) + static_cast<u32>(global<s32>(0x007639e0)) + 1);
    global<s32>(0x00820a8c) = 0;
    u8 *const cell = cell_at(x, y);
    if (MapCell_IsTypeBOr10(cell)) add_point(x, y);
    track_run(x, y, direction, true, saved, MapCell_IsTypeBDOr10Or12Or19(cell) ? 0 : 1);
}

void __cdecl CellRun_Mark(u32 set) {
    s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
    global<u8>(0x00818c50) = 4;
    run(
        x, y,
        [set](u8 *cell) {
            if (static_cast<u8>(set))
                MapCell_SetFlags(cell, nullptr, MapCell_Flags(cell) | 0x20u);
            else
                MapCell_ClearFlags(cell, nullptr, 0x20);
        },
        [] {});
}

void __cdecl CellRun_Corners() {
    s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
    global<u8>(0x00818c50) = 4;
    global<s32>(0x0081b740) = x;
    global<s32>(0x0081b744) = y;
    global<s32>(0x00820a8c) = 1;
    run(x, y, [](u8 *) {}, [&x, &y] { add_point(x, y); });
    add_point(x, y);
    global<s32>(0x0081ede4) = x;
    global<s32>(0x0081ede8) = y;
}

u32 __cdecl TrackRun_AnimateA(const u8 *thing, u32 start) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    u8 direction;
    switch (at<s32>(thing, 0x10)) {  // (Position_StepA inlined)
    case 0: x += 4; y += 1; direction = 4; break;
    case 90: x += 1; y -= 5; direction = 1; break;
    case 180: x -= 5; y -= 2; direction = 0x40; break;
    case 270: x -= 2; y += 4; direction = 0x10; break;
    default: direction = static_cast<u8>(start); break;  // (sic)
    }
    return animate_run(x, y, direction, start);
}

u32 __cdecl TrackRun_AnimateB(const u8 *thing, u32 start) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    u32 code;
    switch (at<s32>(thing, 0x10)) {  // (Position_StepB inlined)
    case 0: x -= 2; y += 1; code = 4; break;
    case 90: x += 1; y += 1; code = 1; break;
    case 180: x += 1; y -= 2; code = 0x40; break;
    case 270: x -= 2; y -= 2; code = 0x10; break;
    default: code = start; break;  // (sic)
    }
    return animate_run(x, y, static_cast<u8>(Nibble_Scale(code)), start);
}

u32 __cdecl PairTable_Add(u32 a, u32 b) {
    const s32 count = global<s32>(0x00820a8c);
    if (count > 0x400)
        return 0;
    global<u32>(0x0081b740 + 8 * count) = a;
    global<u32>(0x0081b744 + 8 * count) = b;
    global<s32>(0x00820a8c) = count + 1;
    return 1;
}

s32 __cdecl PairTable_Commit() {
    const s32 count = global<s32>(0x00820a8c);
    for (s32 i = 0; i < count; i++) {
        global<u32>(0x00818d98 + 8 * (u32)i) = global<u32>(0x0081b740 + 8 * (u32)i);
        global<u32>(0x00818d9c + 8 * (u32)i) = global<u32>(0x0081b744 + 8 * (u32)i);
    }
    global<s32>(0x00820a90) = count;
    global<u32>(0x00820a94) = 0;
    return count > 0 ? count : 0;
}

void CellRun_SetEnv(const CellRunEnv *env) { g_env = env ? env : &kGameEnv; }

u8 __cdecl TrackRun_UpdateA(u8 *thing, u32 flash, u32 start, u32 bank5) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    u8 direction;
    switch (at<s32>(thing, 0x10)) {  // (Position_StepA inlined)
    case 0: x += 4; y += 1; direction = 4; break;
    case 90: x += 1; y -= 5; direction = 1; break;
    case 180: x -= 5; y -= 2; direction = 0x40; break;
    case 270: x -= 2; y += 4; direction = 0x10; break;
    default: direction = static_cast<u8>(start); break;  // (sic)
    }
    global<u8>(0x00818c50) = direction;
    bool done = false;
    u8 found = 0;
    if (!is_end(cell_at(x, y))) found = 1, done = true;
    do {
        u8 *const cell = cell_at(x, y);
        if (MapCell_IsTypeBOr10(cell)) {
            if (MapCell_CornerNeighbours(cell)) {
                turn(cell, direction);
            } else if (has_low(cell)) {
                const u8 *const a = step(cell, direction);
                const u8 *const b = step(cell, Nibble_Scale(direction));
                if (MapCell_NeighbourCount(cell) == 2) {
                    if (has_low(a) && has_low(b))
                        shape(cell, 4);
                    else if (!has_low(a) && !has_low(b))
                        shape(cell, 1);
                    else if (has_low(a) && !has_low(b))
                        shape(cell, 2);
                    else if (!has_low(a) && has_low(b))
                        shape(cell, 8);
                }
                if (!is_end(b) || (is_end(b) && owner(cell) != owner(b)) ||
                    (is_end(a) && MapCell_NeighbourCount(cell) == 1 && (MapCell_Neighbours(cell) & direction)))
                    apply(cell, a, a, 8, 2);
                if (!is_end(a) || (is_end(a) && owner(cell) != owner(a)) || (is_end(b) && MapCell_NeighbourCount(cell) == 1 && [&] {
                        const u8 bits = MapCell_Neighbours(cell);
                        return (static_cast<u8>(Nibble_Scale(direction)) & bits) != 0;
                    }()))
                    apply(cell, a, b, 8, 2);
                g_env->cell53afa0(cell);
                if (static_cast<u8>(start)) play(cell, bank5);
            }
            if (stop_here(cell, direction, flash)) done = true;
            if (end_ahead(cell, direction)) {
                found = 1;
                done = true;
                move(direction, x, y);
            }
        }
        if (!done) move(direction, x, y);
    } while (!done);
    return found;
}

void __cdecl TrackRun_UpdateB(u8 *thing, u32 flash, u32 start, u32 bank5) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    u32 code;
    switch (at<s32>(thing, 0x10)) {  // (Position_StepB inlined)
    case 0: x -= 2; y += 1; code = 4; break;
    case 90: x += 1; y += 1; code = 1; break;
    case 180: x += 1; y -= 2; code = 0x40; break;
    case 270: x -= 2; y -= 2; code = 0x10; break;
    default: code = start; break;  // (sic)
    }
    u8 direction = static_cast<u8>(Nibble_Scale(code));
    global<u8>(0x00818c50) = direction;
    bool done = !is_end(cell_at(x, y));
    do {
        u8 *const cell = cell_at(x, y);
        if (MapCell_IsTypeBOr10(cell)) {
            if (MapCell_CornerNeighbours(cell)) {
                turn(cell, direction);
            } else if (has_low(cell)) {
                const u8 *const a = step(cell, direction);
                const u8 *const b = step(cell, Nibble_Scale(direction));
                const u32 count = MapCell_NeighbourCount(cell);
                if (static_cast<u8>(start)) play(cell, bank5);
                if (count == 2) {
                    if (has_low(a) && has_low(b))
                        shape(cell, 4);
                    else if (!has_low(a) && !has_low(b))
                        shape(cell, 1);
                    else if (has_low(a) && !has_low(b))
                        shape(cell, 8);
                    else if (!has_low(a) && has_low(b))
                        shape(cell, 2);
                }
                if (!is_end(b) || (is_end(b) && owner(cell) != owner(b))) apply(cell, a, a, 8, 2);
                if (is_end(b) && MapCell_NeighbourCount(cell) == 1 && (MapCell_Neighbours(cell) & direction)) apply(cell, a, a, 8, 2);
                if (!is_end(a) || (is_end(a) && owner(cell) != owner(a))) apply(cell, a, b, 8, 2);
                if (is_end(a) && MapCell_NeighbourCount(cell) == 1) {
                    const u8 bits = MapCell_Neighbours(cell);
                    if (static_cast<u8>(Nibble_Scale(direction)) & bits) apply(cell, a, b, 2, 8);
                }
                g_env->cell53afa0(cell);
            }
            if (stop_here(cell, direction, flash)) done = true;
            if (end_ahead(cell, direction)) done = true;
        }
        if (!done) move(direction, x, y);
    } while (!done);
}

void __cdecl TrackCell_Relink(u32 corners) {
    const s32 x = global<s32>(0x007639e0), y = global<s32>(0x007639e4);
    if (!Map_InBounds(x, y)) return;
    u8 *const cell = cell_at(x, y);
    if (!is_end(cell)) return;
    u8 d = 1;
    do {
        u8 *const next = step(cell, d);
        if (!(d & MapCell_Neighbours(cell))) {   // (a side not linked: linked to an end of the same +0x10 word two on)
            if (next && MapCell_IsTypeDOr12ViaParent(next) && owner(next) == owner(cell)) {
                MapCell_SetDirection(cell, nullptr, Nibble_Scale(d));
                MapCell_AddNeighbours(cell, nullptr, d);
                g_env->cell53afa0(cell);
            }
        } else {   // (a linked side: unlinked unless the cell two on is an end, or a piece linked back; next is not checked for null, sic)
            bool keep;
            if (!is_end(next))
                keep = MapCell_IsTypeDOr12ViaParent(next) != 0;
            else
                keep = (static_cast<u8>(Nibble_Scale(d)) & MapCell_Neighbours(next)) != 0;
            if (!keep) {
                MapCell_RemoveNeighbours(cell, nullptr, d);
                g_env->cell53afa0(cell);
            }
        }
        d = static_cast<u8>(Byte_ShiftLog2(d, 4));
    } while (d != 1);
    if (!static_cast<u8>(corners) || !MapCell_CornerNeighbours(cell)) return;
    do {
        if (d & MapCell_Neighbours(cell)) {
            u8 *const next = step(cell, d);
            if (next) {
                if (!MapCell_IsTypeDOr12ViaParent(next)) {
                    MapCell_RemoveNeighbours(cell, nullptr, d);
                    g_env->cell53afa0(cell);
                    MapCell_RemoveNeighbours(next, nullptr, Nibble_Scale(d));
                    g_env->cell53afa0(next);
                } else {
                    MapCell_ClearDirection(cell, nullptr, 0xff);
                    MapCell_SetDirection(cell, nullptr, Nibble_Scale(d));
                }
            }
        }
        d = static_cast<u8>(Byte_ShiftLog2(d, 4));
    } while (d != 1);
}

u32 __cdecl CellRun_Lay(s32 x, s32 y, s32 endX, s32 endY, u32 direction, u32 setDirection) {
    u8 dir = static_cast<u8>(direction);
    global<s32>(0x00818c0c) = 0;
    global<s32>(0x00818c08) = 0;
    global<s32>(0x008186e4) = 0x400;
    global<s32>(0x008186e8) = 0x400;
    bool done = false;
    do {
        u8 *const cell = cell_at(x, y);
        if (at<u32>(cell, 8) != 0x19 && !(global<u8>(0x0081b4cc) && MapCell_NeighbourCount(cell))) {
            dir = 0;
        } else {
            if (MapCell_CornerNeighbours(cell)) {   // (a corner turns: the reverse side, or the direction, against the neighbours)
                u8 back = static_cast<u8>(Nibble_Scale(dir));
                if (!(back & MapCell_Neighbours(cell))) back = dir;
                dir = MapCell_Neighbours(cell) ^ back;
            }
            if (MapCell_JunctionNeighbours(cell)) {   // (a junction: the first side from the left one round that is a neighbour)
                u8 side = 0;
                u8 bit = static_cast<u8>(Byte_ShiftLog2(dir, 0x40));
                do {
                    if (bit & MapCell_Neighbours(cell)) side = bit;
                    bit = static_cast<u8>(Byte_ShiftLog2(bit, 4));
                } while (!side);
                u8 clear;
                if (dir != side) {
                    MapCell_SetDirection(cell, nullptr, side);
                    MapCell_RemoveNeighbours(cell, nullptr, dir);
                    clear = static_cast<u8>(Nibble_Scale(side));
                } else {
                    clear = static_cast<u8>(~(static_cast<u8>(Nibble_Scale(dir)) | dir));
                }
                MapCell_RemoveNeighbours(cell, nullptr, clear);
                g_env->cell5370e0(cell, nullptr, 0x81, 0, 0);
                g_env->cell53afa0(cell);
                dir = side;
            }
        }
        if (x == endX && y == endY) done = true;
        if (static_cast<u8>(setDirection) && MapCell_Direction(cell) != dir) {
            MapCell_SetDirection(cell, nullptr, dir);
            g_env->cell5370e0(cell, nullptr, 0x81, 0, 0);
        }
        static const u32 kPairs[] = {0x007cdba8, 0x007cdbc8, 0x007cdbb8};   // (the pairs of the directions 0x10, 4 and 8)
        for (const u32 pair : kPairs) {
            const s32 *const step = reinterpret_cast<const s32 *>((uintptr_t)pair);
            if (u8 *const next = Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, step[0], step[1], 1)) {
                MapCell_SetParent(next, nullptr, at<u16>(cell, 0));
                g_env->cell538fc0(next, nullptr, 0xc);
            }
        }
        move(dir, x, y);
        if (x < global<s32>(0x008186e4)) global<s32>(0x008186e4) = x;
        if (y < global<s32>(0x008186e8)) global<s32>(0x008186e8) = y;
        if (x > global<s32>(0x00818c0c)) global<s32>(0x00818c0c) = x;
        if (y > global<s32>(0x00818c08)) global<s32>(0x00818c08) = y;
    } while (!done);
    HoardingGrid_Fill();
    return 1;
}

s32 __cdecl CellRun_Connect(u8 *from, u8 *to, u32 link) {
    u8 *sideA = nullptr, *sideB = nullptr;
    s32 count = 0;
    u32 aJunction = 0, bJunction = 0;
    from = taken(from);
    to = taken(to);
    const u16 wordFrom = MapCell_Word24(from), wordTo = MapCell_Word24(to);
    if (!link || static_cast<s16>(wordTo) - static_cast<s16>(wordFrom) >= 0) {
        s32 x, y;
        cell_xy(from, x, y);
        s32 lineX = x, lineY = y, endX = -1, endY = -1, prevX = -1, prevY = -1;
        u8 dir = MapCell_Direction(from);
        bool done = false;
        do {
            u8 *const cell = taken(cell_at(x, y));
            if (!MapCell_IsTypeBDOr10Or12Or19(cell)) return -1;
            const u16 word = MapCell_Word24(cell);
            if (link) {
                if (!sideA) {
                    if (word == wordFrom) aJunction = 0, sideA = cell;
                    if (!sideA && MapCell_JunctionNeighbours(cell)) sideA = cell, aJunction = 1;
                }
                if (word == wordTo) sideB = cell, bJunction = 0;
                if (!sideB && MapCell_JunctionNeighbours(cell)) sideB = cell, bJunction = 1;
            }
            if ((MapCell_IsTypeBDOr10Or12Or19(cell) || at<u32>(cell, 8) == 0x19) && MapCell_CornerNeighbours(cell)) {
                if (count) {
                    u8 back = static_cast<u8>(Nibble_Scale(dir));
                    if (!(back & MapCell_Neighbours(cell))) back = dir;
                    dir = MapCell_Neighbours(cell) ^ back;
                }
                endX = prevX;
                endY = prevY;
            }
            if (cell == to) done = true, endX = x, endY = y;
            if (link) MapCell_SetDirection(cell, nullptr, MapCell_JunctionNeighbours(cell) ? static_cast<u8>(MapCell_Direction(cell) | dir) : dir);
            ++count;
            if (lineX != -1 && endX != -1) {
                Map_ApplyLine(0x10f, lineX, lineY, &endX, &endY);
                lineX = x;
                lineY = y;
                endX = endY = -1;
            }
            prevX = x;
            prevY = y;
            move(dir, x, y);
        } while (!done);
        if (link && sideA && sideB && sideA != sideB) {   // (the side run from A to B)
            cell_xy(sideA, x, y);
            dir = MapCell_Direction(MapCell_JunctionNeighbours(sideA) ? from : sideA);
            bool stop = false;
            do {
                u8 *const cell = taken(cell_at(x, y));
                MapCell_Word24(cell);   // (sic: unused)
                if (!MapCell_IsTypeBDOr10Or12Or19(cell) && at<u32>(cell, 8) != 0x19) {
                    stop = true;
                } else if (MapCell_SideDirectionCount(cell) == 1) {
                    dir = MapCell_Direction(cell);
                } else {
                    MapCell_SetDirection(cell, nullptr, static_cast<u8>(MapCell_Direction(cell) & ~dir));
                }
                if (cell == sideB) stop = true;
                if (MapCell_JunctionNeighbours(cell))
                    g_env->cell53b280(cell, nullptr, -1, -1, 1);
                else if (MapCell_StraightNeighbours(cell) || MapCell_CornerNeighbours(cell) || MapCell_NeighbourCount(cell) == 1)
                    g_env->cell53b280(cell, nullptr, 0, 0, 1);
                move(dir, x, y);
            } while (!stop);
        }
    }
    if (link) {
        const bool shaped = MapCell_StraightNeighbours(from) || MapCell_CornerNeighbours(from);
        if (u8 *const track = global<u8 *>(0x008186ac)) g_env->track545450(at<u32>(track, 0x28));
        s32 x, y;
        if (static_cast<s16>(wordTo) - static_cast<s16>(wordFrom) <= 1) {
            cell_xy(from, x, y);
            if (shaped) g_env->at53bc60(x, y, 0);
        }
        if (aJunction) {
            cell_xy(from, x, y);
            if (shaped) g_env->at53bc60(x, y, 0);
        }
        if (bJunction) {   // (the direction byte over from's pointer: a direction 0 passes its upper three bytes on, sic)
            const u32 d = ((u32)(uintptr_t)from & 0xffffff00) | MapCell_Direction(from);
            cell_xy(to, x, y);
            if (shaped) g_env->at53bc60(x, y, Nibble_Scale(d));
        }
        if (sideA == sideB && sideA && shaped) g_env->cell53b280(sideA, nullptr, 0, 0, 1);
    }
    return count;
}

}  // extern "C"
