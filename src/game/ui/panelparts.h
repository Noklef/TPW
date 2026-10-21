// The steps the game's panels dress their windows' children with (each panel's slot 3, and the dialog [0x7cc1f8]'s stats): a child
// looked up by id (Window_FindChild, not checked unless the step says so) and given a new implementation (Window_SetImpl): a text part
// (0x1a4 bytes, GameHeap_Alloc; its constructor with 0, none when the allocation fails) with font 6 (Language_Font; Window_Set4c), or a
// bar (Window_SaveInsetRect(child, 0, 10) first, then 0x124 bytes, MeshOverlay_Construct with the texture's name). The originals inline
// these; the names are unconfirmed.
#pragma once

#include <cstdint>

#include "../../../re/types.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../render/textparts.h"
#include "window.h"

namespace panelparts {

using PartCtor = u8 *(__fastcall *)(u8 *, void *, u32);

// the bars' texture names in the game's data
inline const char *const kRideStatBar = reinterpret_cast<const char *>(0x00750e00);   // "ridestatbar.wct"
inline const char *const kHappyGrad = reinterpret_cast<const char *>(0x007508c0);     // "happygrad.wct"

// the child `id` given a part made by ctor and font 6; returns the child
inline u8 *dress(u8 *window, PartCtor ctor, u32 id) {
    u8 *const child = Window_FindChild(window, nullptr, id);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? ctor(memory, nullptr, 0) : nullptr);
    Window_Set4c(child, nullptr, Language_Font(6));
    return child;
}

// a TextPart_Ctor part showing the language text `text` (Window_NotifyImpl100)
inline void text(u8 *window, u32 id, u32 text) {
    u8 *const child = dress(window, TextPart_Ctor, id);
    Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(text));
}

// a TextPart_Ctor part given no text (Window_NotifyImpl100(0))
inline void textNone(u8 *window, u32 id) { Window_NotifyImpl100(dress(window, TextPart_Ctor, id), nullptr, 0); }

// a part made by ctor, then the child's slot 7 given value
inline void part(u8 *window, PartCtor ctor, u32 id, u32 value) { vcall<void>(dress(window, ctor, id), 0x1c, value); }

// a part made by ctor and nothing more
inline void part(u8 *window, PartCtor ctor, u32 id) { dress(window, ctor, id); }

// as dress(), when there is such a child (none allocated else); returns the child or none
inline u8 *dressIfAny(u8 *window, PartCtor ctor, u32 id) {
    u8 *const child = Window_FindChild(window, nullptr, id);
    if (!child) return nullptr;
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? ctor(memory, nullptr, 0) : nullptr);
    Window_Set4c(child, nullptr, Language_Font(6));
    return child;
}

// text(), when there is such a child
inline void textIfAny(u8 *window, u32 id, u32 text) {
    if (u8 *const child = dressIfAny(window, TextPart_Ctor, id)) Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(text));
}

// part() with a value, when there is such a child
inline void partIfAny(u8 *window, PartCtor ctor, u32 id, u32 value) {
    if (u8 *const child = dressIfAny(window, ctor, id)) vcall<void>(child, 0x1c, value);
}

// the child `id`'s saved rectangle inset and a bar of the texture `name` as its implementation; returns the child
inline u8 *barOf(u8 *child, const char *name, s32 dx = 0, s32 dy = 10) {
    Window_SaveInsetRect(child, dx, dy);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
    Window_SetImpl(child, nullptr, memory ? MeshOverlay_Construct(memory, nullptr, name) : nullptr);
    return child;
}

// a bar, then the child's slot 7 given value
inline void bar(u8 *window, const char *name, u32 id, u32 value) {
    vcall<void>(barOf(Window_FindChild(window, nullptr, id), name), 0x1c, value);
}

// a bar when there is such a child, and nothing more
inline void barIfAny(u8 *window, const char *name, u32 id) {
    if (u8 *const child = Window_FindChild(window, nullptr, id)) barOf(child, name);
}

// the panels' slot 4 (the same code in each: 0x496f90, 0x4ad720, 0x4afaa0, 0x4b0ed0): with the window +0x1c set (read again for each
// step), it raised (Window_SetDepth: its depth word +0xe0 plus 0xb) and hidden (Window_SetShown(0)), then given a TextPart_Ctor part
// (none when the allocation fails), Window_NotifyC00(1, 1), font 8 (Language_Font; Window_Set4c), Window_Notify40000(0, 0, 0, 0xff)
// and no text (Window_NotifyImpl100(0)). What the window is is unconfirmed (a tip, by its depth).
inline void tip(u8 *panel) {
    u8 *const *const window = reinterpret_cast<u8 *const *>(panel + 0x1c);
    if (!*window) return;
    Window_SetDepth(*window, nullptr, (u16)(*reinterpret_cast<const u16 *>(*window + 0xe0) + 0xb));
    Window_SetShown(*window, nullptr, 0);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(*window, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(*window, nullptr, 1, 1);
    Window_Set4c(*window, nullptr, Language_Font(8));
    Window_Notify40000(*window, nullptr, 0, 0, 0, 0xff);
    Window_NotifyImpl100(*window, nullptr, 0);
}

}  // namespace panelparts
