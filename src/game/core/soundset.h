// The sound module's settings (read by its update, 0x51bd80, when the changed flag 0x803ac8 is
// set, which it then clears): reimplementation of the cdecl setters 0x51bb60-0x51bc20, 0x51e6f0
// and 0x51e710, which store their value and set the changed flag, of 0x51c1c0 and 0x51c6f0, and of
// the sound commands' base constructor 0x51bb40. The options code sets them in pairs, a flag around a value
// (0x51bbe0 1, 0x51bb80 value, 0x51bbe0 0; likewise 0x51bc20 / 0x51bbc0 and 0x51bc00 / 0x51bba0);
// the update scales 0x803ac0 and 0x803abc by the percentage at 0x785914, like volumes.
#pragma once

#include "../../../re/types.h"

extern "C" {

void __cdecl Sound_Set803ab8(u32 value);  // 0x51bb60
void __cdecl Sound_Set803ac0(u32 value);  // 0x51bb80
void __cdecl Sound_Set803abc(u32 value);  // 0x51bba0
void __cdecl Sound_Set803ac4(u32 value);  // 0x51bbc0
void __cdecl Sound_Set803adc(u32 value);  // 0x51bbe0
void __cdecl Sound_Set803ad8(u32 value);  // 0x51bc00
void __cdecl Sound_Set803ae0(u32 value);  // 0x51bc20
void __cdecl Sound_Set803acc(u32 value);  // 0x51e6f0
void __cdecl Sound_Set803ad0(u32 value);  // 0x51e710: its low 16 bits, a u16

// 0x51c1c0: the u16 at 0x803ad2 (when set, 0x51c1d0 plays at the fixed volume [0x700b28]), the
// changed flag left alone.
void __cdecl Sound_Set803ad2(u32 value);

// 0x51c6f0: the sound-on flag 0x802bc8 (set by 0x51b660; the play functions do nothing without it).
u32 __cdecl Sound_IsOn();

// 0x51bb40 (thiscall; value): a sound command's base, abstract (vtable 0x700b58, _purecall
// first), +4 value; returns it. Its callers then set their own vtable and post it to 0x802bcc.
u8 *__fastcall SoundCmd_Ctor(u8 *cmd, void *, u32 value);

// 0x51b4f0 (cdecl; a, b): 0 unless sound is on (0x802bc8) and started (0x802bd4); else the sound manager (0x802bcc) is
// asked through its vtable +4 (&a, a, b, &b) and b as it left it is returned. Its caller asks with a category's name
// ("cat_ride") and keeps the answer.
u32 __cdecl Sound_Query(u32 a, u32 b);

// 0x51b530 (cdecl; name, folders; name unconfirmed): the name looked up in a folder list (Class6fd9d8_FindFirstMatch with a String of the name and an
// empty String for what it finds); 0 when it is not found (the match's AL not 1), or sound is not on (0x802bc8) and started (0x802bd4). Else the
// sound manager (0x802bcc) is asked through its vtable +4 (&folders, the found String's text, name, &name) and name as it left it is returned.
u32 __cdecl Sound_QueryFound(const char *name, u8 *folders);

// 0x51b7c0 (cdecl): when on and started, the sound manager's vtable +0 (&result, not read after); then not started. The
// warning when the allocation count 0x802bd8 is not back to 0 is compiled out.
void __cdecl Sound_Shutdown();
}
