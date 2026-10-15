#include "camera.h"

#include <cmath>

#include "../math/x87.h"

namespace {

template <typename T = u32> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

}  // namespace

extern "C" {

void __cdecl Camera_InitUnusedOne() { global<float>(0x007909e8) = 1.0f; }

void __cdecl Camera_InitUnusedEpsilon() { global<float>(0x00790a94) = 1.0f / 8388608.0f; }

void __cdecl Camera_Init7909ec() { global<float>(0x007909ec) = 110.0f; }

void __cdecl Camera_InitYaw() { global<float>(0x00790a38) = 0.0f; }

void __cdecl Camera_Init7909e0() { global<float>(0x007909e0) = 0.0f; }

void __cdecl Camera_InitPoint() {
    global<float>(0x007908f0) = 475.0f;
    global<float>(0x007908f4) = 0.0f;
    global<float>(0x007908f8) = 175.0f;
}

u32 __cdecl Camera_IsFollowing(const void *thing) {
    const void *const followed = global<const void *>(0x00790a44);
    return (global<u8>(0x00790ab0) & 4) && global(0x00790988) && followed && followed == thing;
}

u32 __cdecl Camera_MatricesClose(const float *a, const float *b) {
    const auto diff = [a, b](u32 i) { return X87Reg(std::fabs((X87Reg(a[i]) - b[i]).v)); };
    // (|d1| + |d0|), then for each pair (j, k): (|dj| + stored) + |dk|.
    X87Reg sum = diff(1) + diff(0);
    const u32 pairs[5][2] = {{2, 4}, {5, 6}, {8, 9}, {10, 12}, {13, 14}};
    for (const auto &pair : pairs) sum = diff(pair[0]) + (float)sum + diff(pair[1]);
    return !(sum.v >= global<float>(0x0074c980));  // fcomp: NaN counts as less
}
}
