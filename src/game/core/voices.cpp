#include "voices.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Record(s32 index) { return global<u8 *>(0x0080cef8) + index * 0x34; }

// The generator at 0x80cee0 stepped (seed * 0x343fd + 0x269ec3); its high half, signed.
s32 NextRandom() {
    global<u32>(0x0080cee0) = global<u32>(0x0080cee0) * 0x343fd + 0x269ec3;
    return (s32)global<u32>(0x0080cee0) >> 16;
}

}  // namespace

extern "C" {

u32 __cdecl Voice_Start(s32 voice, s32 x, s32 y, s32 z) {
    if (global<u32>(0x00816d40) == 0 || voice < 0 || voice >= 0x14) return 0xffffffffu;
    if (global<u32>(0x0080ced8) != 0 && global<u8>(0x0080c6f4 + voice * 0x68) != 0) return 0;
    if (global<s16>(0x00816d22) < 0) return 0xffffffffu;
    const s16 oldActive = global<s16>(0x00816d1e);
    global<s16>(0x00816d1e) = global<s16>(0x00816d22);
    s32 index = global<s16>(0x00816d1e);
    const s16 nextFree = global<s16>(0x0081655c + index * 0x68);
    global<s16>(0x00816d22) = nextFree;
    if (nextFree >= 0) global<u16>(0x0081655e + nextFree * 0x68) = 0xffff;
    index = global<s16>(0x00816d1e);
    global<s16>(0x0081655c + index * 0x68) = oldActive;
    if (oldActive >= 0) global<s16>(0x0081655e + oldActive * 0x68) = global<s16>(0x00816d1e);
    index = global<s16>(0x00816d1e);
    u8 *const instance = reinterpret_cast<u8 *>(0x008164fc + index * 0x68);
    const s16 previous = at<s16>(instance, 0x62);
    const s16 next = at<s16>(instance, 0x60);
    const u8 *const source = reinterpret_cast<const u8 *>(0x0080c6b0 + voice * 0x68);
    for (u32 i = 0; i < 0x68; ++i) instance[i] = source[i];
    at<s16>(instance, 0x60) = next;
    at<s16>(instance, 0x62) = previous;
    at<u32>(instance, 0) = 1;
    at<s32>(instance, 0xc) = x >> 4;
    at<s32>(instance, 0x10) = y >> 4;
    at<s32>(instance, 0x14) = z >> 4;
    at<u16>(instance, 8) = global<u16>(0x0080ceea);
    global<u16>(0x0080ceea) = (u16)(global<u16>(0x0080ceea) + 1);
    at<u8>(instance, 0xa) = 0;
    return ((u32)at<u16>(instance, 8) << 16) | (u32)index;
}

s32 __cdecl FreeList_Take(s32 index) {
    const s16 head = global<s16>(0x00816d24);
    if (head < 0) return -1;
    s16 *const slot = reinterpret_cast<s16 *>(0x0080cf02 + index * 0x140);
    const s16 old = *slot;
    *slot = head;
    const s16 next = at<s16>(Record(global<s16>(0x00816d24)), 0x30);
    global<s16>(0x00816d24) = next;
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
    s16 *const slot = reinterpret_cast<s16 *>(0x0080cf02 + index * 0x140);
    if ((s32)*slot == record) *slot = next;
    if (previous >= 0) at<u16>(Record(previous), 0x30) = at<u16>(rec, 0x30);
    if (next >= 0) at<u16>(Record(next), 0x32) = at<u16>(rec, 0x32);
    at<u16>(rec, 0x24) = 0;
    at<u16>(rec, 0x30) = global<u16>(0x00816d24);
    at<u16>(rec, 0x32) = 0xffff;
    global<u16>(0x00816d24) = (u16)record;
}

u8 *__cdecl Voice_ScaleLevels(s32 index) {
    u8 *const voice = reinterpret_cast<u8 *>(0x0080cefc + index * 0x140);
    const s32 factor = global<s32>(0x0080cef0);
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
    u8 *const entry = reinterpret_cast<u8 *>(0x0080cefc + index * 0x140);
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
            global<s32>(0x0080ced0) = global<s32>(0x0080ced0) + 1;
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
            const s32 *const sines = global<const s32 *>(0x0080ceec);
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
    if (global<u32>(0x00816d40) == 0) return;
    const u32 bytes = global<u32>(0x0080cedc) * 0x34;
    u8 *const records = global<u8 *>(0x0080cef8);
    global<u32>(0x0080ced0) = 0;
    global<u32>(0x0080ced4) = 0;
    global<u32>(0x0080cee0) = 1;
    global<u32>(0x0080cee4) = 1;
    global<u16>(0x0080cee8) = 1;
    global<u16>(0x0080ceea) = 1;
    for (u32 i = 0; i < bytes; ++i) records[i] = 0;
    for (u32 i = 0; i < 0x9600; ++i) global<u8>(0x0080cefc + i) = 0;
    for (u32 i = 0; i < 0x820; ++i) global<u8>(0x008164fc + i) = 0;
    global<s16>(0x00816d1c) = -1;
    global<s16>(0x00816d20) = 0;
    global<s16>(0x0080cfce) = -1;
    global<s16>(0x0080cfcc) = 1;
    global<s16>(0x0081648c) = -1;
    global<s16>(0x0081648e) = 0x76;
    for (s32 i = 1; i < 0x77; ++i) {
        global<s16>(0x0080cfce + i * 0x140) = (s16)(i - 1);
        global<s16>(0x0080cfcc + i * 0x140) = (s16)(i + 1);
    }
    global<s16>(0x00816d1e) = -1;
    global<s16>(0x00816d22) = 0;
    global<s16>(0x0081655e) = -1;
    global<s16>(0x0081655c) = 1;
    global<s16>(0x00816d14) = -1;
    global<s16>(0x00816d16) = 0x12;
    for (s32 i = 1; i < 0x13; ++i) {
        global<s16>(0x0081655e + i * 0x68) = (s16)(i - 1);
        global<s16>(0x0081655c + i * 0x68) = (s16)(i + 1);
    }
    global<s16>(0x00816d24) = 0;
    at<s16>(global<u8 *>(0x0080cef8), 0x32) = -1;
    at<s16>(global<u8 *>(0x0080cef8), 0x30) = 1;
    const s32 count = global<s32>(0x0080cedc);
    at<s16>(Record(count - 1), 0x30) = -1;
    at<s16>(Record(global<s32>(0x0080cedc) - 1), 0x32) = (s16)(global<s32>(0x0080cedc) - 2);
    for (s32 i = 1; i < global<s32>(0x0080cedc) - 1; ++i) {
        at<s16>(Record(i), 0x32) = (s16)(i - 1);
        at<s16>(Record(i), 0x30) = (s16)(i + 1);
    }
    global<u32>(0x00816d3c) = 0;
}
}  // extern "C"
