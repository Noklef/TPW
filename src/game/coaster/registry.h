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

// 0x436150: the first entry, over the links' chains (+0x4c, next +0x28), whose +8 is tile
// and whose byte +4 has neither 1 nor 4 set, else 0.
u8 *__cdecl CoasterLinks_FindKey(u32 tile);

// 0x434000: CoasterLinks_FindKey for the tile key x + y * the map width (0x7a0854 -> +0x6c
// -> +0x18), the key to *key when key is not null.
u8 *__cdecl CoasterLinks_FindTile(u32 x, u32 y, u32 *key);

// 0x433f70 (cdecl; x, z, key, each may be null; name unconfirmed): CoasterLinks_FindTile for the cell of the point at 0x7a1c70 (x =
// __ftol(+0 0.1), z = __ftol(+8 0.1)), the cell to *x / *z as well.
u8 *__cdecl CoasterLinks_FindAtFocus(s32 *x, s32 *z, u32 *key);

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

// 0x444c50: when the current link (0x790fec) has type (+4) flag 2, sets 0x79c664 to 1 and
// flag 0x40 of 0x79c658.
void __cdecl CoasterLink_SetEnd();

// 0x444c80: when the current link has type flag 2, sets 0x79c664 to 0 and clears flag 0x40
// of 0x79c658.
void __cdecl CoasterLink_ClearEnd();

// 0x4418d0: the selected car (0x79c640).
u8 *__cdecl Coaster_SelectedCar();

// 0x4418e0: sets the selected car.
void __cdecl Coaster_SelectCar(u8 *car);

// 0x4418f0: CoasterLink_ById(object+0x20), 0 for no object.
u8 *__cdecl Object_CoasterLink(const u8 *object);

// 0x441910: whether object's link has flag 1 but not 2 at +0x3c.
u32 __cdecl Object_LinkOpen(const u8 *object);

// 0x441940: whether object's link holds no contacts (+0x140).
u32 __cdecl Object_LinkBare(const u8 *object);

// 0x434710: copies 0x79221c, 0x790fd8, 0x79104c and 0x791cf8 to *a, *b, *c and *d.
void __cdecl Coasters_GetState(u32 *a, u32 *b, u32 *c, u32 *d);

// 0x434f10 (cdecl; name unconfirmed): those four as the cells within reach of the edited piece: the piece is, for the current link
// (0x790fec) with flag 2 (byte +0x3c), the selection's (0x79c6a4) +0x68 - or its +0x64 when that has flag 0x10000 (+8) - with edit
// flag 0x100 (0x79c658), else the link's +0x60; without flag 2, the selection's +0x68 with flag 0x100, else the first of the link's
// +0x58 chain (through +0x68) without flag 0x2000 (none: the piece is null, and its +0x100 read faults, as the original). Its cell
// (+0x100 -> +0xc, +0x10) +/- reach, reach = __ftol(fsqrt of the dword +0xa0 of the link's type (+4) at +0x1c8 with edit flag 0x40,
// else +0xa8)) + 1; the lows raised to 0x7a01bc, the highs held below 0x7a01bc + 0x7a0314 and 0x7a029c + 0x7a0374 (all signed).
// sic: the z low is raised to 0x7a01bc, the x origin, as the x low is.
void __cdecl CoasterEdit_SetBounds();

// 0x4479a0 / 0x447a10 (cdecl; names unconfirmed): the edit's two tools (0x79c644 = 1 / 2), each recorded (ActionRec_Write of the
// recorder 0x785a08, code 0x37 / 0x38) and ending in CoasterEdit_SetBounds; flag 0x2000 of the current link (0x790fec, +0; there
// must be one) cleared first. Tool 1 clears edit flag 0x40 (0x79c658) when tool 2 was on with 0x79c664 2 and the link's type (+4)
// has flag 2 (its first byte); 0x79c664 = 0. Tool 2, for a link whose type has flag 2: with the end piece flagged 0x80 (+8; the
// link's +0x58 without its flag 1 at +0x3c, else its +0x60 when it has flag 2) 0x79c664 = 1, else, unless flag 0x40 is on, 0x79c664
// = 2; either way flag 0x40 set.
void __cdecl CoasterEdit_Tool1();

void __cdecl CoasterEdit_Tool2();

// 0x4445b0 (cdecl; tool; name unconfirmed): CoasterEdit_Tool2 for tool 2; for any other, CoasterEdit_Tool1 with 0x79c644 = tool.
// (Tool1 and Tool2 are this with 1 and 2.)
void __cdecl CoasterEdit_SetTool(u32 tool);

// 0x447990: flag 1 of 0x79c658.
u32 __cdecl CoasterEdit_Flag1();

// 0x430ac0: sets the current link (0x790fec).
void __cdecl CoasterLink_SetCurrent(u8 *link);

// 0x445910: the edit tools on offer for the current link (0x790fec, which must be set):
// 0x1fc, plus 1 unless CoasterLink_EndMarked; 2 when its type (+4) has flag 0x80, and 0x200
// too when that has 0x100; less 1 and 2 when +0x3c has flag 1 but not 2, else less 0x40.
// Stored to *out (when given) and 0x79c6e0, and returned.
// sic: three paths by 0x79c638's flags all come to the same.
u32 __cdecl CoasterEdit_Tools(u32 *out);

// 0x441970: whether object's link (Object_CoasterLink) is another open one: it has no +0x140,
// flag 1 but not 2 in +0x3c, and is not the current link (0x790fec).
u32 __cdecl Object_LinkOtherOpen(const u8 *object);

// 0x43e0e0: object's link's (Object_CoasterLink, which must be found) +0x10c, +0x110, +0x114
// and +0x108, to a, b, c and d.
void __cdecl Object_LinkParams(const u8 *object, u32 *a, u32 *b, u32 *c, u32 *d);

// 0x43e120: 0x792404 = 1.0f.
void __cdecl Coasters_ResetUnit();

// 0x43e140: 0x7925e0 = 1.0f / 8388608.0f (constants 0x6fe108 / 0x6fe0d8), one float ulp at 1.
void __cdecl Coasters_InitEpsilon();

// 0x43e980: 0x792400 = links, 0x7925dc = links zeroed 0x1c-byte blocks and 0x792408 = nodes
// zeroed 0x10-byte blocks (callers pass 0x790fd0 and 0x791cfc).
void __cdecl Coasters_AllocTables(u32 nodes, u32 links);

// 0x435500: the tables for 0x790fd0 nodes and 0x791cfc links, and the id table (0x790b04 used,
// 0x790b00 holes, 0x791ce0 next) started.
void __cdecl Coasters_InitTables();

// 0x441840: stops the coaster's flashing car (+0x144; its ride, 0x7a4610[car+0x70], is
// unflashed when it has flag 0x4000000) and flashes another: which & 1 the car at +0x58
// (+0x60 with the coaster's 2 at +0x3c), else which & 2 the one at +0x54 (+0x64), its ride
// flashed unless it already has 0x4000000, and kept at +0x144.
void __cdecl Coaster_FlashCar(u8 *coaster, u32 which);

// 0x463ab0: claims ride index (table 0x7a4610) or, when index is 0, the first of 1..0x7a3ebc +
// 0x7a4084 not yet claimed (+4 flag 0x10000) whose type (its +0 found in 0x7ae6a8 from
// 0x7a439c down) is type and +0xe4 key, and when desc is given whose +0xe8 / +0xec / +8 -> +0x10
// are desc[0..2] and, when desc[3] is set, either the object definition of type's +0xf8 (set
// 0x7890a0) has +0x18 desc[3] or, with no definition, +0xf0 / +0xf4 are a / b (desc[3]'s two
// words when 0). Sets value on its coaster link (by index) when it has +4 flag 0x40000, flags it
// claimed and returns index, else 0. The fifth argument is unused.
u32 __cdecl Ride_Claim(u32 index, u32 type, u32 key, const u32 *desc, u32, u32 value, u32 a, u32 b);
}
