#include "tracksquare.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "mapcell.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *cellAt(u32 id) { return global<u8 *>(0x007cf83c) + 0x294 + (id & 0xffff) * 0x44; }

const TrackSquareEnv kGameEnv = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)0x00532fc0)};
const TrackSquareEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void TrackSquare_SetEnv(const TrackSquareEnv *env) { g_env = env ? env : &kGameEnv; }

u8 __cdecl TrackSquare_Check(s32 x, s32 y, u32, u32 skip, u32 strict, u32 direction) {
    if (!Map_InBounds(x, y) || !Map_InBounds(x, y + 1) || !Map_InBounds(x + 1, y) || !Map_InBounds(x + 1, y + 1)) return 1;
    const u32 id = ((u32)y << 7) + (u32)x;
    u8 *const cells[4] = {cellAt(id + 1), cellAt(id + 0x81), cellAt(id + 2), cellAt(id + 0x82)};
    const u32 kind = Global81ecac_IfNoFlag();
    if (kind != 1 && kind != 2) return 1;
    const u32 flags = (strict ? 0x200 : 0x100) | (kind == 2 ? 0x10 : 0xb);
    if (!strict && skip) return 1;
    for (u8 *const cell : cells) {
        if (!cell) continue;
        const u32 ok = g_env->test532fc0(cell, nullptr, flags, 5, Byte_ShiftLog2(direction, 0x10) & 0xff);
        if (strict && !ok) return 0;
    }
    return 1;
}
}
