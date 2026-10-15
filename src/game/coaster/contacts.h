// Contacts between the nodes of an owner's list (owner+0x54, linked by +0x64): each node
// keeps up to 8 (+0xb4, count +0xb0), both ways round; flag 0x400000 (+8) marks one that
// had more than fit, 0x200000 one that lost a contact while so marked. owner+0x140 counts
// the contacts held.
#pragma once

#include "../../../re/types.h"

extern "C" {

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
}
