// The coaster editor's adjust tools: toggling the adjustment of one of the selected piece's values. Reimplementation of 0x441cd0, 0x441e80 and
// 0x442010 (names unconfirmed).
//
// The piece being edited is [0x79c640] (its handle the word at +0; +8 flags; its definition at +0x100, whose +4 byte has flag 2 for a piece
// that cannot be adjusted and whose +0x18 points at the type flags). The editor's flags are 0x79c658: bits 0-5 the tool in use (2 and 4 the
// adjusters here), 0x200 / 0x400 the value(s) being adjusted, 0x2000 set with a tool picked before a piece. Each toggle, for a piece whose type
// allows it (or with its tool on already) and that can be adjusted: with no tool on, its tool goes on (and maybe 0x2000); with its tool on, the
// tool goes off and the value is recorded (the action recorder 0x785a08) and the game's changed flag 0x2000 at [0x790fec] cleared. Then with
// the tool on: 0x79c630 / 0x79c634 take 0x790ac8 / 0x790acc, and for a piece the adjusted value's pointer (0x79c614, 0x79c6fc), its value
// (0x79c6f0, 0x79c6d4), range (0x79c61c..0x79c65c, 0x79c6e4..0x79c66c) and step (0x79c6ec, 0x79c6a8) are set and the highlight shown
// (Highlight_SetShown(1)); without a piece the tool goes off again. With the tool off, flags 0x1e00 are cleared and the highlight hidden.
// Each returns 1.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x441cd0 (cdecl): tool 4 for type flag 0x200: the value at +0xf8 (recorded as 0x3b with it, 0 and the handle). Taking it on a piece with
// flag 0x80 at +8, a value over 0.5 is first lowered by 1; its range is -0.35..0.35 with that flag, else 0.15..0.85; step 0.01.
u32 __cdecl CoasterEdit_ToggleAdjustF8();

// 0x441e80 (cdecl): only with a game ([0x790fec]) whose +0x40 is set: tool 2 for type flag 0x40: the value at +0xe0 (recorded as 0x3a with
// the handle), its range +0xd4 less and plus the type's +0x94, step 0.02; the piece's +8 gets flag 1.
u32 __cdecl CoasterEdit_ToggleAdjustE0();

// 0x442010 (cdecl): tool 4 for type flags 0x180 (0x2004 when taken with no tool): the angles at +0xd8 (type flag 0x100: 0x200, the piece's
// +8 flag 4) and +0xdc (type flag 0x80: 0x400, the piece's +8 flag 2), recorded as 0x3b with both and the handle; ranges -pi..pi
// (as floats 0xc0490e38 / 0x40490e38), step 0.02.
u32 __cdecl CoasterEdit_ToggleAdjustAngles();

// 0x4421e0 (cdecl; name unconfirmed): only with a game whose +0x40 is set and a piece whose definition lacks flag 2 (else 1):
// CoasterEdit_ToggleAdjustF8 when the piece's +8 has flag 0x40, else CoasterEdit_ToggleAdjustAngles.
u32 __cdecl CoasterEdit_ToggleAdjust();
}
