// A theme record's names as text (the record: see balancefile.h, globalsave.h): its own, +0x34 and +0x76 (mSignNameA, mSignNameB) when the name has been changed (+0xb8), else the object definitions' for
// its global.sam's id (+0x1c's +0x18).
#pragma once

#include "../../../re/types.h"

struct ThemeNamesEnv {
    s32(__cdecl *snwprintf)(u16 *to, u32 count, const u16 *format, ...);          // CRT_snwprintf
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, ...);                      // CRT_swprintf
    void(__fastcall *fullName)(u8 *defs, void *, u32 id, u16 *out, u32 size);     // ObjectDefs_GetFullName
    void(__fastcall *name)(u8 *defs, void *, u32 id, u16 *out, u32 size);         // ObjectDefs_GetName
    void(__fastcall *suffix)(u8 *defs, void *, u32 id, u16 *out, u32 size);       // ObjectDefs_GetSuffix
};
void ThemeNames_SetEnv(const ThemeNamesEnv *env);  // nullptr: the game's

extern "C" {

// 0x5b12e0 (thiscall (to, count), pops 8; name invented, unconfirmed): the name changed: an empty A gives B (wcsncpy), an A ending in "-" is cut of it in the static 0xf7bf78 (0x20 characters) and joined to B
// by the format 0x749850, an empty B gives A (wcsncpy), else both by the format 0x75c0c0. Not changed: the full name of the definitions 0x7890a0, or the text 0x78a374 (swprintf) with no global.sam.
void __fastcall ThemeRecord_FullName(u8 *self, void *, u16 *to, u32 count);

// 0x5b1410 (thiscall (to, count), pops 8; name invented, unconfirmed): the name changed: an empty A gives the text 0x78a374, an A of two or more ending in "--" is cut by one in the static 0xf7c800, else A. Not
// changed: the definitions' name, or the text 0x78a374 (swprintf) with no global.sam.
void __fastcall ThemeRecord_Name(u8 *self, void *, u16 *to, u32 count);

// 0x5b1520 (thiscall (to, count), pops 8; name invented, unconfirmed): the name changed: B. Not changed: the definitions' suffix, or the text 0x78a374 (swprintf) with no global.sam.
void __fastcall ThemeRecord_Suffix(u8 *self, void *, u16 *to, u32 count);
}
