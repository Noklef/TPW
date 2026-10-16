#include "small24.h"

#include <cstdint>

#include "../world/object.h"
#include "accessors.h"
#include "tiny_a.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl Format_FillInfo(u32 a, u32 *out, u32 c, u32 d) {
    if (!out)
        return;
    const u32 *record = reinterpret_cast<const u32 *>((uintptr_t)(0x008bd558 + global<u32>(0x008bd554) * 32));
    u32 local[8];
    for (u32 i = 0; i < 8; ++i)
        local[i] = record[i];
    out[6] = a;
    out[7] = c;
    out[4] = local[3];
    out[8] = d;
    out[0] = local[4];
    out[9] = (local[3] >> 3) * c;
    out[1] = local[5];
    out[2] = local[6];
    out[3] = local[7];
    out[5] = local[1] & 0x20;
}

void __fastcall List_SpliceOne(u8 *list, void *, u8 *a, u8 *x, u8 *c) {
    if (a == c)
        return;
    u8 *n = at<u8 *>(c, 0);
    if (a == n)
        return;
    at<u8 *>(at<u8 *>(n, 4), 0) = a;
    at<u8 *>(at<u8 *>(c, 4), 0) = n;
    at<u8 *>(at<u8 *>(a, 4), 0) = c;
    u8 *nPrev = at<u8 *>(n, 4);
    u8 *aPrev = at<u8 *>(a, 4);
    at<u8 *>(a, 4) = nPrev;
    at<u8 *>(n, 4) = at<u8 *>(c, 4);
    at<u8 *>(c, 4) = aPrev;
    at<u32>(list, 8) += 1;
    at<u32>(x, 8) -= 1;
}

void __fastcall Obj_ApplyToGlobalObject(u8 *object) {
    u8 *game = global<u8 *>(0x007c2658);
    const u32 returned = Set198_4e21e0(game, nullptr, at<u32>(object, 0x28));
    const u32 product = (u32)(u16)(s16)((s32)at<s8>(object, 0x24) * 0x32);
    Object_SetQualityOfGoods(game, nullptr, (returned & 0xffff0000u) | product);
    Obj_Set194(game, nullptr, at<u32>(object, 0x2c));
}

}  // extern "C"
