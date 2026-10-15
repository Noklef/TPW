// A model instance's part states: reimplementation of 0x44a870-0x44aadb and 0x44b4c0.
//
// The instance's model (+4) has +0x48 parts of 0x14 bytes at +0x7c, each with a face +0x46
// past its index (the first +0x44 faces at +0x70, 0xa0 bytes each, the rest at +0x74, 0x58
// bytes), and a node tree from +0x78 (+0 flags, +8 next sibling, +0xc first child, +0x50 the
// node's face index). The instance's +0x28 is then a PartStates block, with one 0x14-byte
// entry per part (+0 flags, +4 its 0x40-byte slot, +0xc a sprite script, -1 for none).
#pragma once

#include "../../../re/types.h"

struct PartStates {
    u32 flags;     // 1: has animated parts, 4: some active, 8: a part has flag 2
    u8 *entries;   // 0x14 bytes per part
    u32 active;    // what 0x44b4c0 counts down
    u32 slots;     // how many 0x40-byte slots
    u8 *slotData;  // and the slots
};

extern "C" {

// 0x44a870: the instance's part states, for a model with parts. Parts with flag 0x10 or 0x20
// are animated (their face gets 0x20000000, the entry flag 1 (and 0x10 for a face with 0x400)
// and no script); faces of parts with 8 / 0x10000 get 0x40 / 0x20000, and a part with 0x40040
// sets 0x10000000 at its face's +4. One slot is made per face carrying 0x20000000, whether or
// not this set it (sic: the slots are then handed to the entries with flag 1, which can be
// more).
void __cdecl PartStates_Create(u8 *instance);

// 0x44aa10: marks, below node, the entries of animated parts' leaf nodes (0x20), and gives
// nodes with no animated part in their subtree 0x40000000; 1 if node's subtree has none.
u32 __cdecl PartStates_MarkNodes(u8 *instance, u8 *node, const u8 *model);

// 0x44aae0: frees them.
void __cdecl PartStates_Free(u8 *instance);

// 0x44b4c0: one active part fewer (none left: flag 4 cleared); the part's entry loses flag 2
// and its script (SpriteScript_Kill).
void __cdecl PartStates_Release(u8 *instance, u32 part);
}
