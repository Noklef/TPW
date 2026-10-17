#include "ime.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The difference of two pointers in dwords, shifted arithmetically as the compiled code does.
s32 Dwords(const u32 *a, const u32 *b) { return (s32)((uintptr_t)a - (uintptr_t)b) >> 2; }

}  // namespace

extern "C" {

u8 *__fastcall Ime_Ctor(u8 *ime) {
    // The original's `push ecx` makes room for a local allocator it never sets, then copies its
    // byte (the pushed this's top byte) into each vector (sic).
    const u8 junk = (u8)((uintptr_t)ime >> 24);
    for (u32 v = 8; v < 0x48; v += 0x10) {
        at<u8>(ime, v) = junk;
        at<u32>(ime, v + 4) = 0;
        at<u32>(ime, v + 8) = 0;
        at<u32>(ime, v + 0xc) = 0;
    }
    at<u8>(ime, 0x48) = 0;
    at<u32>(ime, 0x4c) = 0;
    at<u32>(ime, 0x50) = 0;
    at<u32>(ime, 0x54) = 0xffffffff;
    at<u32>(ime, 0x5c) = 0;
    at<u32>(ime, 0) = 0x006fef04;
    return ime;
}

u32 *__fastcall ImeVector_Copy(void *, void *, const u32 *first, const u32 *last, u32 *dest) {
    for (; first != last; ++first, ++dest)
        if (dest) *dest = *first;
    return dest;
}

void __fastcall ImeVector_Fill(void *, void *, u32 *dest, u32 n, const u32 *value) {
    for (; n != 0; --n, ++dest)
        if (dest) *dest = *value;
}

u32 __fastcall ImeVector_Size(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 8) - first) >> 2) : 0;
}

u32 __fastcall ImeVector_Capacity(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 0xc) - first) >> 2) : 0;
}

u32 __fastcall ImeVector16_Size(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 8) - first) >> 4) : 0;
}

u32 __fastcall ImeVector16_Capacity(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 0xc) - first) >> 4) : 0;
}

void __cdecl ImeVector_Construct(u32 *slot, const u32 *value) {
    if (slot) *slot = *value;
}

u8 *__fastcall Ime_Current(const u8 *ime) {
    const s32 index = at<s32>(ime, 0x54);
    return index == -1 ? nullptr : at<u8 *>(ime, 0x3c) + index * 0x24;
}

void __fastcall Ime_SetLimit(u8 *ime, void *, u32 limit) {
    const u32 most = *reinterpret_cast<const u32 *>(0x006feee0);
    at<u32>(ime, 0x58) = limit < most ? limit : most;
}

u8 *__fastcall ImeHost_Ctor(u8 *host) {
    Ime_Ctor(host + 0x1c);
    at<u32>(host, 0x17c) = 0;
    // Another unset local (the push ecx's slot): the pushed this's top byte (sic).
    at<u8>(host, 0x18c) = (u8)((uintptr_t)host >> 24);
    at<u32>(host, 0x190) = 0;
    at<u32>(host, 0x194) = 0;
    at<u32>(host, 0x198) = 0;
    at<u32>(host, 8) = 0;
    at<u32>(host, 0) = 0;
    at<u8>(host, 0xc) = 0;
    at<u32>(host, 0x10) = 0xffffffff;
    return host;
}

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
}
