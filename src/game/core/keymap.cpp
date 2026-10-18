#include "keymap.h"

#include <cstdint>
#include <cstring>

#include "../files/file.h"
#include "crt.h"
#include "gameheap.h"
#include "textarg.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *map(u32 address) { return *reinterpret_cast<u8 **>(address); }

constexpr u32 kMaps[5] = {0x0078718c, 0x00787a60, 0x00787c14, 0x0078733c, 0x0078721c};

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 Fire(u8 *table, u32 a, u32 b, u32 handlerAt, u16 flag) {
    if (at<u16>(table, 6) == 0)
        return 0;
    s32 found = -1;
    const s32 count = at<s16>(table, 4);
    for (s32 i = 0; i < count && found < 0; ++i) {
        const u8 *entry = at<u8 *>(table, 0) + 0x14 * i;
        if (at<u16>(entry, 2) == (u16)a && at<u16>(entry, 4) == (u16)b)
            found = i;
    }
    if (found < 0)
        return 0;
    u8 *entry = at<u8 *>(table, 0) + 0x14 * found;
    u32 result = 0;
    if (void *handler = at<void *>(entry, handlerAt))
        result = reinterpret_cast<u32(__fastcall *)(void *)>(handler)(handler);
    at<u16>(at<u8 *>(table, 0) + 0x14 * found, 6) = flag;
    return result;
}

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

void __fastcall KeyMap_SetKeyFromText(u8 *keys, void *, u32 action, u32 text) {
    if ((s16)action < 0 || (s16)action >= at<s16>(keys, 4)) return;
    u32 slot = action;  // (the original's own argument slot: the formatter's two characters land over it)
    TextFormat().format(global<void *>(0x0078ba84), reinterpret_cast<u16 *>(&slot), 2, text);
    u8 *const entries = at<u8 *>(keys, 0);
    const s16 a = (s16)action;
    if (at<s16>(entries, a * 0x14) == a) {
        at<u16>(entries, a * 0x14 + 2) = (u16)slot;
        return;
    }
    const s32 count = at<s16>(keys, 4);
    s32 found = -1;
    for (s32 i = 0; i < count && (s16)found < 0; ++i)
        if (at<s16>(entries, i * 0x14) == a) found = i;
    if ((s16)found <= -1) return;
    at<u16>(entries, (s16)found * 0x14 + 2) = (u16)slot;
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

void __cdecl KeyRecord_Push(u32 index, u32 shift, u32 a, u32 b, u32 c) {
    const u32 record = 0x00f86918 + 68 * index;
    u32 second;
    if (shift) {
        global<u32>(record + 4) = a;
        second = b;
    } else {
        global<u32>(record + 4) = global<u32>(record + 0xc);
        second = global<u32>(record + 0x10);
    }
    global<u32>(record + 8) = second;
    global<u32>(record + 0xc) = a;
    global<u32>(record + 0x10) = b;
    global<u32>(record + 0) = c;
}

void __cdecl KeyMaps_EnableAllAndClearFlag() {
    KeyMaps_EnableAll();
    global<u8>(0x007c24d0) = 0;
}

void __cdecl KeyMaps_ResetAllAndSetFlag() {
    KeyMaps_ResetAll();
    global<u8>(0x007c24d0) = 1;
}

void __cdecl KeyMaps_FreeAll() {
    static const u32 kSlots[6] = {0x0078718c, 0x00787a60, 0x00787c14, 0x0078733c, 0x00787294, 0x0078721c};
    for (const u32 slot : kSlots)
        if (void *const block = global<void *>(slot)) GameHeap_Free(block);
}

u32 __cdecl Code_Remap46b460(u32 code) {
    static const u16 kLetters[0x1b] = {0x3b,  0x5e9, 0x5e0, 0x5d1, 0x5d2, 0x5e7, 0x5db, 0x5e2, 0x5d9, 0x5df, 0x5d7, 0x5dc, 0x5da, 0x5e6,
                                       0x5de, 0x5dd, 0x5e4, 0x2f,  0x5e8, 0x5d3, 0x5d0, 0x5d5, 0x5d4, 0x27,  0x5e1, 0x5d8, 0x5d6};
    u32 mapped = code & 0xffff;
    switch (mapped) {
    case 0x27: mapped = 0x2c; break;
    case 0x2c: mapped = 0x5ea; break;
    case 0x2e: mapped = 0x5e5; break;
    case 0x2f: mapped = 0x2e; break;
    case 0x3b: mapped = 0x5e3; break;
    default:
        if (mapped >= 0x60 && mapped <= 0x7a)
            mapped = kLetters[mapped - 0x60];
    }
    return (code & 0xffff0000u) | mapped;
}

u32 __fastcall Actions_FireDown(u8 *table, void *, u32 a, u32 b) { return Fire(table, a, b, 0x10, 1); }

u32 __fastcall Actions_FireUp(u8 *table, void *, u32 a, u32 b) { return Fire(table, a, b, 0xc, 0); }
}

namespace {
using KeyStateFn = s16(__stdcall *)(s32);
u32 Modifiers(u32 slot) {
    const KeyStateFn state = *reinterpret_cast<const KeyStateFn *>((uintptr_t)slot);
    u32 held = 0;
    if (state(0x10) < 0) held = 3;
    if (state(0x11) < 0) held |= 0xc;
    if (state(0x12) < 0) held |= 0x30;
    return held;
}
}  // namespace

extern "C" {
u32 __cdecl KeyMap_Modifiers() { return Modifiers(0x006fd324); }

u32 __cdecl KeyMap_AsyncModifiers() { return Modifiers(0x006fd320); }
}
