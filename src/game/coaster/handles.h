// A track piece's spline handles, from its neighbours. Name invented (unconfirmed).
#pragma once

#include "../../../re/types.h"

// 0x431660 (cdecl; piece, rules, coaster): clears the piece's flags 0xe000000. prev = +0x68 (none with flag 0x80000), next = +0x64 (none
// with 0x40000; with 0x100000 the coaster's +0x18's +0x64). Neither: nothing more. "Railed": a type info (+0x100's +0x18) with bit 0x4000.
// Handles a (+0x28) and b (+0x34); +0x40 and +0x58 are then the place +0x4c plus a third of each. The ways are over rules +0xc:
//   One neighbour: the way to it and its negative, which is which by the end; +0x94 = 0. The next only: b is the way. The prev only and
//   railed without flag 0x80: b is the way if the dot of (prev - prev's prev) with (prev - piece) is below the info's +0xb0, else a; else a.
//   Both: +0x94 = acos of the flat ways' dot (-1..1, a NaN to 1), negated on a negative cross product; dot = the ways' dot, normalised:
//     Below the info's +0xb0 (without an info -0.999; a NaN too) or above 0.999: railed: half the normalised ways' difference over +0xc by
//     their lengths, a or b by flag 0x80; else the ways themselves.
//     Else railed: the normalised ways' sum by rules +8 by each length, b negated. Else the sum and the ways' cross product, both
//     normalised, a = their cross product by rules +8 by the first length, b its negative by the second, and flag 0x10000000 kept when
//     rules' bit 8 is set, flag 0x40 clear and the cross product's y is below 0 (a NaN too).
// With an info that is not railed, and a railed info on the piece at +0x68 or +0x64 (the links, whatever the flags): a's and b's y = 0.
void __cdecl TrackPiece_Handles(u8 *piece, const u8 *rules, const u8 *coaster);
