#include "guisettings.h"

#include <cstdint>

#include "../core/accessors.h"
#include "guisystem.h"
#include "cursor.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void CursorSlot(u32 slot) {
    u8 *const cursor = gCursor_faa608;
    if (cursor) (*reinterpret_cast<void(__fastcall **)(u8 *)>(*reinterpret_cast<u32 *>(cursor) + slot * 4))(cursor);
}

}  // namespace

extern "C" {

void __cdecl Gui_SetWords540(u32 a, u32 b) {
    gGuisettings_faa540 = static_cast<u16>(a);
    gGuisettings_faa544 = static_cast<u16>(b);
}

u32 __cdecl Gui_List5f0Count() { return gGuisystem_faa5f0 ? Fn_669c30(gGuisystem_faa5f0) : static_cast<u32>(-1); }

void __cdecl Gui_SetScreenSize(u32 width, u32 height) {
    global<u16>(0x00faa5c4) = static_cast<u16>(width);
    global<u16>(0x00faa5c0) = static_cast<u16>(height);
}

void __cdecl Gui_SetWord600(u32 value) { gGui_faa600 = static_cast<u16>(value); }

void __cdecl Gui_CursorSlot3() { CursorSlot(3); }

void __cdecl Gui_CursorSlot2() { CursorSlot(2); }

void __cdecl Gui_SetPointer48c(u32 value) { global<u32>(0x0077c48c) = value ? value : 0x00faa550; }

void __cdecl Gui_SetPointer490(u32 value) { global<u32>(0x0077c490) = value ? value : 0x00faa580; }
}
