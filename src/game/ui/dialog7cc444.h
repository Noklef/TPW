// The window at 0x7cc444 (built from the script 0x7579c8 with the handler 0x4b7a70): a panel of the park analyser's recent figures,
// each the newest sample of one of its CircBufU32s of 144 and the one 12 samples before it. Reimplementation of 0x4b7b00. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b7b00 (no arguments; name unconfirmed): unless the window [0x7cc444] is up, Window7c24c8_Close, then the window built
// (WindowScript_Build of [0x7cb2ac], the script 0x7579c8, the handler 0x4b7a70; nothing more without it) into [0x7cc444] and made the
// [0x7c24c8] one (Window7c24c8_Set); its child 0xadb3 a hover label of the language text 0xba (HoverLabel_Attach). Its child 0xada1
// (nothing more without it) then has its children dressed (looked up each, not checked), each with a new part (0x1a4 bytes; none when
// the allocation fails) as its implementation and a font (Language_Font): 0xadae / 0xadaf (TextPart_Ctor, font 8, the texts 0xbc / 0xbb
// as 0x100), 0xada9, 0xada3, 0xada4, 0xada5 and 0xadb0 (TextPart_Ctor, font 6, the texts 0xbd-0xc1); then the game record of [0x7cf83c]'s
// analyser (GameTable7cfb90_Get1da720_519510), and the children given a figure through their slot 7: 0xada8 / 0xadac (TextPart6ff3b0_Ctor)
// the newest and the 12-back sample of its CircBufU32 1 (+0x1f350), 0xada2 / 0xadad (TextPart6ff350_Ctor, first slot 7 given 0 and 2, 1
// as 0xc00) those of 15 (+0x213b0), 0xada7 / 0xadaa of 5 (+0x1fc90), 0xada6 / 0xadab of 2 (+0x1f5a0), 0xadb1 / 0xadb2 of 0 (+0x1f100)
// (6ff3b0 parts but for 15's; font 6 each). A sample `back` before the cursor (+0x244) is there when the entries (+0x248) are more than
// back (signed) and the cursor is at least back, or it is not but the buffer has wrapped (+0x24c: the entries added to it). When it is
// not there, the figure is the one before it: for 15's two the dword the newest's allocation went to (sic: the first gives the 0xada2
// part's own pointer), for the others the one the last figure was (the first: the 0xadb0 part's pointer).
void __cdecl Dialog7cc444_Create();
}
