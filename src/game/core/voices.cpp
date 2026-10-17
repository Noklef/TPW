#include "voices.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Record(s32 index) { return global<u8 *>(0x0080cef8) + index * 0x34; }

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
}  // extern "C"
