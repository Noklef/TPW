// Whether a coaster piece may go on a tile: reimplementation of 0x434740 (cdecl; name invented, unconfirmed).
//
// The coaster being built is [0x790fec] (+4 its type, whose two rule blocks are at +0xa8 and +0x1c8; +0x3c flags; +0x54, +0x58,
// +0x60, +0x64 pieces); the editor's flags are 0x79c658 (0x100 moving the selected piece [0x79c6a4], 0x40 the second rule block).
// A piece has +0 its kind, +8 flags (0x40, 0x10000, 0x20000), +0x64 the piece before it, +0x68 the one after, +0x100 its node (+4
// flags, +8 its tile's index, +0xc / +0x10 its tile, +0x14 its coaster, +0x18 its rules, +0x28 the next node of its link, +0x30 a
// count against the rules' +0xcc, +0x34 an object whose +8 flags 0x200 / 0x400 matter). The map is [0x7a0854]+0x6c (+0x18 width,
// +0x1c height); [0x79fc54]+0x2c its tiles' flags (dwords, bit 0 taken). A rule block: +0x74..+0x90 two pairs of angle limits
// (+0x74 / +0x7c and +0x78 / +0x80 with the node's flag 2, else +0x84 / +0x8c and +0x88 / +0x90), +0x9c / +0xa0 the squared
// distances allowed, +0xa4 / +0xa8 the distances the limits are interpolated between.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x434740: (x, z) the tile; piece the piece placed from (null: the editor's, by its flags, its tile and its next's tile read from
// their nodes; given: (px, pz) its tile and (nx, nz) its next's). 0 off the map or (no piece) on a taken tile. From a piece: 0 on
// its own tile; outside the squared distances 0 when the piece's kind is 2 or its node's flag 2 is clear; with a next piece
// (unless held by the 0x10000 / 0x20000 flags while moving), the angle at the piece between the tile and the next tile (CRT acos
// of the unit vectors' dot, clamped to -1..1) must lie within the limits interpolated by the distance. Then (no piece) a node of
// the coaster links (0x790fe0, +0x4c, +0x28) on the tile, with its +4 bits 0 and 2 clear, must belong to this coaster and to the
// rules in force, under their +0xcc count, its object's flags 0x600 (0x200 when not moving) clear; on the tile of the coaster's
// +0x64 (flag 2) or +0x54 (no flag 1) piece it holds that piece (0 when 0x79c664 is 1), as moving holds the selected one. A held
// piece with a piece before it (and not flagged 0x10000) is checked again: the one before it placed from the held piece standing
// on the tile, and the one before that from it. 1 or 0.
// sic: with no piece and the chosen piece without a next, nx is left unset where the angle test may read it (the bench keeps
// that test away: the original reads the uninitialised slot).
s32 __cdecl CoasterPiece_CanPlaceAt(u32 x, u32 z, u8 *piece, u32 px, u32 pz, u32 nx, u32 nz);
}
