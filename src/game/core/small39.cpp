#include "small39.h"

#include <cstdint>

#include "gameheap.h"
#include "ime.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The difference of two pointers in dwords, shifted arithmetically as the compiled code does.
s32 Dwords(const u32 *a, const u32 *b) { return (s32)((uintptr_t)a - (uintptr_t)b) >> 2; }

}  // namespace

extern "C" {

u32 *__fastcall ImeVector_InsertOne(u8 *vec, void *, u32 *position, const u32 *value) {
    u32 *const first = at<u32 *>(vec, 4);
    u32 *const last = at<u32 *>(vec, 8);
    const s32 index = Dwords(position, first);
    if ((u32)Dwords(at<u32 *>(vec, 0xc), last) == 0) {
        const s32 size = first == nullptr ? 0 : Dwords(last, first);
        const s32 grow = (u32)size <= 1 ? 1 : size;
        const s32 capacity = size + grow;
        u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)(capacity < 0 ? 0 : capacity) << 2));
        u32 *to = block;
        for (u32 *from = first; from != position; ++from, ++to)
            ImeVector_Construct(to, from);
        ImeVector_Construct(to, value);
        ImeVector_Copy(vec, nullptr, position, at<u32 *>(vec, 8), to + 1);
        GameHeap_Free(at<u32 *>(vec, 4));
        at<u32 *>(vec, 0xc) = block + capacity;
        const u32 old = ImeVector_Size(vec);
        at<u32 *>(vec, 8) = block + old + 1;
        at<u32 *>(vec, 4) = block;
        return block + index;
    }
    if ((u32)Dwords(last, position) < 1) {
        ImeVector_Copy(vec, nullptr, position, last, position + 1);
        ImeVector_Fill(vec, nullptr, at<u32 *>(vec, 8), (u32)(1 - Dwords(at<u32 *>(vec, 8), position)), value);
        u32 *const end = at<u32 *>(vec, 8);
        if (position != end) {
            u32 *p = position;
            do {
                *p = *value;
                ++p;
            } while (p != end);
        }
    } else {
        ImeVector_Copy(vec, nullptr, last - 1, last, last);
        u32 *to = at<u32 *>(vec, 8);
        u32 *from = to - 1;
        if (position != from) {
            do {
                const u32 v = from[-1];
                --from;
                --to;
                *to = v;
            } while (from != position);
        }
        u32 *const stop = position + 1;
        u32 *p = position;
        if (position != stop) {
            do {
                *p = *value;
                ++p;
            } while (p != stop);
        }
    }
    at<u32 *>(vec, 8) = at<u32 *>(vec, 8) + 1;
    return at<u32 *>(vec, 4) + index;
}

void __fastcall ImeVector_InsertN(u8 *vec, void *, u32 *position, u32 count, const u32 *value) {
    u32 *const last = at<u32 *>(vec, 8);
    if ((u32)Dwords(at<u32 *>(vec, 0xc), last) >= count) {
        if ((u32)Dwords(last, position) >= count) {
            if (count == 0) return;
            u32 *to = last;
            for (u32 *from = last - count; from != last; ++from, ++to)
                if (to) *to = *from;
            u32 *e = at<u32 *>(vec, 8) - count;
            to = at<u32 *>(vec, 8);
            if (position != e) {
                do {
                    const u32 v = e[-1];
                    --e;
                    --to;
                    *to = v;
                } while (e != position);
            }
            u32 *const stop = position + count;
            u32 *p = position;
            if (position != stop) {
                do {
                    *p = *value;
                    ++p;
                } while (p != stop);
            }
        } else {
            u32 *to = position + count;
            if (position != last) {
                u32 *from = to - count;
                do {
                    if (to) *to = *from;
                    ++from;
                    ++to;
                } while (from != last);
            }
            u32 *fill = at<u32 *>(vec, 8);
            for (s32 n = (s32)count - Dwords(fill, position); n != 0; --n, ++fill)
                if (fill) *fill = *value;
            u32 *const end = at<u32 *>(vec, 8);
            u32 *p = position;
            if (position != end) {
                do {
                    *p = *value;
                    ++p;
                } while (p != end);
            }
        }
        at<u32 *>(vec, 8) = at<u32 *>(vec, 8) + count;
        return;
    }
    u32 *const first = at<u32 *>(vec, 4);
    const s32 size = first == nullptr ? 0 : Dwords(last, first);
    const s32 grow = count >= (u32)size ? (s32)count : size;
    s32 capacity = grow + size;
    u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)(capacity < 0 ? 0 : capacity) << 2));
    u32 *to = block;
    for (u32 *from = at<u32 *>(vec, 4); from != position; ++from, ++to)
        if (to) *to = *from;
    u32 *fill = to;
    for (u32 n = count; n != 0; --n, ++fill)
        if (fill) *fill = *value;
    u32 *const end = at<u32 *>(vec, 8);
    u32 *dst = to + count;
    if (position != end) {
        for (u32 *from = position; from != end; ++from, ++dst)
            if (dst) *dst = *from;
    }
    GameHeap_Free(at<u32 *>(vec, 4));
    at<u32 *>(vec, 0xc) = block + capacity;
    u32 *const old = at<u32 *>(vec, 4);
    at<u32 *>(vec, 4) = block;
    at<u32 *>(vec, 8) = block + count + (old == nullptr ? 0 : Dwords(at<u32 *>(vec, 8), old));
}

}  // extern "C"
