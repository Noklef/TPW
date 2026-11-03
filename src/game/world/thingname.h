// A thing's two names as text. Reimplementation of 0x4e2860 and 0x4e2920; the names are invented, unconfirmed (src/game/ui/signnamedialog.h calls them nameA, nameB).
#pragma once

#include "../../../re/types.h"

// The calls (the port by default; the bench's recorder).
struct ThingNameEnv {
    s32(__cdecl *snwprintf)(u16 *to, u32 count, const u16 *format, ...);   // CRT_snwprintf
};

void ThingName_SetEnv(const ThingNameEnv *env);

// 0x4e2860 (thiscall (to, count), pops 8): the text +0x3c8 into to: empty, the format 0x78a374 alone. A text of 2 or more ending "--": first through the static 0x7cdc58 (0x21 units,
// its word +0x40 cleared first), with its last unit cleared (at the static's word (length - 1); not bounded by its size, sic). Else with the format 0x75c0cc.
void __fastcall Thing_NameA(u8 *thing, void *, u16 *to, u32 count);

// 0x4e2920 (thiscall (to, count), pops 8): the text +0x40a into to, with the format 0x75c0cc.
void __fastcall Thing_NameB(u8 *thing, void *, u16 *to, u32 count);
