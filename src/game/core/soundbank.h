// The sound bank's file: reimplementation of 0x6c58f0 and the bank opener 0x6c5a00 (the rest of the bank is the exe's).
#pragma once

#include "../../../re/types.h"

struct String;

extern "C" {

// 0x6c58f0 (thiscall (opener, path, hdFirst), pops 0xc; `this` unused): path (a String) gets "HD.sdt" (hdFirst not 0) or "HW.sdt"
// appended (its vtable +0x20) and the opener's slot 1 is called (&status, the path's text, &info: a local whose +0x20 points to an
// empty text at +0x24, its first 0x20 bytes unset); when the dword the call returns a pointer to is negative, path is set back to
// a copy taken on entry (vtable +0x18) and gets the other suffix appended, for the caller to try (nothing is opened again here, sic).
void __fastcall SoundBank_OpenSdt(void *bank, void *, u8 *opener, String *path, u32 hdFirst);

// 0x6c5a00 (thiscall (out, opener, path, unused, hdFirst), pops 0x14; `this` passed on, unused; an SEH frame for the copy): a copy of
// path (String_CtorCopy) set back into it (vtable +0x18), SoundBank_OpenSdt (opener, path, hdFirst), then the opener's slot 1 again
// (&status, the path's text, &info as SoundBank_OpenSdt's); *out = -1 when the dword it returns a pointer to is negative, else 0.
// The copy destroyed. Returns out.
u32 *__fastcall SoundBank_Open(void *bank, void *, u32 *out, u8 *opener, String *path, u32 unused, u32 hdFirst);

}  // extern "C"
