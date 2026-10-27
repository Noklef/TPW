#include "demolish.h"

#include <cstdint>

#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *cellAt(u32 id) { return global<u8 *>(0x007cf83c) + 0x294 + (id & 0xffff) * 0x44; }
bool lineType(s32 type) { return type == 4 || type == 9 || type == 10; }

const DemolishEnv kGameEnv = {reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)0x00527ee0),
                              reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)0x005367a0)};
const DemolishEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void Demolish_SetEnv(const DemolishEnv *env) { g_env = env ? env : &kGameEnv; }

u8 __cdecl Map_DemolishRect(s32 x0, s32 y0, s32 x1, s32 y1) {
    const s32 minX = x0 < x1 ? x0 : x1;
    const s32 minY = y0 < y1 ? y0 : y1;
    const s32 maxX = x0 > x1 ? x0 : x1;
    const s32 maxY = y0 > y1 ? y0 : y1;
    for (s32 y = minY; y <= maxY; ++y) {
        const u32 row = ((u32)y << 7) + 1;
        for (s32 x = minX; x <= maxX; ++x) {
            const u8 *const cell = cellAt(row + (u32)x);
            const s32 type = at<s32>(cell, 8);
            if (!(type == 4 || (type > 8 && type <= 10))) continue;
            const u16 owner = at<u16>(cell, 0x10);
            bool ok = true;
            const u8 *c;
            s32 i = x;
            do {   // (right)
                c = cellAt(row + (u32)i);
                if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                if (i > (s32)((u32)maxX + 1)) ok = false;
                ++i;
            } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(i, y));
            if (ok) {
                i = x;
                do {   // (left)
                    c = cellAt(row + (u32)i);
                    if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                    if (i < (s32)((u32)minX - 1)) ok = false;
                    --i;
                } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(i, y));
            }
            if (ok) {
                i = y;
                do {   // (down)
                    c = cellAt(((u32)i << 7) + (u32)x + 1);
                    if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                    if (i > (s32)((u32)maxY + 1)) ok = false;
                    ++i;
                } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(x, i));
            }
            if (ok) {
                i = y;
                do {   // (up)
                    c = cellAt(((u32)i << 7) + (u32)x + 1);
                    if (MapCell_HasFlags(c, nullptr, 0x10)) ok = false;
                    if (i < (s32)((u32)minY - 1)) ok = false;
                    --i;
                } while (lineType(at<s32>(c, 8)) && at<u16>(c, 0x10) == owner && Map_InBounds(x, i));
                if (ok) g_env->demolish(x, y, 0);
            }
        }
    }
    for (s32 y = minY; y <= maxY; ++y) {
        for (s32 x = minX; x <= maxX; ++x) {
            u8 *const c = cellAt(((u32)y << 7) + 1 + (u32)x);
            if (MapCell_IsType1(c)) g_env->cell5367a0(c, nullptr, 0, 0);
        }
    }
    return 1;
}
}
