// The GUI library's registry of named windows: a singly linked list at 0xfaa610 of 0x18-byte nodes (+0 the window, +4 its name, at most
// 12 characters with no terminator when it fills them (strncpy), +0x10 the name's hash, +0x14 the next node). Reimplementation of
// 0x6588b6, 0x659a58, 0x659b52, 0x659bc9 and 0x659c42. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6588b6 (cdecl (text, multiplier)): h = 0, then for each character (signed) h = (c ^ h) * multiplier; returns h.
s32 __cdecl Gui_HashName(const char *text, s32 multiplier);

// 0x659a58 (cdecl (window, name)): with a window and a name of at least one character, its hash (Gui_HashName, 0x2f); the list is walked
// to its last node, stopping early at a node with that hash -- the last node's own hash is never compared (sic). Unless one matched, a
// node is allocated (Mem_Alloc 0x18; a failure answers 0): the window's word +0x10 set to 1 (0x659ce0), the node gets the window, the
// name (strncpy, 12), the hash and no next, and goes on the end of the list (or becomes its head). 1 in AX for a node added, else 0.
u32 __cdecl GuiWindows_Register(u8 *window, const char *name);

// 0x659b52 (cdecl (destroy, a word)): every node freed (Mem_Free), the head left as it was (sic); for destroy (not 0) the windows (not 0)
// are first deleted through their slot 0 (thiscall (1)).
void __cdecl GuiWindows_Clear(u32 destroy);

// 0x659bc9 (cdecl (name)): the window of the first node whose hash is the name's (Gui_HashName, 0x2f) asked for its slot 0x20 (thiscall,
// no arguments) and that returned; 0 when none matches (or the window is 0).
u32 __cdecl GuiWindows_FindByName(const char *name);

// 0x659c42 (cdecl (hash)): the same for a hash given.
u32 __cdecl GuiWindows_FindByHash(s32 hash);

}
