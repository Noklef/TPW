#include "route.h"

#include <cstdint>
#include <cstring>

#include "linestep.h"
#include "mapcell.h"
#include "wallfollow.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 StepX(u32 d) { return global<u8>(0x007622b0 + d * 2); }
u8 StepY(u32 d) { return global<u8>(0x007622b1 + d * 2); }

u32 Blocked(u8 x, u8 y, u32 dir) { return MapCell_StepBlocked(x, y, dir, global<s32>(0x007cf3c0)); }

// Whether the straight walk (LineStep_Init's) from one cell reaches the other.
bool Reaches(u16 from, u16 to) {
    LineStep_Init(from, to);
    u8 x = (u8)from, y = (u8)(from >> 8), lastX = 0, lastY = 0;
    u32 dir = 0xffffffff;
    bool first = true;
    while (x != (u8)to || y != (u8)(to >> 8)) {
        const u32 previous = dir;
        s32 &sum = global<s32>(0x007cf398);
        if (sum >= global<s32>(0x007cf3a8)) {
            dir = global<u32>(0x007cf3b4);
            sum -= global<s32>(0x007cf3a8);
        } else {
            dir = global<u32>(0x007cf3b0);
            sum += global<s32>(0x007cf3ac);
        }
        if (Blocked(x, y, dir))
            return false;
        if (first) {
            first = false;
        } else if (dir != previous) {  // a turn: each way round the corner open, or its second step blocked too
            if (!Blocked(x, y, previous) && Blocked((u8)(x + StepX(previous)), (u8)(y + StepY(previous)), dir))
                return false;
            if (!Blocked(lastX, lastY, dir) && Blocked((u8)(lastX + StepX(dir)), (u8)(lastY + StepY(dir)), previous))
                return false;
        }
        lastX = x;
        lastY = y;
        x = (u8)(x + StepX(dir));
        y = (u8)(y + StepY(dir));
    }
    return true;
}

// The cell appended to the route (no bound on the count).
void Append(u8 *route, u16 cell) {
    at<u16>(route, 6 + route[0] * 2) = cell;
    ++route[0];
}

// The search's failure: the cell it got to appended (no bound), 0x70000000.
u32 Fail(u8 *route, u8 x, u8 y) {
    Append(route, (u16)(x | y << 8));
    return 0x70000000;
}

// Whether walker a's next step (in its direction) lands on walker b's cell, b heading the other way.
bool HeadOn(u8 *a, u8 *b) {
    const u32 d = at<u32>(a, 0x200);
    return b[0x208] == (u8)(a[0x208] + StepX(d)) && b[0x209] == (u8)(a[0x209] + StepY(d)) && ((d - 2) & 3) == at<u32>(b, 0x200);
}

// The walker's cells and then its own cell appended; false (nothing appended) when they would take the count past 0xfc.
bool TakeWalk(u8 *route, u8 *walker) {
    const u32 n = walker[0];
    if ((s32)(route[0] + n + 2) > 0xfc)
        return false;
    for (u32 i = 0; i < n; ++i)
        Append(route, at<u16>(walker, 6 + i * 2));
    Append(route, at<u16>(walker, 0x208));
    return true;
}

// A wall follower (wallfollow.h) set off from the cell towards the route's target.
void SetOff(u8 *walker, u8 *route, u16 cell, u32 dir, u32 turn) {
    walker[0] = 0;
    at<u16>(walker, 4) = at<u16>(route, 4);
    at<u32>(walker, 0x200) = dir;
    at<u32>(walker, 0x204) = turn;
    at<u16>(walker, 0x208) = cell;
    at<u16>(walker, 0x20a) = cell;
}

}  // namespace

extern "C" {

u32 __cdecl Route_Straighten(u8 *route, u32 first) {
    u32 result = 0, dropped;
    do {
        u32 kept = first, next = first;
        u16 from = first == 0 ? at<u16>(route, 2) : at<u16>(route, 4 + first * 2);
        dropped = 0;
        if (next < route[0]) {
            do {
                u32 ahead = 4;
                for (; ahead != 0; --ahead)
                    if (next + ahead < route[0] && Reaches(from, at<u16>(route, 6 + (next + ahead) * 2)))
                        break;
                if (ahead != 0) {
                    next += ahead;
                    dropped = result = 1;
                } else {
                    from = at<u16>(route, 6 + next * 2);
                    at<u16>(route, 6 + kept * 2) = from;
                    ++kept;
                    ++next;
                }
            } while (next < route[0]);
        }
        route[0] = (u8)kept;  // sic: first itself when it was not under the count
    } while (dropped);
    return result;
}

u32 __cdecl Route_Search(u8 *route, u32 from, u32 limit) {
    u32 &steps = global<u32>(0x007cf3a4);
    u8 x = (u8)from, y = (u8)(from >> 8);
    u32 dir = 0;
    bool first = true;
    LineStep_Init(from, at<u16>(route, 4));
    for (; steps < limit && (x != route[4] || y != route[5]); ++steps) {
        const u32 previous = dir;
        s32 &sum = global<s32>(0x007cf398);
        if (sum >= global<s32>(0x007cf3a8)) {
            sum -= global<s32>(0x007cf3a8);
            dir = global<u32>(0x007cf3b4);
        } else {
            sum += global<s32>(0x007cf3ac);
            dir = global<u32>(0x007cf3b0);
        }
        if (!Blocked(x, y, dir)) {
            const u8 lastX = x, lastY = y;
            x = (u8)(x + StepX(dir));
            y = (u8)(y + StepY(dir));
            if (first) {
                first = false;
                continue;
            }
            if (dir == previous)
                continue;
            bool corner;  // a turn: the cell kept when a step round the corner from it is open but the corner's other step blocked
            if (!Blocked(x, y, previous))
                corner = Blocked((u8)(x + StepX(previous)), (u8)(y + StepY(previous)), dir) != 0;
            else  // (sic: the two steps tested again, the first the one just taken)
                corner = !Blocked(lastX, lastY, dir) && Blocked(x, y, previous);
            if (corner) {
                Append(route, (u16)(x | y << 8));
                if (route[0] > 0xfa)
                    return Fail(route, x, y);
            }
            continue;
        }
        // Blocked: the cell kept and two walkers sent round the edge, one each way, till one is back on the line.
        if (global<u32>(0x007cf3c8) != 0) {
            global<u8>(0x007cf3a0) = route[0];
            global<u32>(0x007cf3b8) = steps;
            global<u8>(0x007cf39c) = (u8)dir;
        }
        const u16 cell = (u16)(x | y << 8);
        Append(route, cell);
        u8 left[0x20c], right[0x20c];
        SetOff(left, route, cell, (dir + 1) & 3, 0xffffffff);
        SetOff(right, route, cell, (dir - 1) & 3, 1);
        u32 &leftDir = at<u32>(left, 0x200), &rightDir = at<u32>(right, 0x200);
        if (steps < limit) {
            for (;;) {
                if (leftDir != 0x33) {
                    WallFollower_Step(left);
                    if (leftDir != 0x33 && leftDir != 0x32 && HeadOn(left, right))
                        return Fail(route, x, y);
                }
                WallFollower_Step(right);
                if (leftDir != 0x33 && leftDir != 0x32 && HeadOn(left, right))
                    return Fail(route, x, y);
                if (leftDir == 0x32) {
                    if (!TakeWalk(route, left))
                        return Fail(route, x, y);
                    if (global<u32>(0x007cf3c8) != 0)
                        global<u8>(0x007cf3bc) = 0xff;
                    break;
                }
                if (rightDir == 0x32) {
                    if (!TakeWalk(route, right))
                        return Fail(route, x, y);
                    if (global<u32>(0x007cf3c8) != 0)
                        global<u8>(0x007cf3bc) = 1;
                    break;
                }
                if (leftDir == 0x33 && rightDir == 0x33)
                    return Fail(route, x, y);
                if (++steps >= limit)
                    break;
            }
        }
        if (steps == limit)
            return Fail(route, x, y);
        Route_Straighten(route, 0);
        const u16 last = at<u16>(route, 4 + route[0] * 2);
        x = (u8)last;
        y = (u8)(last >> 8);
        LineStep_Init(last, at<u16>(route, 4));
        first = true;
    }
    if (route[0] < 0xfa && x == route[4] && y == route[5]) {
        Append(route, (u16)(x | y << 8));
        return steps;
    }
    return 0x70000000;
}

u32 __cdecl Route_SearchFrom(u8 *route, u32 mode) {
    global<u32>(0x007cf3c0) = mode;
    route[0] = 0;
    global<u32>(0x007cf3a4) = 0;
    if (global<u32>(0x007cf3c8) != 0)
        global<u8>(0x007cf3a0) = 0xff;
    return Route_Search(route, at<u16>(route, 2), 60000);
}

u32 __cdecl Route_Refine(u8 *route, u32 result) {
    Route_Straighten(route, 0);
    for (u32 rounds = 9;;) {  // each cell, from the first: the route round it searched again from the cell before to the cell after
        u32 i = 0;
        for (;; ++i) {
            if (i + 1 >= route[0])
                goto corners;
            u8 local[0x206];
            local[0] = 0;
            at<u16>(local, 2) = i == 0 ? at<u16>(route, 2) : at<u16>(route, 4 + i * 2);
            at<u16>(local, 4) = at<u16>(route, 8 + i * 2);
            global<u32>(0x007cf3a4) = 0;
            if (global<u32>(0x007cf3c8) != 0)
                global<u8>(0x007cf3a0) = 0xff;
            if (Route_Search(local, at<u16>(local, 2) | (u32)at<u16>(local, 4) << 16, 60000) == 0x70000000)
                continue;
            const u32 n = local[0];
            bool through = false;  // the new way passes the cell (its last cell, the one after, not looked at)
            for (u32 k = 1; k < n && !through; ++k)
                through = local[6 + (k - 1) * 2] == route[6 + i * 2] && local[7 + (k - 1) * 2] == route[7 + i * 2];
            if (through || (s32)(route[0] + n - 2) > 0xfc)
                continue;
            // the cell and the one after replaced by the new way's cells (it ends at the one after)
            memmove(route + 6 + (i + n) * 2, route + 10 + i * 2, (route[0] - i) * 2 - 4);
            memcpy(route + 6 + i * 2, local + 6, n * 2);
            route[0] = (u8)(route[0] + n - 2);
            Route_Straighten(route, i < 4 ? 0 : i - 4);
            if (--rounds == 0)
                return result;
            break;
        }
    }
corners:
    // Each diagonal step of one cell: the corner cell put in before it when one step round the corner is blocked and the other open (looked at
    // from the cell stepped to, then from the cell stepped from). (The original first tests that the distances over their greatest common divisor
    // are both odd, as they always are for these; and looks at the corner for those further apart too, keeping nothing.)
    for (u32 j = 0; (s32)j < (s32)route[0]; ++j) {
        const u16 from = j == 0 ? at<u16>(route, 2) : at<u16>(route, 4 + j * 2);
        const u8 fx = (u8)from, fy = (u8)(from >> 8);
        const s32 dx = (s32)route[6 + j * 2] - fx, dy = (s32)route[7 + j * 2] - fy;
        if ((dx != 1 && dx != -1) || (dy != 1 && dy != -1))
            continue;
        const u8 ax = (u8)(fx + dx), ay = (u8)(fy + dy);
        const u32 up = dy > 0 ? 0 : 2, back = dx > 0 ? 3 : 1;  // from the cell stepped to: towards the other's row, towards its column
        const u32 down = dy > 0 ? 2 : 0, across = dx > 0 ? 1 : 3;  // from the cell stepped from: towards the other's row, its column
        s8 ox, oy;
        if (Blocked(ax, ay, up) && !Blocked(ax, ay, back)) {
            ox = (s8)-dx;
            oy = 0;
        } else if (Blocked(ax, ay, back) && !Blocked(ax, ay, up)) {
            ox = 0;
            oy = (s8)-dy;
        } else if (Blocked(fx, fy, down) && !Blocked(fx, fy, across)) {
            ox = 0;
            oy = (s8)-dy;
        } else if (Blocked(fx, fy, across) && !Blocked(fx, fy, down)) {
            ox = (s8)-dx;
            oy = 0;
        } else {
            continue;
        }
        if (route[0] + 1 >= 0xfc)
            continue;
        memmove(route + 8 + j * 2, route + 6 + j * 2, (route[0] - j) * 2);
        route[6 + j * 2] = (u8)(route[6 + j * 2] + ox);
        route[7 + j * 2] = (u8)(route[7 + j * 2] + oy);
        ++route[0];
        ++j;
    }
    return result;
}

u32 __cdecl Route_Find(u8 *route, u32 mode) {
    global<u32>(0x007cf3c0) = mode;
    route[0] = 0;
    global<u32>(0x007cf3a4) = 0;
    if (global<u32>(0x007cf3c8) != 0)
        global<u8>(0x007cf3a0) = 0xff;
    return Route_Refine(route, Route_Search(route, at<u16>(route, 2), 60000));
}

s32 __cdecl Route_CellDistance(u32 from, u32 to, u32 mode) {
    u8 route[0x200];  // (only the start, the target and what the search writes are read)
    route[2] = (u8)((from - 1) & 0x7f);
    route[3] = (u8)((from - 1) >> 7);
    route[4] = (u8)((to - 1) & 0x7f);
    route[5] = (u8)((to - 1) >> 7);
    if (Route_SearchFrom(route, mode) == 0x70000000)
        return -1;
    s32 total = 0;
    for (s32 k = 0; k < (s32)route[0] - 1; ++k) {
        const s32 dy = (s32)route[7 + 2 * k] - (s32)route[9 + 2 * k];
        const s32 dx = (s32)route[6 + 2 * k] - (s32)route[8 + 2 * k];
        total += (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
    }
    return total;
}
}
