#include "wallfollow.h"

#include <cstdint>

#include "mapcell.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The column and row steps of direction d (the table at 0x7622b0, read past its four entries as the original does).
u8 StepX(u32 d) { return global<u8>(0x007622b0 + d * 2); }
u8 StepY(u32 d) { return global<u8>(0x007622b1 + d * 2); }

// The walker's cell recorded; true when that makes more than 0xfb.
bool Record(u8 *walker) {
    at<u16>(walker, 6 + walker[0] * 2) = at<u16>(walker, 0x208);
    return ++walker[0] > 0xfb;
}

}  // namespace

extern "C" {

void __cdecl WallFollower_Step(u8 *walker) {
    u32 &dir = at<u32>(walker, 0x200);
    if (dir == 0x33)
        return;
    const u32 mode = global<u32>(0x007cf3c0);
    u8 &x = walker[0x208], &y = walker[0x209];
    if (MapCell_StepBlocked((mode & 0xffffff00) | x, y, dir, (s32)mode)) {  // sic: the mode's upper bytes ride on x
        dir = (dir - at<u32>(walker, 0x204)) & 3;
        if (Record(walker)) {
            dir = 0x33;
            return;
        }
    } else {
        x = (u8)(x + StepX(dir));
        y = (u8)(y + StepY(dir));
        const u32 turned = (dir + at<u32>(walker, 0x204)) & 3;
        if (!MapCell_StepBlocked(x, y, turned, (s32)mode)) {
            dir = turned;
            if (Record(walker)) {
                dir = 0x33;
                return;
            }
        }
    }
    const u8 tx = walker[4], ty = walker[5];
    switch ((dir + at<u32>(walker, 0x204)) & 3) {
    case 0: if (ty < y) return; break;
    case 1: if (tx > x) return; break;
    case 2: if (ty > y) return; break;
    default: if (tx < x) return; break;
    }
    switch (dir) {
    case 0: if (ty > y) return; break;
    case 1: if (tx < x) return; break;
    case 2: if (ty < y) return; break;
    case 3: if (tx > x) return; break;
    default: break;
    }
    const u8 lx = walker[0x20a], ly = walker[0x20b];
    if (lx <= tx ? (x < lx || x > tx) : (x > lx || x < tx))
        return;
    if (ly <= ty ? (y < ly || y > ty) : (y > ly || y < ty))
        return;
    if (lx == x && ly == y)
        return;
    dir = 0x32;
}
}
