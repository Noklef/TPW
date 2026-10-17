#include "route.h"

#include <cstdint>

#include "linestep.h"
#include "mapcell.h"

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
}
