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

// 0x432430 (cdecl; ten arguments, the last four unused; name invented, unconfirmed): one node's handles. Its +0x28 the way (raw), +0x1c its negative, each resized: over their own
// length by before (+0x1c) and after (+0x28), by rules +8; with a railed info (bit 0x4000) by a size from rules +8 (the node +0x10 piece's way to its neighbour, by flag 0x80, against the
// sine table at its +0xec; then at its +0xf8 modulo 0.5) where that piece's +0x68 (flag 0x80, before) or +0x64 (else, after) has flag 0x40, else by 1.7, the other side always by the size.
// +0x34 and +0x4c are then the place +0x40 plus a third of each.
void __cdecl TrackNode_Handles(const u8 *rules, const float *way, u8 *node, float before, float after, const u8 *info, void *, void *, u32, u32);

// 0x4321c0 (cdecl; piece, source; name invented, unconfirmed): for each node of the piece (+0x90: count, the lists at +4 and +8, 0x58 bytes a node) the handles of both lists' nodes, the way
// normalised: with a source its +8's +0x28's +4 list's [the node's +4] (20 bytes an entry, +4 a pointer: +0x20), else straight up or down with flag 0x40 (down without 0x80), else the first
// list node's +4's +0x28. The gaps from TrackPiece_NodeGaps, for each list. rules = [0x792218] +4, the info = the piece's +0x100's +0x18.
void __cdecl TrackPiece_NodeHandles(const u8 *piece, const u8 *source);
