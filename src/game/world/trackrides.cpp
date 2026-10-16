#include "trackrides.h"

#include <cstdint>

#include "../motion/animator.h"
#include "../render/particles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool loaded() { return global<u32>(0x00877b58) == 0x4a454647; }

// The ride a handle indexes, or 0; whether the handle also names it (its index and serial).
u8 *RideOf(u32 handle) {
    return loaded() ? reinterpret_cast<u8 *>(global<uintptr_t>(0x00877b60) + (handle & 0xff) * 0xd0) : nullptr;
}

u32 NameOf(const u8 *ride) {
    if (!loaded())
        return 0;
    return (u32)((s32)((uintptr_t)ride - global<uintptr_t>(0x00877b60)) / 0xd0) | at<u32>(ride, 0) << 8;
}

}  // namespace

extern "C" {

s32 __cdecl TrakChunk_Write(u8 *chunk, const void *buffer, s32 count) {
    s32 result, done;
    if (*Stream_Write(&result, at<Stream *>(chunk, 4), buffer, count, &done) < 0) return 0;
    at<s32>(chunk, 0x10) += count;
    return done;
}

s32 *__cdecl TrakStream_Read(s32 *result, Stream *stream, void *buffer, s32 count, s32 *done) {
    Stream_Read(result, stream, buffer, count, done);
    return result;
}

u8 *__cdecl TrakPool_FirstFree() {
    u8 *record = global<u8 *>(0x00877b68);
    if (!record) return nullptr;
    for (u32 i = 0; i < 0x100; ++i, record += 0xac) {
        if (!(record[0] & 1)) return record;
    }
    return nullptr;
}

u32 __cdecl TrakRide_Field5c(u32 handle) {
    const u8 *ride = RideOf(handle);
    return handle == NameOf(ride) ? at<u32>(ride, 0x5c) : 0;
}

u32 __cdecl TrakRide_PopNode(u32 handle) {
    u8 *ride = RideOf(handle);
    if (!loaded() || handle != NameOf(ride))
        return 0;
    u8 *node = at<u8 *>(ride, 0xc8);
    if (!node)
        return 0;
    at<u32>(ride, 0xc8) = at<u32>(node, 0x10);
    at<u32>(node, 0x10) = global<u32>(0x00877b8c);
    global<u32>(0x00877b8c) = (u32)(uintptr_t)node;
    return at<u32>(node, 0);
}

u32 __cdecl TrakChunk_Read(u8 *chunk, u8 *buffer, u32 count) {
    u32 n = at<u32>(chunk, 0x10) - at<u32>(chunk, 8);
    if (count < n)
        n = count;
    s32 result, done;
    Stream_Read(&result, at<Stream *>(chunk, 4), buffer, n, &done);
    if (result < 0)
        return 0;  // (the debug output is gone)
    at<u32>(chunk, 8) += done;
    for (u32 i = done; i < count && (u32)done < count; ++i)
        buffer[i] = 0;
    return done;
}

u32 __cdecl TrakChunk_ReadDword(u8 *chunk) {
    u32 value = (u32)(uintptr_t)chunk;  // (the slot the read goes to starts as the chunk pointer)
    u32 n = at<u32>(chunk, 0x10) - at<u32>(chunk, 8);
    if (4 < n)
        n = 4;
    s32 result, done;
    Stream_Read(&result, at<Stream *>(chunk, 4), &value, n, &done);
    if (result < 0)
        return 0;
    at<u32>(chunk, 8) += done;
    return done == 4 ? value : 0;
}

void __cdecl TrakRide_Fixed(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    const u32 was = at<u32>(ride, 0x54);
    at<u32>(ride, 0x54) = 0;
    if (was != 2)
        return;
    const s32 kind = at<s32>(ride, 0);
    if (!(kind > -15 && (kind < -2 || kind == -1)))
        return;
    u8 *record = global<u8 *>(0x00877b68);
    u32 left = global<u32>(0x00877b6c);
    while (left != 0) {
        if ((at<u8>(record, 0) & 1) && (--left, at<u8 *>(record, 0x9c) == ride) && at<u32>(record, 0x2c) != 0xffffffffu) {
            ParticleSystem_End(at<u32>(record, 0x2c));
            at<u32>(record, 0x2c) = 0xffffffffu;
            if (at<s32>(at<u8 *>(record, 0x9c), 0) == -1 && (at<u32>(record, 0) & 0x4000)) {
                at<u32>(record, 0x10) = 5;
                if (at<u32>(record, 8) != 0)
                    Layer_ClearQueue(reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(record, 8)], 0);
                if (at<u32>(record, 8) != 0)
                    Animator_Play(reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(record, 8)], 5, 0, 1, 1.0f, 0);
            }
        }
        record += 0xac;
    }
}

void __cdecl TrakRide_SetPerformance(u32 handle, s32 percent) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    if (percent < 0)
        percent = 0;
    else if (percent > 99)
        percent = 100;
    at<s32>(ride, 0x1c) = percent;
    for (u32 i = 0; i < 4; ++i) {
        const s32 from = at<s32>(ride, 0x30 + 4 * i), to = at<s32>(ride, 0x40 + 4 * i);
        at<s32>(ride, 0x20 + 4 * i) = (to - from) * percent / 100 + from;
    }
}

s32 __cdecl TrakRide_CountPieces(u32 handle, s32 *others, s32 *longestRun, s32 *kind11, s32 *free) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return 0;
    s32 total = 0, run = 0, kind11Count = 0, longest = 0, freeCount = 0, otherCount = 0;
    for (s32 pass = 2; pass != 0; --pass) {
        for (u8 *piece = at<u8 *>(ride, 0xbc); piece; piece = at<u8 *>(piece, 0x24)) {
            switch (at<u32>(piece, 4) & 0xffff) {
            case 11:
                ++run;
                ++kind11Count;
                break;
            case 12:
            case 13:
                if (at<s32>(piece, 8) == 0)
                    ++freeCount;
                ++run;
                break;
            case 9:
            case 10:
                ++run;
                break;
            default:
                if (longest < run)
                    longest = run;
                run = 0;
                ++otherCount;
            }
            ++total;
        }
    }
    if (others)
        *others = otherCount / 2;
    if (longestRun)
        *longestRun = longest;
    if (kind11)
        *kind11 = kind11Count / 2;
    if (free)
        *free = freeCount;
    return total / 2;
}

u32 __cdecl TrakRide_AddNextCar(u32 handle, u32 value) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride) || at<u32>(ride, 0x50) == 0)
        return 0;
    u8 *node = global<u8 *>(0x00877b8c);
    if (!node)
        return 0;
    global<u32>(0x00877b8c) = at<u32>(node, 0x10);
    at<u32>(node, 0x10) = at<u32>(ride, 0xc4);
    at<u8 *>(ride, 0xc4) = node;
    at<u32>(node, 0) = value;
    at<u32>(node, 4) = handle;
    at<u32>(node, 8) = 0xffffffffu;
    return 1;
}

void __cdecl TrakRide_Close(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride) || at<u32>(ride, 0x50) == 0)
        return;
    at<u32>(ride, 0x50) = 0;
    u8 *node = at<u8 *>(ride, 0xc8);
    if (!node) {
        at<u32>(ride, 0xc8) = at<u32>(ride, 0xc4);
    } else {
        while (u8 *next = at<u8 *>(node, 0x10))
            node = next;
        at<u32>(node, 0x10) = at<u32>(ride, 0xc4);
    }
    at<u32>(ride, 0xc4) = 0;
    const s32 kind = at<s32>(ride, 0);
    if (kind != -14 && kind != -11 && kind != -6 && kind != -3 && kind != -1)
        return;
    u8 *record = global<u8 *>(0x00877b68);
    u32 left = global<u32>(0x00877b6c);
    if (!record || !left)
        return;
    do {
        if (at<u8>(record, 0) & 1) {
            --left;
            if (at<u8 *>(record, 0x9c) == ride) {
                at<u32>(record, 0x88) = 0;
                at<u32>(record, 0) = (at<u32>(record, 0) & 0xffffbfa1u) | 0x10020u;
            }
        }
        record += 0xac;
    } while (left != 0);
}

void __cdecl TrakRide_MarkWornOut(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle == NameOf(ride))
        at<u32>(ride, 0x54) = 1;
}

void __cdecl TrakRide_SetField4(u32 handle, u32 value) {
    u8 *ride = RideOf(handle);
    if (handle == NameOf(ride))
        at<u32>(ride, 4) = value;
}

void __cdecl TrakRide_PushNode(u32 handle, u32 value) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    u8 *node = global<u8 *>(0x00877b8c);
    if (!node)
        return;
    global<u32>(0x00877b8c) = at<u32>(node, 0x10);
    at<u32>(node, 0x10) = at<u32>(ride, 0xc8);
    at<u8 *>(ride, 0xc8) = node;
    at<u32>(node, 4) = handle;
    at<u32>(node, 0) = value;
}

u32 __cdecl TrakRide_LinkedValue(u32 handle) {
    const uintptr_t rides = global<uintptr_t>(0x00877b60);
    const uintptr_t ride = loaded() ? rides + (handle & 0xff) * 0xd0 : 0;
    // The handle of that ride: its index (from the pointer, as the original divides it back
    // out) and serial.
    const u32 named = loaded() ? (u32)((s32)(ride - rides) / 0xd0) | at<u32>((const void *)ride, 0) << 8 : 0;
    if (handle != named) return 0;
    const u8 *link = at<const u8 *>((const void *)ride, 0x58);
    if (!link || !(at<u32>(link, 0) & 0x400000)) return 0;
    return at<u32>(link, 8);
}
}
