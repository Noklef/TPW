// The track's 2x2 square test (0x5286a0): the four map cells from (x, y) to (x + 1, y + 1) each put to agent B's cell test 0x532fc0 with
// the flags of the track kind being built.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the square test calls in the game that is not ported yet (the benches point it at a recorder).
struct TrackSquareEnv {
    u32(__fastcall *test532fc0)(u8 *cell, void *, u32 flags, u32 five, u32 direction);   // 0x532fc0 (agent B's, thiscall on a map cell, pops 0xc)
};
// The square test's calls go to env (nullptr: the game's).
void TrackSquare_SetEnv(const TrackSquareEnv *env);

// 0x5286a0 (cdecl (x, y, unused, skip, strict, direction); name invented): nothing (1) unless (x, y), (x, y + 1), (x + 1, y) and
// (x + 1, y + 1) are all on the map (Map_InBounds) and the track kind being built (Global81ecac_IfNoFlag) is 1 or 2. The cells (of
// [0x7cf83c], ids (y << 7) + x + 1, + 0x81, + 2, + 0x82 cut to 16 bits) in that order each get 0x532fc0 (flags, 5, Byte_ShiftLog2(direction,
// 0x10)'s low byte), flags 0x200 with strict, else 0x100, or'ed with 0x10 for kind 2 and 0xb for kind 1: with strict a cell answering 0
// stops it (0); without strict, skip set calls none. Returns 1 or 0 in AL.
u8 __cdecl TrackSquare_Check(s32 x, s32 y, u32 unused, u32 skip, u32 strict, u32 direction);
}
