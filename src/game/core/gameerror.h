// The game's error report (the object 0x405750 shows: +0 set, +4 the error's index 0-0x2a into the
// string table's messages, +8, +0xc a text of 0x50): reimplementation of 0x405b90 and 0x405710.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorder): Charset_FormatWide.
void GameError_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x405750 (thiscall; name invented, unconfirmed): with the report set and [0x78b530] set, its message in a message box (MessageBoxW, "Theme Park World", 0x10010). Index 0x2a or below but 0xc and
// 0xd: the entry of the table at the jump's arms in the string table [0x78ba88], as 0x100 wide chars. Index 0xc: the engine's errors (Entries8_Pop of 0x879ad0) popped to the end, the last of kind 3
// giving its code (none: 0), the report set to that code's index (GameError_IndexOfCode), +8 cleared, and shown instead. Index 0xd or past 0x2a: no text is made, so the box shows the uninitialised
// buffer (sic). A text at +0xc is added, "%ls (%hs)" over the buffer itself.
void __fastcall GameError_Show(u8 *report);

// 0x405b90 (thiscall, the report unused; code, pops 4; name unconfirmed): the error index of a library code:
// 0xb0754000-0xb0754018 give 0xe-0x26 in turn, 0xb0754100 / 0xb0754101 / 0xb0754102 give 0x27 / 0x28 / 0x29,
// any other 0x2a.
u32 __fastcall GameError_IndexOfCode(void *report, void *, u32 code);

// 0x405710 (thiscall; index, arg, text, pops 12; name unconfirmed): the report set: +4 the index, +8 arg, the text (when given) into +0xc
// (strncpy of 0x50: no NUL when it is that long), then +0 = 1.
void __fastcall GameError_Set(u8 *report, void *, u32 index, u32 arg, const char *text);
}
