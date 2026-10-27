// Whether a kind of thing can be built on a map cell: the build tools' checks of a cell (thiscall on a 0x44-byte map cell: its id word +0,
// type +8, direction +0xd, flags +0xe / +0xf, owner +0x10), answering 0 (no), 1 (yes) or a code, and leaving their findings in the
// globals 0x816d4c, 0x816d50 (blocked), 0x816d54, 0x816d58, 0x816d5c (too dear) and 0x816d60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x535670 (thiscall (kind, alone), pops 8; name unconfirmed): kind 0x1f gives 2, 0x3a 0xd; on a flag 0x20 cell kinds 4, 9 and 0xa give 1,
// and kind 0x18 gives 0 on a type 1 cell; 0x18 is then taken as 1. Kind 0x36: whether 0x816d50 is set. A type 4 / 9 / 0xa cell's object
// (Cell_FirstObject of the world's cell its +0x10 names; the thing's slot read whether set or not) of kind 3 sets 0x818c14 to its
// definition's +0x54 (ObjectDefs_Find). Then, in order: 0xb (0x816d4c set) for kind 0x15 on an owned (+0x10 = 0x818698) flag-0x100 type
// 0x15 cell while 0x816d50 is clear; 1 on a flag 0x40 cell; on a type 4 / 9 / 0xa cell 0 when 0x818c10 > 0x818c14 (signed) for a kind
// other than 0x15 / 0x16 while 0x816d58 and 0x816d50 (as read before) are clear, else 1 with 0x816d50 set. Otherwise the cell's entry
// (MapHandle_Entry28; for a 0xc / 0x11 one the world's entry of its parent) decides: kinds 0xb / 0x10 on a track entry (type 0xb, 0xd,
// 0x10, 0x12 or 0x19) without 0x8187f8 give 1 with 0x816d50 set; 0x816d58 sets 0x816d50; without alone a set 0x816d50 gives 1; a type
// 0x19 entry gives 1 with 0x816d50 set; a compatible kind (Kinds_Compatible) unlike the cell's adds the price (0x81b6dc for kind 1,
// 0x81ea14 for 3, else 0) to the cost (Global81af4c_Add) unless 0x8186d4 or kind 3 on a type 1 cell; a cost over the money (Source_Field0c
// of the world's table entry, signed) when the game is not free (0x785a2c) gives 1 with 0x816d50, 0x816d48 and 0x816d5c set; then the
// kinds' own rules (kind 0x14, 3 on a type 3 cell, the cell's own kind under 0x820aa4 / 0x818688 / 0x820ab8, 0x15 / 0x16, 4, the entry
// at a corner or junction, kind 1 beside a single type 3 neighbour giving 8 with 0x816d60 set, ...). Returns 0, 1, 2, 8, 0xb or 0xd.
u32 __fastcall MapCell_CanBuild(u8 *cell, void *, u32 kind, u32 alone);

// 0x539760 (thiscall (kind, alone, direction byte), pops 0xc; name unconfirmed): the same for track-like kinds (0xb / 0x10 track, 0xe / 0x13,
// 0xf, 0x19 hoardings) facing a direction. Kind 0x39: whether 0x816d50 is set. The cell (MapHandle_Cell44 of its id) and its three steps
// (Map_StepCell by the pairs 0x7cdbc8, 0x7cdbb8, 0x7cdba8) are blocked when any is of type 4, 7 / 0x1e, 0xa, 9 or 0x15; 1 when any has flag
// 0x40; 0 when 0x818c10 is above the object priority of a footprint cell among the cell, the first step and the second (looked at twice;
// the third never: sic). For 0xb / 0x10 the price (0x81aebc / 0x81b43c) is taken and the three entries beside (Map_StepCell28) that are,
// or whose parents are, track owned by another player block it and set 0x816d50. Then 0x816d58 sets 0x816d50, which without alone gives
// 1; a hoarding on a hoarding only at either of the points 0x7639f0 / 0x7639f8 (else 0), on a type 0 or 7 cell 0, elsewhere 1 with
// 0x816d50 set; 0x816d54 set for the kind's own owned cell; the price added to the cost (Global81af4c_Add) for a new type, and the cost
// over the money (unless free or kind 0xe / 0x13) gives 1 (0x816d50 and 0x816d48 set unless alone, 0x816d5c set); then the joins (under
// 0x820ac8 / 0x818688 / 0x820ad0: 0xb with 0x816d4c set when the direction fits), the facing rules (Nibble_Scale of the cell's direction
// +0xd), 0xd for kind 0xf, 9 for kinds 0xe / 0x13 on an owned track cell without more than two side neighbours, the corners. A refusal
// sets 0x816d58 (or 0x816d50) unless alone. Returns 0, 1, 9, 0xb or 0xd.
u32 __fastcall MapCell_CanBuildTrack(u8 *cell, void *, u32 kind, u32 alone, u32 direction);
}
