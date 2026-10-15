// A text scanner's character classes and token reader (the level and terrain text files):
// reimplementation of 0x427450-0x427614. The scanner is +0 the next character, +4 its position
// and +8 the text's length; the class tests take a pointer to the next-character pointer and
// compare the character signed (bytes from 0x80 are in no class but the blank one).
#pragma once

#include "../../../re/types.h"

struct TextScan {
    const char *next;
    u32 pos;
    u32 length;
};

extern "C" {

// 0x427450 / 0x427480 / 0x4274a0: 1 when **at is a letter (A-Z, a-z) / a digit / punctuation
// (! to /, : to @, [ to `, { to ~), else 0.
u32 __cdecl TextScan_IsAlpha(const char *const *at);
u32 __cdecl TextScan_IsDigit(const char *const *at);
u32 __cdecl TextScan_IsPunct(const char *const *at);

// 0x4274e0: 1 when **at is none of those (nor '#' or ';', which are punctuation anyway).
u32 __cdecl TextScan_IsBlank(const char *const *at);

// 0x427540: reads into out (emptied first) the run of characters in the chosen classes
// (letters, digits, punctuation, '#' / ';'), at most size - 1 of them. Returns 1 when the
// text ran out (at the start, or after a character taken at the last position, which is not
// stepped past), else 0 (stopped at a character outside the classes or by size).
// sic: once pos has reached the length the character there is still taken.
u32 __cdecl TextScan_Read(TextScan *scan, char *out, s32 size, u32 letters, u32 digits, u32 puncts, u32 marks);
}
