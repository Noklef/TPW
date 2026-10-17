#include "route.h"

#include <cstdint>

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
}
