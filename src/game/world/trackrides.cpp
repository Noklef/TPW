#include "trackrides.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool loaded() { return global<u32>(0x00877b58) == 0x4a454647; }

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
