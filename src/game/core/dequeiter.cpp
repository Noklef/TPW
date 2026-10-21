#include "dequeiter.h"

#include <cstdint>
#include <initializer_list>
#include "gameheap.h"
#include "methods.h"
#include "seqs.h"
#include "sync.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 &Dword(u32 address) { return *reinterpret_cast<u32 *>(static_cast<uintptr_t>(address)); }

// Both iterators cleared and the map freed (its pointer kept), as the deques' block release does when the count is 0.
void ClearIterators(u8 *deque) {
    for (u32 offset = 4; offset < 0x24; offset += 4)
        at<u32>(deque, offset) = 0;
    GameHeap_Free(at<void *>(deque, 0x24));
}

// a - b in dwords, as the deque code computes it: the positions' difference within one block, else across the blocks between.
u32 Distance(const DequeIt &a, const DequeIt &b) {
    if (a.node == b.node)
        return static_cast<u32>(static_cast<s32>(a.cur - b.cur) >> 2);
    return (static_cast<u32>(static_cast<s32>(a.node - b.node) >> 2) + 0x3fffff) * 1024 + static_cast<u32>(static_cast<s32>(b.end - b.cur) >> 2) +
           static_cast<u32>(static_cast<s32>(a.cur - a.first) >> 2);
}

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

void __fastcall Deque32_AllocMap(u8 *deque) {
    s32 bytes = static_cast<s32>(at<u32>(deque, 0x28) << 2);
    if (bytes < 0)
        bytes = 0;
    at<void *>(deque, 0x24) = GameHeap_Alloc(static_cast<u32>(bytes));
}

void __fastcall Deque32_FreeFront(u8 *deque) {
    const u32 node = at<u32>(deque, 0x10);
    at<u32>(deque, 0x10) = node + 4;
    GameHeap_Free(reinterpret_cast<void *>(static_cast<uintptr_t>(Dword(node))));
    if (at<u32>(deque, 0x2c) == 0) {
        ClearIterators(deque);
        return;
    }
    const u32 next = at<u32>(deque, 0x10);
    const u32 block = Dword(next);
    at<u32>(deque, 4) = block;
    at<u32>(deque, 8) = block + 0x1000;
    at<u32>(deque, 0xc) = block;
    at<u32>(deque, 0x10) = next;
}

void __fastcall Deque32_FreeBack(u8 *deque) {
    const u32 node = at<u32>(deque, 0x20);
    at<u32>(deque, 0x20) = node - 4;
    GameHeap_Free(reinterpret_cast<void *>(static_cast<uintptr_t>(Dword(node))));
    if (at<u32>(deque, 0x2c) == 0) {
        ClearIterators(deque);
        return;
    }
    const u32 previous = at<u32>(deque, 0x20);
    const u32 block = Dword(previous);
    at<u32>(deque, 0x14) = block;
    at<u32>(deque, 0x18) = block + 0x1000;
    at<u32>(deque, 0x1c) = block + 0x1000;
    at<u32>(deque, 0x20) = previous;
}

void __fastcall Deque32_PopFront(u8 *deque) {
    const u32 cur = at<u32>(deque, 0xc) + 4;
    at<u32>(deque, 0xc) = cur;
    const u32 count = at<u32>(deque, 0x2c) - 1;
    at<u32>(deque, 0x2c) = count;
    if (count == 0 || cur == at<u32>(deque, 8))
        Deque32_FreeFront(deque);
}

void __fastcall Deque32_GrowBack(u8 *deque) {
    const u32 block = static_cast<u32>(reinterpret_cast<uintptr_t>(GameHeap_Alloc(0x1000)));
    DequeIt &begin = at<DequeIt>(deque, 4);
    DequeIt &end = at<DequeIt>(deque, 0x14);
    if (at<u32>(deque, 0x2c) == 0) {
        at<u32>(deque, 0x28) = 2;
        const u32 map = static_cast<u32>(reinterpret_cast<uintptr_t>(GameHeap_Alloc(8)));
        at<u32>(deque, 0x24) = map;
        Dword(map + 4) = block;
        const u32 node = at<u32>(deque, 0x24) + 4;
        const u32 first = Dword(node);
        begin = DequeIt{first, first + 0x1000, block + 0x800, node};
        end = begin;
        return;
    }
    u32 slot = end.node;
    if (slot < at<u32>(deque, 0x24) + at<u32>(deque, 0x28) * 4 - 4) {
        slot += 4;
        end.node = slot;
        Dword(slot) = block;
    } else {
        const u32 used = static_cast<u32>((static_cast<s32>(slot - begin.node) >> 2) + 1);
        s32 bytes = static_cast<s32>(used * 8);
        if (bytes < 0)
            bytes = 0;
        const u32 map = static_cast<u32>(reinterpret_cast<uintptr_t>(GameHeap_Alloc(static_cast<u32>(bytes))));
        const u32 base = map + ((used * 2) >> 2) * 4;
        u32 to = base;
        for (u32 from = begin.node; from != end.node + 4; from += 4, to += 4)
            Dword(to) = Dword(from);
        GameHeap_Free(at<void *>(deque, 0x24));
        at<u32>(deque, 0x28) = used * 2;
        at<u32>(deque, 0x24) = map;
        slot = base + used * 4;
        Dword(slot) = block;
        const u32 first = Dword(base);
        begin = DequeIt{first, first + 0x1000, begin.cur, base};
    }
    const u32 first = Dword(slot);
    end = DequeIt{first, first + 0x1000, block, slot};
}

u32 *__cdecl Deque32_Copy(u32 *out, DequeIt first, DequeIt last, DequeIt dest) {
    while (first.cur != last.cur) {
        Dword(dest.cur) = Dword(first.cur);
        dest.cur += 4;
        if (dest.cur == dest.end) {
            dest.node += 4;
            dest.first = Dword(dest.node);
            dest.cur = dest.first;
            dest.end = dest.first + 0x1000;
        }
        first.cur += 4;
        if (first.cur == first.end) {
            first.node += 4;
            first.cur = Dword(first.node);
            first.end = first.cur + 0x1000;
        }
    }
    *reinterpret_cast<DequeIt *>(out) = dest;
    return out;
}

u32 *__cdecl Deque32_CopyBackward(u32 *out, DequeIt first, DequeIt last, DequeIt dest) {
    while (last.cur != first.cur) {
        if (last.cur == last.first) {
            last.node -= 4;
            last.first = Dword(last.node);
            last.cur = last.first + 0x1000;
        }
        last.cur -= 4;
        if (dest.cur == dest.first) {
            dest.node -= 4;
            dest.first = Dword(dest.node);
            dest.end = dest.first + 0x1000;
            dest.cur = dest.end;
        }
        dest.cur -= 4;
        Dword(dest.cur) = Dword(last.cur);
    }
    *reinterpret_cast<DequeIt *>(out) = dest;
    return out;
}

u32 *__fastcall Deque32_Erase(u8 *deque, void *, u32 *out, DequeIt first, DequeIt last) {
    DequeIt &begin = at<DequeIt>(deque, 4);
    DequeIt &end = at<DequeIt>(deque, 0x14);
    const u32 erased = Distance(last, first);
    const u32 before = Distance(first, begin);
    if (before < Distance(end, last)) {
        DequeIt from = first, to = last;
        const u32 stop = begin.cur;
        while (stop != from.cur) {
            DequeIter_Prev(&from.first);
            DequeIter_Prev(&to.first);
            Dword(to.cur) = Dword(from.cur);
        }
        for (u32 i = erased; i != 0; i--) {
            const u32 cur = at<u32>(deque, 0xc) + 4;
            const u32 count = at<u32>(deque, 0x2c) - 1;
            at<u32>(deque, 0xc) = cur;
            at<u32>(deque, 0x2c) = count;
            if (count == 0 || cur == at<u32>(deque, 8))
                Deque32_FreeFront(deque);
        }
    } else {
        DequeIt from = last;
        const u32 stop = end.cur;
        u32 to = first.cur, toEnd = first.end, toNode = first.node;
        while (from.cur != stop) {
            Dword(to) = Dword(from.cur);
            to += 4;
            if (to == toEnd) {
                to = Dword(toNode + 4);
                toNode += 4;
                toEnd = to + 0x1000;
            }
            DequeIter_Inc(reinterpret_cast<u8 *>(&from));
        }
        for (u32 i = erased; i != 0; i--) {
            const u32 cur = at<u32>(deque, 0x1c) - 4;
            const u32 count = at<u32>(deque, 0x2c) - 1;
            at<u32>(deque, 0x1c) = cur;
            at<u32>(deque, 0x2c) = count;
            if (count == 0 || cur == at<u32>(deque, 0x14))
                Deque32_FreeBack(deque);
        }
    }
    DequeIt result = begin;
    if (before != 0) {
        result.cur = begin.cur + before * 4;
        s32 offset = static_cast<s32>(result.cur - begin.first) >> 2;
        const s32 blocks = offset >= 0 ? static_cast<s32>(static_cast<u32>(offset) >> 10) : -static_cast<s32>((0x3ffu - static_cast<u32>(offset)) >> 10);
        if (blocks != 0) {
            result.node = begin.node + static_cast<u32>(blocks) * 4;
            result.first = Dword(result.node);
            offset -= blocks << 10;
            result.end = result.first + 0x1000;
            result.cur = result.first + static_cast<u32>(offset) * 4;
        }
    }
    *reinterpret_cast<DequeIt *>(out) = result;
    return out;
}
}
