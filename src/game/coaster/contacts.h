// Contacts between the nodes of an owner's list (owner+0x54, linked by +0x64): each node
// keeps up to 8 (+0xb4, count +0xb0), both ways round; flag 0x400000 (+8) marks one that
// had more than fit, 0x200000 one that lost a contact while so marked. owner+0x140 counts
// the contacts held.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// Contacts_TestPair's calls go by the game's addresses; a table of replacements (tests/fncmp/contactspair.py gives them recorders).
void Contacts_SetCalls(const AddrCall *table, u32 count);

// 0x439c40 (cdecl; name unconfirmed): the span nodeA..nextA of owner1 against nodeB..nextB of owner2. With one owner, none of: the spans' next types the other's first (+0x100), a next type with flag 1 (+4) against a first node whose +0 is 0. Not with nodeB's flag 0x10000 or when the boxes (+0x98) do not overlap (a NaN does).
// The segments of both (Contacts_SegmentsInBox) in the boxes' common part, into the arrays at 0x792238 and 0x792230, all pairs: the globals 0x79223c, 0x792240, 0x7922dc, 0x792244 set for the test 0x43a2b0 and, for a pair overlapping that it answers non-zero, each node joins the other's contacts (up to 8, then flag 0x400000) and it ends.
void __cdecl Contacts_TestPair(u8 *owner1, u8 *owner2, u8 *nodeA, const u8 *nextA, u8 *nodeB, const u8 *nextB);

// 0x43ad50: drops all of node's contacts: from every node in the owner's list when node is
// marked 0x400000 (clearing the mark), else from each node it holds. Each node losing any
// closes up its list (zeroing the rest), and is marked 0x200000 when marked 0x400000; the
// owner's count drops by those and by node's own.
void __cdecl Contacts_Clear(u8 *owner, u8 *node);

// 0x43afa0: clears the contacts of every node in the owner's list, marking each 0x200000.
void __cdecl Contacts_ClearAll(u8 *owner);

// 0x43af70: Contacts_Clear for node, then for its partner (+0x68) if any.
// sic: node comes before owner here.
void __cdecl Contacts_ClearPair(u8 *node, u8 *owner);

// 0x4346f0: marks node, and its partner (+0x68) if any, 0x200000.
void __cdecl Contacts_MarkLostPair(u8 *node);

// 0x439920: boxes each node of the owner's list that has a next one: min (+0x98) and max
// (+0xa4) over the positions (+4) of the owner's points (+0x9c, 0x38 bytes each) from the
// node's index (+0xfc) to the next node's, padded by the +0x58 of the node's type (+0x100
// -> +0x18), else of the next node's, else 0.
// sic: a point compares below min when either is NaN, and then only min takes it.
void __cdecl Contacts_UpdateBounds(u8 *owner);

// 0x43a060: the segments between the owner's points (+0x9c, 0x38 bytes, position at +4)
// from node's index (+0xfc) up to next's that have an end in box (min xyz, max xyz) grown
// by the pad (as Contacts_UpdateBounds): each to out as {index, min xyz, max xyz} (0x1c
// bytes) over its two ends grown by the pad again, and their number to *count.
// sic: an end is in the box unless below min, NaN or above max, so a NaN max lets it in;
// and x's max takes end + pad where y and z take pad + end.
void __cdecl Contacts_SegmentsInBox(const u8 *owner, const u8 *node, const u8 *next, const float *box, u8 *out,
                                    u32 *count);
}

// 0x439ab0 (cdecl; name invented, unconfirmed): all the coaster's pairs tested, at x87 rounding toward zero (the control word kept and put back). The four arrays made for this: 0x792238, 0x792230 of +0xa0 * 0x1c bytes, 0x7922e8, 0x7922e4 of +0xa0 * 0x78, zeroed, freed at the end, in the other order. Each node of +0x54's chain by +0x64,
// the last not, with flag 0x200000 at +8: Contacts_Clear; without flag 0x10000 too, both flags 0x600000 go off and Contacts_TestPair with each node after the next that has its own next, and each of the chain +0x68 from its partner's +0x68.
extern "C" {
void __cdecl Contacts_TestAll(u8 *owner);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gContacts_792230, 0x00792230, 0x4);
GAME_DATA(u32, gContacts_792238, 0x00792238, 0x4);
GAME_DATA(u8 *, gContacts_79223c, 0x0079223c, 0x4);
GAME_DATA(u8 *, gContacts_792240, 0x00792240, 0x4);
GAME_DATA(u8 *, gContacts_792244, 0x00792244, 0x4);
GAME_DATA(u8 *, gContacts_7922dc, 0x007922dc, 0x4);
GAME_DATA(u32, gContacts_TestAll_7922e4, 0x007922e4, 0x4);
GAME_DATA(u32, gContacts_TestAll_7922e8, 0x007922e8, 0x4);
