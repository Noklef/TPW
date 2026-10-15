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
}
