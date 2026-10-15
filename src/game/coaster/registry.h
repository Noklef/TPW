// The coasters: the table at 0x790bd0 (by index), their count (0x792214) and the list of
// coaster links at 0x790fe0 (next +0x18, coaster key +0x24).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x438720: the coaster count (0x792214).
u32 __cdecl Coaster_Count();

// 0x438730: the node list (+0x54 -> +0x90 -> +4) of link n of the list at 0x790fe0, its
// +0x28 to *value when value is not null.
// sic: n past the list's end follows a null link.
u32 __cdecl CoasterLink_Nodes(u32 n, u32 *value);

// 0x43b050: +0x14 of the first link whose +0x24 is key, else 0.
u32 __cdecl CoasterLink_Find(u32 key);

// 0x43e0b0: coaster index's +0x100 and +0x104 to *a and *b.
void __cdecl Coaster_GetPair(u32 index, u32 *a, u32 *b);

// 0x4386d0: the position (+0x30, three floats copied as they are) of the entry *slot of
// ride index's (table 0x7a4610) list (+8 -> +0x28 -> +4: 0x14-byte entries, each pointing
// at its object from +4), or zeros when it has none.
void __cdecl Ride_EntryPosition(u32 index, const u32 *slot, float *out);

// 0x434000: the first entry, over the links' chains (+0x4c, next +0x28), whose +8 is the
// tile key x + y * the map width (0x7a0854 -> +0x6c -> +0x18) and whose byte +4 has neither
// 1 nor 4 set, else 0; the key to *key when key is not null.
u8 *__cdecl CoasterLinks_FindTile(u32 x, u32 y, u32 *key);

// 0x444c00: whether the current link (0x790fec) has type (+4) flag 2 and its end piece is
// marked 0x80 (+8): the +0x60 piece when +0x3c has 1 (then only with 2 too), else the
// +0x58 one.
u32 __cdecl CoasterLink_EndMarked();

// 0x442d10: when CoasterLink_EndMarked, sets 0x79c664 to 1 and flag 0x40 of 0x79c658.
void __cdecl CoasterLink_CheckEnd();

// 0x443ee0: the first car, over the current link's (0x790fec) trains (+0x4c, next +0x28;
// cars: count +0x30, pointers +0x34) not flagged 1 (+4), whose ride (0x7a4610[car+0x70]) is
// ride, else 0; 0 at once when the link's +0x54 is 0.
u8 *__cdecl CoasterLink_FindCarOnRide(u32 ride);

// 0x442cd0: unless 0x79c658 has flag 0x40, when the current link (0x790fec) has type (+4)
// flag 2, sets 0x79c664 to 2 and that flag; returns 1.
u32 __cdecl CoasterLink_BeginEnd();

// 0x436820: bit 18 of +4 of entry index of the table at 0x7ae6a8.
u32 __cdecl Table7ae6a8_Bit18(u32 index);

// 0x436840: bit 18 of ride index's (table 0x7a4610) +4.
u32 __cdecl Ride_Bit18(u32 index);

// 0x436860: the first link of the list at 0x790fe0 whose +0x10 is id, else 0.
u8 *__cdecl CoasterLink_ById(u32 id);

// 0x436880: +0x124 -> +0x34 -> +0xc of link id when it has flag 0x800 (+0), else 0.
u32 __cdecl CoasterLink_IdValue(u32 id);

// 0x4368c0: sets +0x28 of link id, if any.
void __cdecl CoasterLink_SetById(u32 id, u32 value);
}
