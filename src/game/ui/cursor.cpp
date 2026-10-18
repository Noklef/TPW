#include "cursor.h"

#include <cstdint>

#include "../core/accessors.h"
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

}
