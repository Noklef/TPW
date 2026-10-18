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

// 0x40caa0 (name unconfirmed; the low words count): for an action in range, the formatter (TextFormat) writes text's first
// character (table 0x78ba84, two characters) over the action's own argument, and the word left there becomes the key (+2) of
// the action's entry (its own index, else the first for it; none: nothing).
void __fastcall KeyMap_SetKeyFromText(u8 *map, void *, u32 action, u32 text);

// 0x40cb40: enabled = 1.
void __fastcall KeyMap_Enable(u8 *map);

// 0x40cb50: enabled = 0 and every held flag cleared (entries and count read again each step).
void __fastcall KeyMap_Reset(u8 *map);

// 0x40cf60 / 0x40cfa0: Enable / Reset on all five maps.
void __cdecl KeyMaps_EnableAll();
void __cdecl KeyMaps_ResetAll();

// 0x5ed700 (cdecl; index, shift, a, b, c): the 68-byte record at 0xf86918 + 68 * index: with shift set the dwords at +4 and +8 = a, b;
// otherwise they get those at +0xc and +0x10; then +0xc = a, +0x10 = b and +0 = c.
void __cdecl KeyRecord_Push(u32 index, u32 shift, u32 a, u32 b, u32 c);

// 0x486b60 / 0x486b70 (cdecl): KeyMaps_EnableAll / KeyMaps_ResetAll, then the byte at 0x7c24d0 = 0 / 1.
void __cdecl KeyMaps_EnableAllAndClearFlag();

void __cdecl KeyMaps_ResetAllAndSetFlag();

// 0x40d070 (cdecl; name unconfirmed): the blocks of the maps system, game, camera, cheat, 0x787294 and shortcuts freed (each when set; the
// pointers are left as they were).
void __cdecl KeyMaps_FreeAll();

// 0x46b460 (cdecl; code): the low word of code remapped when it is 0x27, 0x2c, 0x2e, 0x2f, 0x3b or 0x60 to 0x7a (0x27 -> 0x2c, 0x2c -> 0x5ea, 0x2e -> 0x5e5, 0x2f -> 0x2e,
// 0x3b -> 0x5e3, 0x60 -> 0x3b, 0x71 -> 0x2f, 0x77 -> 0x27, the others of 0x61 to 0x7a to 0x5d0 to 0x5e9); the upper word and any other code are returned as they are.
u32 __cdecl Code_Remap46b460(u32 code);

// 0x40c900 / 0x40c990 (thiscall on {entries, count (word at +4), enabled (word at +6)}; a, b, pops 8): 0 unless enabled. Otherwise the first of the 0x14-byte entries (count
// of them, signed) whose words at +2 and +4 are the low words of a and b has its handler (a pointer at +0x10 / +0xc; called with ECX = the pointer, no arguments) called when
// there is one, and its word at +6 set to 1 / 0. Returns what the handler returned (0 for none, or no entry).
u32 __fastcall Actions_FireDown(u8 *table, void *, u32 a, u32 b);

u32 __fastcall Actions_FireUp(u8 *table, void *, u32 a, u32 b);
// 0x46b420 / 0x486aa0 (cdecl): the modifier keys held, from GetKeyState / GetAsyncKeyState (import slots 0x6fd324 / 0x6fd320; held when the
// SHORT answer is negative): 3 for shift (VK_SHIFT), 0xc for control (VK_CONTROL), 0x30 for alt (VK_MENU), or'd.
u32 __cdecl KeyMap_Modifiers();
u32 __cdecl KeyMap_AsyncModifiers();
}
