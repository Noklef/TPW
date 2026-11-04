// The swear filter over the global save info's two word lists (core/globalsave; names invented, unconfirmed): reimplementation of 0x5d3230 and its find 0x5d2d40.
#pragma once

#include "../../../re/types.h"
#include "stdwstring.h"

struct SwearFilterEnv {
    u32(__cdecl *lower)(u32 c);   // towlower (0x67e5d0, C library, agent A's)
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
}
