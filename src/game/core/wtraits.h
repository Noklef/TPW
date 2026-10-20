// The 16-bit character traits the game's wide strings use (the IME at 0x47905e, 0x47aebf,
// 0x47b674): reimplementation of 0x479150, 0x479230 and 0x478cf0, and of the wide string's
// 0x47ba30, and the GUI library's length (0x665f60).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x479150: moves n characters from src to dst a character at a time, from the end when dst
// lies inside [src, src + n), else from the start; returns dst.
u16 *__cdecl WTraits_Move(u16 *dst, const u16 *src, u32 n);

// 0x479230: copies n characters from src to dst, from the start; returns dst.
u16 *__cdecl WTraits_Copy(u16 *dst, const u16 *src, u32 n);

// 0x478cf0: n characters of ch (its low 16 bits) at dst; returns dst.
u16 *__cdecl WTraits_Assign(u16 *dst, u32 ch, u32 n);

// 0x47ba30 (thiscall): the IME's wide string's length (+8) set and its characters (+4) ended there.
void __fastcall ImeWString_SetLength(u8 *string, void *, u32 length);

// 0x665f60 (cdecl (text); the GUI library's): the text's length in characters (wcslen).
u32 __cdecl WTraits_Length(const u16 *text);
}
