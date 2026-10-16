#include "dequeiter.h"

#include <cstdint>
#include <initializer_list>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__fastcall DequeIter_Set(u32 *it, void *, u32 cur, const u32 *node) {
    it[0] = node[0];
    it[3] = (u32)(uintptr_t)node;
    it[1] = node[0] + 0x1000;
    it[2] = cur;
    return it;
}

u32 *__fastcall DequeIter_Plus(const u32 *it, void *, u32 *out, s32 n) {
    u32 first = it[0], end = it[1], cur = it[2] + 4 * (u32)n, slot = it[3];
    s32 offset = (s32)(cur - first) >> 2;
    s32 blocks = offset >= 0 ? (s32)((u32)offset >> 10) : -(s32)((0x3ffu - (u32)offset) >> 10);
    if (blocks) {
        slot += 4 * (u32)blocks;
        first = at<u32>((void *)(uintptr_t)slot, 0);
        end = first + 0x1000;
        offset -= (s32)((u32)blocks << 10);
        cur = first + 4 * (u32)offset;
    }
    out[0] = first;
    out[1] = end;
    out[2] = cur;
    out[3] = slot;
    return out;
}

s32 __fastcall DequeIter_Diff(const u32 *it, void *, const u32 *other) {
    if (it[3] == other[3])
        return (s32)(it[2] - other[2]) >> 2;
    const s32 slots = ((s32)(it[3] - other[3]) >> 2) + 0x3fffff;
    return ((s32)(other[1] - other[2]) >> 2) + (s32)((u32)slots << 10) + ((s32)(it[2] - it[0]) >> 2);
}

u32 *__fastcall DequeIter_Prev(u32 *it) {
    if (it[2] == it[0]) {
        it[3] -= 4;
        it[0] = at<u32>((void *)(uintptr_t)it[3], 0);
        it[1] = it[0] + 0x1000;
        it[2] = it[1];
    }
    it[2] -= 4;
    return it;
}

u8 *__fastcall Class703c14_Ctor(u8 *object) {
    const u8 top = (u8)((u32)(uintptr_t)object >> 24);
    at<u32>(object, 0x10) = 1;
    for (u32 offset : {0x14u, 0x18u, 0x1cu, 0x28u, 0x2cu, 0x30u, 0x38u, 0x3cu, 0x40u})
        at<u32>(object, offset) = 0;
    at<u8>(object, 0x24) = top;
    at<u8>(object, 0x34) = top;
    at<u32>(object, 0) = 0x00703c14;
    return object;
}
}
