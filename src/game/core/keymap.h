// The keyboard maps ("system" 0x78718c, "game" 0x787a60, "camera" 0x787c14, "cheat" 0x78733c,
// "shortcuts" 0x78721c; built at 0x40cb80): reimplementation of 0x40c870, 0x40c8c0, 0x40ca20,
// 0x40cb40, 0x40cb50, 0x40cf60 and 0x40cfa0. A map is +0 its entries, +4 (s16) their count,
// +6 (s16) enabled, +8 its name; an entry (0x14 bytes) +0 (s16) its action, +2 its key, +4 its
// modifiers, +6 (s16) set while held. An action's entry is usually at its own index.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40c870: the index of the first entry for key with modifiers (+2, +4), or -1.
s32 __fastcall KeyMap_FindKey(const u8 *map, void *, s16 key, s16 modifiers);

// 0x40c8c0: the index of the first entry for action, or -1.
s32 __fastcall KeyMap_FindAction(const u8 *map, void *, s16 action);

// 0x40ca20: the held flag of action's entry (its own index, else the first for it); 0 for an
// action out of range or not found. Only AX is set.
u16 __fastcall KeyMap_IsHeld(const u8 *map, void *, s16 action);

// 0x40cb40: enabled = 1.
void __fastcall KeyMap_Enable(u8 *map);

// 0x40cb50: enabled = 0 and every held flag cleared (entries and count read again each step).
void __fastcall KeyMap_Reset(u8 *map);

// 0x40cf60 / 0x40cfa0: Enable / Reset on all five maps.
void __cdecl KeyMaps_EnableAll();
void __cdecl KeyMaps_ResetAll();
}
