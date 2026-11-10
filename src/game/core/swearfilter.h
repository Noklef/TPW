// The swear filter over the global save info's two word lists (core/globalsave; names invented, unconfirmed): reimplementation of 0x5d3230 and its find 0x5d2d40.
#pragma once

#include "../../../re/types.h"
#include "stdwstring.h"

struct SwearFilterEnv {
    u32(__cdecl *lower)(u32 c);   // towlower (0x67e5d0, C library, agent A's)
    u8 *(__fastcall *open)(u8 *buf, void *, const char *path, u32 mode);   // NFileBuf_Open (a recorder in the bench, which fills the buffer's get area)
    s32(__cdecl *fclose)(void *file);   // CRT_fclose
};
void SwearFilter_SetEnv(const SwearFilterEnv *env);  // nullptr: the game's

extern "C" {

// 0x5d2d40 (cdecl (text, word); name invented, unconfirmed): wcsistr: the first place in the text where the word stands (towlower, 16 bits), no retry on a part match's mismatching
// character (sic: "aab" does not find "ab"). An empty word: the text. Not found: 0. The text's end ends a part match without success.
u16 *__cdecl SwearFilter_Find(u16 *text, const u16 *word);

// 0x5d3230 (thiscall on the global save info (text, flag), pops 8; name invented, unconfirmed): for each word of the list [+4] (a list node: next +0, the wide string's pointer +0xc): a first ' ': the word
// from there, whole at a word's start; a first '*': from there, anywhere; else the word's part anywhere in a word, so from that word's start (the characters iswspace tells). Each place found in the
// text (null: 0x6fd6c4), not at such a start: passed over. At a start: the exceptions of the list [+0x10] (node: pointer +0xc, length +0x10) by wcsnicmp at that start (a '*' word: at the place), an exception
// passes it over. Otherwise the place starred when the flag's byte is set: the '*' word's characters; the others' from their start through the word and on to the next space or the end. The search goes on past it.
// Returns 1 when anything was to be starred, else 0.
u8 __fastcall SwearFilter_Apply(const u8 *self, void *, StdWString *text, u32 flag);

// 0x5d2de0 (thiscall, this unused (path, list, flag), pops 12; SEH frame dropped; name invented, unconfirmed): an ifstream<char> on the stack (the stack object is the original's: 0x210 bytes). A flag byte of 0 destroys it and answers 0. Opened with mode 0x21;
// no file (failbit set when the open answered 0): destroyed, answers 0. Otherwise, until eofbit: 2 bytes read at the line's next place (after end of file they stay what they were); a unit 10 ends the line. A line whose first low byte is '#' is
// skipped, and the place not put back (sic: every later line then counts as that line, on towards the end of the frame). Otherwise it is copied to the second buffer, 0x80 bytes on (sic: lines of 64 units or more overrun: their end is overwritten before it is
// reached), up to unit 13 or 10; each unit of it complemented up to the first that is 0; a std::wstring of it put at the list's end. Then fclose: unless that answers 0 (then _Init (0, 2)) failbit set. Destroyed, answers 1.
u8 __fastcall SwearFilter_LoadFile(u8 *self, void *, const char *path, u8 *list, u32 flag);

// 0x5d3140 (thiscall on the global save info (swears, allowed, flag), pops 12; name invented, unconfirmed): both lists (+0, sentinel +4, count +8; +0xc, +0x10, +0x14) emptied node by node (the first with CountedBuf_Deleter (0), the second with CountedBuf_Reset (1)), then
// SwearFilter_LoadFile (swears, self, flag); when that answers 0 so does this. Otherwise SwearFilter_LoadFile (allowed, self + 0xc, flag), whatever it answers, and 1. Both calls have the same stack place, so a read past the second file's end
// sees the first's units there (sic: a last line without its end, on a place where the first file had a 10, counts as ended). The first file's sees what the stack held: here 0.
u8 __fastcall SwearFilter_Load(u8 *self, void *, const char *swears, const char *allowed, u32 flag);
}

#include "gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class702960_vtable, 0x00702960, 0x4);
