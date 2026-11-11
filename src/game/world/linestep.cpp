#include "linestep.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl LineStep_Init(u32 from, u32 to) {
    const s32 dx = (s32)(to & 0xff) - (s32)(from & 0xff);
    const s32 dy = (s32)((to >> 8) & 0xff) - (s32)((from >> 8) & 0xff);
    const s32 absX = dx < 0 ? -dx : dx, absY = dy < 0 ? -dy : dy;
    gLineStep_XMajor_7cf3c4 = absX > absY ? 1 : 0;
    const u32 rowDirection = dy > 0 ? 2 : 0;
    u32 columnDirection = dx > 0 ? 1 : 3;
    u32 rowStored = rowDirection;
    if ((from & 0xff) == (to & 0xff))
        columnDirection = rowDirection;
    if (((from >> 8) & 0xff) == ((to >> 8) & 0xff))
        rowStored = columnDirection;
    gLineStep_ColumnDir_7cf3b0 = columnDirection;
    gLineStep_RowDir_7cf3b4 = rowStored;
    gLineStep_AbsX_7cf3a8 = absX;
    gLineStep_AbsY_7cf3ac = absY;
    gLineStep_Sum_7cf398 = (absX >> 1) + (absY >> 1);
}
}
