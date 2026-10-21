#include "cursor.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall CursorTable_Get(const u8 *table, void *, u32 index) {
    const u32 i = index & 0xffff;
    if (i < at<u16>(table, 4)) return at<u8 **>(table, 0)[i];
    return nullptr;
}

void __fastcall CursorTable_Clear(u8 *table) {
    if (at<u8 **>(table, 0)) {
        for (s32 i = 0; i < (s32)at<u16>(table, 4); ++i) {
            if (u8 *const cursor = at<u8 **>(table, 0)[i]) vcall<void>(cursor, 0, 1u);
        }
        GameHeap_Free(at<u8 **>(table, 0));
    }
    at<u16>(table, 4) = 0;
}

u16 __fastcall CursorTable_Init(u8 *table, void *, u32 count) {
    at<u8 **>(table, 0) = static_cast<u8 **>(GameHeap_Alloc((count & 0xffff) << 2));
    if (!at<u8 **>(table, 0)) return 0;
    for (s32 i = 0; i < (s32)(count & 0xffff); ++i) at<u8 **>(table, 0)[i] = nullptr;
    at<u16>(table, 4) = (u16)count;
    return 1;
}

void __cdecl Gui_SetCursorTable(u8 *table) {
    if (!table || table == global<u8 *>(0x00faa604)) return;
    if (global<u8 *>(0x00faa608)) vcall<void>(global<u8 *>(0x00faa608), 0x10, 0u);
    global<u8 *>(0x00faa604) = table;
    u8 *const cursor = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, global<u16>(0x0077c484));
    if (cursor) vcall<void>(cursor, 0x10, 1u);
    global<u8 *>(0x00faa608) = cursor;
}

void __cdecl Gui_SetCursor(u32 id) {
    if (!global<u8 *>(0x00faa604)) return;
    if ((s16)id == global<s16>(0x0077c484)) return;
    if ((s16)id == -1) id = global<u16>(0x00faa600);
    u8 had = 0;
    if (u8 *const old = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, global<u16>(0x0077c484))) {
        vcall<void>(old, 0x10, 0u);
        had = Fn_66190a(old);
    }
    u8 *c = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, (u16)id);
    if (!c) c = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, global<u16>(0x00faa600));
    global<u8 *>(0x00faa608) = c;
    global<u16>(0x0077c484) = (u16)id;
    if (!c) return;
    vcall<void>(c, 0x10, 1u);
    if (had) vcall<void>(c, 0x8);
}

void __fastcall Window_SetCursor(u8 *window, void *, u32 id) {
    at<u16>(window, 0x50) = (u16)id;
    if (window == global<u8 *>(0x00faa5e0)) Gui_SetCursor(at<u16>(window, 0x50));
}

void __fastcall Window_ApplyCursor(const u8 *window) { Gui_SetCursor(at<u16>(window, 0x50)); }

void __cdecl GameCursor_Set(u32 id) {
    u8 *const window = global<u8 *>(0x007cb2ac);
    if (!window) return;
    Window_SetCursor(window, nullptr, id);
    global<s32>(0x007cb2d8) = (s16)id;
}

void __fastcall Obj7b51f0_SetCursor(void *, void *, u32 id) {
    if (global<u8 *>(0x007cb2ac)) {
        GameCursor_Set(id);
        return;
    }
    if (u8 *const window = global<u8 *>(0x007cc8c0)) Window_SetCursor(window, nullptr, id);
    if (u8 *const window = global<u8 *>(0x007cc8bc)) Window_SetCursor(window, nullptr, id);
}

void __cdecl GameCursor_Update() {
    const auto pick = [](u32 otherwise) -> u32 {
        if (global<u32>(0x00816d60)) return 5;
        if (global<u32>(0x00816d4c)) return 0x12;
        return otherwise;
    };
    switch (global<u32>(0x0081ae2c)) {
    case 0x01: GameCursor_Set(pick(3)); break;
    case 0x03: GameCursor_Set(pick(4)); break;
    case 0x0b:
    case 0x10: GameCursor_Set(pick(6)); break;
    case 0x15: GameCursor_Set(global<u32>(0x00816d4c) ? 0x12 : 0xb); break;
    case 0x16: GameCursor_Set(global<u32>(0x00816d4c) ? 0x12 : 0x11); break;
    case 0x04:
    case 0x3b: GameCursor_Set(7); break;
    case 0x39: GameCursor_Set(3); break;
    default: break;
    }
    if (Global816d48_Get()) GameCursor_Set(8);
    if (global<u32>(0x00816d5c)) GameCursor_Set(0x14);
}

}
