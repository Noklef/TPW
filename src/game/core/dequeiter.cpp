#include "dequeiter.h"

#include <cstdint>
#include <initializer_list>
#include "gameheap.h"
#include "seqs.h"
#include "sync.h"

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

void __fastcall Deque8_Clear(u8 *deque) {
    while (at<u32>(deque, 0x2c)) {
        const u32 cursor = at<u32>(deque, 0xc) + 8;
        const u32 count = at<u32>(deque, 0x2c) - 1;
        at<u32>(deque, 0xc) = cursor;
        at<u32>(deque, 0x2c) = count;
        if (count != 0 && cursor != at<u32>(deque, 8))
            continue;
        // The block just finished is freed and the map moves on.
        u32 *const slot = at<u32 *>(deque, 0x10);
        at<u32 *>(deque, 0x10) = slot + 1;
        GameHeap_Free(reinterpret_cast<void *>(*slot));
        if (at<u32>(deque, 0x2c) == 0) {
            for (u32 offset = 0x4; offset <= 0x10; offset += 4)
                at<u32>(deque, offset) = 0;
            for (u32 offset = 0x14; offset <= 0x20; offset += 4)
                at<u32>(deque, offset) = 0;
            GameHeap_Free(at<void *>(deque, 0x24));
        } else {
            const u32 first = *at<u32 *>(deque, 0x10);
            at<u32>(deque, 0x4) = first;
            at<u32>(deque, 0x8) = first + 0x1000;
            at<u32>(deque, 0xc) = first;
        }
    }
}

u32 __cdecl CommandQueue_Post(u32 code, u32 param) {
    if (!param) param = *reinterpret_cast<const u32 *>(0x0078ba88);
    u8 *const q = reinterpret_cast<u8 *>(0x007b4ba8);  // the begin iterator at +4, the end one at +0x14, the map +0x24, its size +0x28, the count +0x2c
    CritSec_Enter(reinterpret_cast<u8 *>(0x007b4bd8));
    u32 cur;
    if (at<u32>(q, 0x2c) != 0 && at<u32>(q, 0x1c) != at<u32>(q, 0x18)) {
        cur = at<u32>(q, 0x1c);
    } else {
        const u32 block = (u32)(uintptr_t)GameHeap_Alloc(0x1000);
        cur = block;
        if (at<u32>(q, 0x2c) == 0) {
            at<u32>(q, 0x28) = 2;
            at<u32 *>(q, 0x24) = static_cast<u32 *>(GameHeap_Alloc(8));
            at<u32 *>(q, 0x24)[1] = block;
            u32 *const node = at<u32 *>(q, 0x24) + 1;
            const u32 first = *node;
            cur = block + 0x800;
            at<u32>(q, 0xc) = cur;
            at<u32>(q, 4) = first;
            at<u32 *>(q, 0x10) = node;
            at<u32>(q, 0x14) = first;
            at<u32>(q, 8) = first + 0x1000;
            at<u32>(q, 0x18) = first + 0x1000;
            at<u32 *>(q, 0x20) = node;
        } else if (at<u32>(q, 0x20) < at<u32>(q, 0x24) + at<u32>(q, 0x28) * 4 - 4) {
            u32 *const node = at<u32 *>(q, 0x20) + 1;
            at<u32 *>(q, 0x20) = node;
            *node = block;
            const u32 first = *at<u32 *>(q, 0x20);
            at<u32>(q, 0x14) = first;
            at<u32>(q, 0x18) = first + 0x1000;
        } else {  // the map grown to twice the blocks in use, they in its middle
            const s32 count = ((s32)(at<u32>(q, 0x20) - at<u32>(q, 0x10)) >> 2) + 1;
            const u32 size = (u32)count * 2;
            s32 bytes = (s32)(size * 4);
            if (bytes < 0) bytes = 0;
            u32 *const map = static_cast<u32 *>(GameHeap_Alloc((u32)bytes));
            u32 *const from = map + (size >> 2);
            Dwords_Copy(at<u32 *>(q, 0x10), at<u32 *>(q, 0x20) + 1, from);
            GameHeap_Free(at<void *>(q, 0x24));
            at<u32 *>(q, 0x24) = map;
            at<u32>(q, 0x28) = size;
            u32 *const node = from + count;
            *node = block;
            u32 it[4];
            DequeIter_Set(it, nullptr, at<u32>(q, 0xc), from);
            for (u32 i = 0; i < 4; ++i) at<u32>(q, 4 + 4 * i) = it[i];
            DequeIter_Set(it, nullptr, block, node);
            at<u32>(q, 0x14) = it[0];
            at<u32>(q, 0x18) = it[1];
            cur = it[2];
            at<u32>(q, 0x1c) = cur;
            at<u32>(q, 0x20) = it[3];
        }
    }
    at<u32>(q, 0x1c) = cur + 8;
    if (cur) {
        at<u32>((void *)(uintptr_t)cur, 0) = param;
        at<u32>((void *)(uintptr_t)cur, 4) = code;
    }
    ++at<u32>(q, 0x2c);
    return CritSec_Leave(reinterpret_cast<u8 *>(0x007b4bd8));
}
}
