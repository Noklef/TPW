#include "keymap.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *map(u32 address) { return *reinterpret_cast<u8 **>(address); }

constexpr u32 kMaps[5] = {0x0078718c, 0x00787a60, 0x00787c14, 0x0078733c, 0x0078721c};

}  // namespace

extern "C" {

s32 __fastcall KeyMap_FindKey(const u8 *keys, void *, s16 key, s16 modifiers) {
    const s32 count = at<s16>(keys, 4);
    s32 found = -1;
    for (s32 i = 0; i < count && (s16)found < 0; ++i) {
        const u8 *e = at<const u8 *>(keys, 0) + i * 0x14;
        if (at<s16>(e, 2) == key && at<s16>(e, 4) == modifiers) found = i;
    }
    return found;
}

s32 __fastcall KeyMap_FindAction(const u8 *keys, void *, s16 action) {
    const s32 count = at<s16>(keys, 4);
    s32 found = -1;
    for (s32 i = 0; i < count && (s16)found < 0; ++i)
        if (at<s16>(at<const u8 *>(keys, 0) + i * 0x14, 0) == action) found = i;
    return found;
}

u16 __fastcall KeyMap_IsHeld(const u8 *keys, void *, s16 action) {
    const s16 count = at<s16>(keys, 4);
    if (action < 0 || action >= count) return 0;
    const u8 *entries = at<const u8 *>(keys, 0);
    if (at<s16>(entries, action * 0x14) == action) return at<u16>(entries, action * 0x14 + 6);
    s32 found = -1;
    for (s32 i = 0; i < count && (s16)found < 0; ++i)
        if (at<s16>(entries, i * 0x14) == action) found = i;
    if ((s16)found <= -1) return 0;
    return at<u16>(entries, (s16)found * 0x14 + 6);
}

void __fastcall KeyMap_Enable(u8 *keys) { at<s16>(keys, 6) = 1; }

void __fastcall KeyMap_Reset(u8 *keys) {
    at<s16>(keys, 6) = 0;
    for (s32 i = 0; i < at<s16>(keys, 4); ++i) at<s16>(at<u8 *>(keys, 0) + i * 0x14, 6) = 0;
}

void __cdecl KeyMaps_EnableAll() {
    for (u32 address : kMaps) at<s16>(map(address), 6) = 1;
}

void __cdecl KeyMaps_ResetAll() {
    for (u32 address : kMaps) KeyMap_Reset(map(address));
}
}
