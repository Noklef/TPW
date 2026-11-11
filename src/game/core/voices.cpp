#include "voices.h"

#include <cstdint>
#include "../render/particles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Record(s32 index) { return global<u8 *>(((u32)(uintptr_t)&gParticleState + 0x28)) + index * 0x34; }

// The generator at 0x80cee0 stepped (seed * 0x343fd + 0x269ec3); its high half, signed.
s32 NextRandom() {
    global<u32>(((u32)(uintptr_t)&gParticleState + 0x10)) = global<u32>(((u32)(uintptr_t)&gParticleState + 0x10)) * 0x343fd + 0x269ec3;
    return (s32)global<u32>(((u32)(uintptr_t)&gParticleState + 0x10)) >> 16;
}

}  // namespace

extern "C" {

u32 __cdecl Voice_Start(s32 voice, s32 x, s32 y, s32 z) {
    if (gParticles_816d40 == 0 || voice < 0 || voice >= 0x14) return 0xffffffffu;
    if (gParticleState.x08 != 0 && global<u8>(((u32)(uintptr_t)&gParticleEffects + 0x8384) + voice * 0x68) != 0) return 0;
    if (gParticleState.x9e52 < 0) return 0xffffffffu;
    const s16 oldActive = gParticleState.x9e4e;
    gParticleState.x9e4e = gParticleState.x9e52;
    s32 index = gParticleState.x9e4e;
    const s16 nextFree = global<s16>(((u32)(uintptr_t)&gParticleState + 0x968c) + index * 0x68);
    gParticleState.x9e52 = nextFree;
    if (nextFree >= 0) global<u16>(((u32)(uintptr_t)&gParticleState + 0x968e) + nextFree * 0x68) = 0xffff;
    index = gParticleState.x9e4e;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x968c) + index * 0x68) = oldActive;
    if (oldActive >= 0) global<s16>(((u32)(uintptr_t)&gParticleState + 0x968e) + oldActive * 0x68) = gParticleState.x9e4e;
    index = gParticleState.x9e4e;
    u8 *const instance = reinterpret_cast<u8 *>(((u32)(uintptr_t)&gParticleState + 0x962c) + index * 0x68);
    const s16 previous = at<s16>(instance, 0x62);
    const s16 next = at<s16>(instance, 0x60);
    const u8 *const source = reinterpret_cast<const u8 *>(((u32)(uintptr_t)&gParticleEffects + 0x8340) + voice * 0x68);
    for (u32 i = 0; i < 0x68; ++i) instance[i] = source[i];
    at<s16>(instance, 0x60) = next;
    at<s16>(instance, 0x62) = previous;
    at<u32>(instance, 0) = 1;
    at<s32>(instance, 0xc) = x >> 4;
    at<s32>(instance, 0x10) = y >> 4;
    at<s32>(instance, 0x14) = z >> 4;
    at<u16>(instance, 8) = global<u16>(((u32)(uintptr_t)&gParticleState + 0x1a));
    global<u16>(((u32)(uintptr_t)&gParticleState + 0x1a)) = (u16)(global<u16>(((u32)(uintptr_t)&gParticleState + 0x1a)) + 1);
    at<u8>(instance, 0xa) = 0;
    return ((u32)at<u16>(instance, 8) << 16) | (u32)index;
}

s32 __cdecl FreeList_Take(s32 index) {
    const s16 head = global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e54));
    if (head < 0) return -1;
    s16 *const slot = reinterpret_cast<s16 *>(((u32)(uintptr_t)&gParticleState + 0x32) + index * 0x140);
    const s16 old = *slot;
    *slot = head;
    const s16 next = at<s16>(Record(global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e54))), 0x30);
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e54)) = next;
    if (next >= 0) at<u16>(Record(next), 0x32) = 0xffff;
    at<s16>(Record(*slot), 0x30) = old;
    if (old >= 0) at<s16>(Record(old), 0x32) = *slot;
    return *slot;
}

void __cdecl FreeList_Return(s32 record, s32 index) {
    if (record < 0) return;
    u8 *const rec = Record(record);
    const s16 previous = at<s16>(rec, 0x32);
    const s16 next = at<s16>(rec, 0x30);
    s16 *const slot = reinterpret_cast<s16 *>(((u32)(uintptr_t)&gParticleState + 0x32) + index * 0x140);
    if ((s32)*slot == record) *slot = next;
    if (previous >= 0) at<u16>(Record(previous), 0x30) = at<u16>(rec, 0x30);
    if (next >= 0) at<u16>(Record(next), 0x32) = at<u16>(rec, 0x32);
    at<u16>(rec, 0x24) = 0;
    at<u16>(rec, 0x30) = global<u16>(((u32)(uintptr_t)&gParticleState + 0x9e54));
    at<u16>(rec, 0x32) = 0xffff;
    global<u16>(((u32)(uintptr_t)&gParticleState + 0x9e54)) = (u16)record;
}

u8 *__cdecl Voice_ScaleLevels(s32 index) {
    u8 *const voice = reinterpret_cast<u8 *>(((u32)(uintptr_t)&gParticleState + 0x2c) + index * 0x140);
    const s32 factor = global<s32>(((u32)(uintptr_t)&gParticleState + 0x20));
    for (u32 offset = 0x68; offset < 0x6c; ++offset) {
        const s8 value = at<s8>(voice, offset);
        if (value == 0) continue;
        s32 scaled = (factor * (s32)value) >> 10;
        if (scaled > 0x7f) scaled = 0x7f;
        if (scaled == 0) scaled = 1;
        at<u8>(voice, offset) = (u8)scaled;
    }
    const s16 half = at<s16>(voice, 0x64);
    if (half > 0) {
        const s16 scaled = (s16)((factor * (s32)half) >> 10);
        at<s16>(voice, 0x64) = scaled == 0 ? (s16)1 : scaled;
    }
    const s32 whole = at<s32>(voice, 0x60);
    if (whole > 0) {
        const s32 scaled = (s32)((u32)factor * (u32)whole) >> 10;
        at<s32>(voice, 0x60) = scaled == 0 ? 1 : scaled;
    }
    return voice;
}

void __cdecl Emitter_Spawn(s32 index, u32 reset) {
    u8 *const entry = reinterpret_cast<u8 *>(((u32)(uintptr_t)&gParticleState + 0x2c) + index * 0x140);
    s32 spread = at<s32>(entry, 0xcc);
    if (reset) {
        at<u16>(entry, 0) = 1;
        at<s32>(entry, 0xcc) = 0;
        return;
    }
    spread += at<s16>(entry, 0x50);
    if (at<s16>(entry, 0) > 0) {
        const s32 scale = (spread >> 7) + 0x20;
        for (s32 i = 0;; ++i) {
            const s32 divisor = at<s32>(entry, 0xd4);
            s32 count = at<s8>(entry, 0x68);
            if (divisor != 0) {
                const s32 quarter = (s32)((u32)at<s32>(entry, 0x20) << 2) / divisor;  // sic: faults for a divisor of -1 at 0x80000000
                if (quarter == 0) count = at<s8>(entry, 0x6b);
                else if (quarter == 1) count = at<s8>(entry, 0x6a);
                else if (quarter == 2) count = at<s8>(entry, 0x69);
            }
            if (!(i < (s32)((u32)count * (u32)scale))) break;
            const s32 taken = FreeList_Take(index);
            if (taken < 0) continue;
            global<s32>((u32)(uintptr_t)&gParticleState) = global<s32>((u32)(uintptr_t)&gParticleState) + 1;
            at<u16>(entry, 8) = (u16)(at<u16>(entry, 8) + 1);
            u8 *const record = Record(taken);
            at<u16>(record, 0x26) = (u16)index;
            at<u16>(record, 0x24) = 2;
            at<u16>(record, 0x28) = at<u16>(entry, 0x74);
            at<s16>(record, 0x2a) = at<s8>(entry, 4);
            at<u32>(record, 0x18) = at<u32>(entry, 0x114);
            const s32 half = at<s32>(entry, 0x78) >> 1;
            s32 jitter = 0;
            if (half != 0) jitter = NextRandom() % half;
            const s32 life = at<s32>(entry, 0x78) + jitter;
            at<s32>(record, 0x20) = life;
            at<s32>(record, 0x1c) = life;
            if (life == 0) at<s32>(record, 0x1c) = 1;
            const s32 angle = (NextRandom() & 0xfff) >> 3;
            const s32 *const sines = global<const s32 *>(((u32)(uintptr_t)&gParticleState + 0x1c));
            const u32 cosine = (u32)(angle + 0x80) & 0x1ff, sine = (u32)angle & 0x1ff;
            at<s32>(record, 0) = ((s32)(((u32)sines[cosine] * (u32)spread) << 2) >> 10) + at<s32>(entry, 0x14);
            at<s32>(record, 4) = at<s32>(entry, 0x18);
            at<s32>(record, 8) = at<s32>(entry, 0x1c) - ((s32)(((u32)sines[sine] * (u32)spread) << 2) >> 10);
            at<s32>(record, 0xc) = (s32)(((u32)sines[cosine] * (u32)(s32)at<s16>(entry, 0x50)) << 2) >> 10;
            at<s32>(record, 0x10) = at<s32>(entry, 0x3c) + 0x14;
            at<s32>(record, 0x14) = -((s32)(((u32)sines[sine] * (u32)(s32)at<s16>(entry, 0x50)) << 2) >> 10);
            at<u32>(record, 0x18) = at<u32>(entry, 0x114);
        }
    }
    at<s32>(entry, 0xcc) = spread;
}

void __cdecl Emitters_Reset() {
    if (gParticles_816d40 == 0) return;
    const u32 bytes = global<u32>(((u32)(uintptr_t)&gParticleState + 0xc)) * 0x34;
    u8 *const records = global<u8 *>(((u32)(uintptr_t)&gParticleState + 0x28));
    global<u32>((u32)(uintptr_t)&gParticleState) = 0;
    global<u32>(((u32)(uintptr_t)&gParticleState + 0x4)) = 0;
    global<u32>(((u32)(uintptr_t)&gParticleState + 0x10)) = 1;
    gParticleState.x14 = 1;
    global<u16>(((u32)(uintptr_t)&gParticleState + 0x18)) = 1;
    global<u16>(((u32)(uintptr_t)&gParticleState + 0x1a)) = 1;
    for (u32 i = 0; i < bytes; ++i) records[i] = 0;
    for (u32 i = 0; i < 0x9600; ++i) global<u8>(((u32)(uintptr_t)&gParticleState + 0x2c) + i) = 0;
    for (u32 i = 0; i < 0x820; ++i) global<u8>(((u32)(uintptr_t)&gParticleState + 0x962c) + i) = 0;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e4c)) = -1;
    gParticleState.x9e50 = 0;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0xfe)) = -1;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0xfc)) = 1;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x95bc)) = -1;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x95be)) = 0x76;
    for (s32 i = 1; i < 0x77; ++i) {
        global<s16>(((u32)(uintptr_t)&gParticleState + 0xfe) + i * 0x140) = (s16)(i - 1);
        global<s16>(((u32)(uintptr_t)&gParticleState + 0xfc) + i * 0x140) = (s16)(i + 1);
    }
    gParticleState.x9e4e = -1;
    gParticleState.x9e52 = 0;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x968e)) = -1;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x968c)) = 1;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e44)) = -1;
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e46)) = 0x12;
    for (s32 i = 1; i < 0x13; ++i) {
        global<s16>(((u32)(uintptr_t)&gParticleState + 0x968e) + i * 0x68) = (s16)(i - 1);
        global<s16>(((u32)(uintptr_t)&gParticleState + 0x968c) + i * 0x68) = (s16)(i + 1);
    }
    global<s16>(((u32)(uintptr_t)&gParticleState + 0x9e54)) = 0;
    at<s16>(global<u8 *>(((u32)(uintptr_t)&gParticleState + 0x28)), 0x32) = -1;
    at<s16>(global<u8 *>(((u32)(uintptr_t)&gParticleState + 0x28)), 0x30) = 1;
    const s32 count = global<s32>(((u32)(uintptr_t)&gParticleState + 0xc));
    at<s16>(Record(count - 1), 0x30) = -1;
    at<s16>(Record(global<s32>(((u32)(uintptr_t)&gParticleState + 0xc)) - 1), 0x32) = (s16)(global<s32>(((u32)(uintptr_t)&gParticleState + 0xc)) - 2);
    for (s32 i = 1; i < global<s32>(((u32)(uintptr_t)&gParticleState + 0xc)) - 1; ++i) {
        at<s16>(Record(i), 0x32) = (s16)(i - 1);
        at<s16>(Record(i), 0x30) = (s16)(i + 1);
    }
    gParticles_816d3c = 0;
}
}  // extern "C"
