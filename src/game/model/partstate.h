// A model instance's part states: reimplementation of 0x44a640, 0x44a870-0x44b13e and 0x44b4c0.
//
// The instance's model (+4) has +0x48 parts of 0x14 bytes at +0x7c, each with a face +0x46
// past its index (the first +0x44 faces at +0x70, 0xa0 bytes each, the rest at +0x74, 0x58
// bytes), and a node tree from +0x78 (+0 flags, +8 next sibling, +0xc first child, +0x50 the
// node's face index). The instance's +0x28 is then a PartStates block, with one 0x14-byte
// entry per part (+0 flags, +4 its 0x40-byte slot, a matrix 0x44ab30 fills walking the node
// tree, +0xc a sprite script, -1 for none).
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

// 0x44ab30: when the instance (flag 1) and its states (flag 1) are awake, the part
// matrices: PartStates_UpdateNode from the model's root (+0x78) and the identity.
void __cdecl PartStates_Update(u8 *instance);

// 0x44aae0: frees them.
void __cdecl PartStates_Free(u8 *instance);

// 0x44b4c0: one active part fewer (none left: flag 4 cleared); the part's entry loses flag 2
// and its script (SpriteScript_Kill).
void __cdecl PartStates_Release(u8 *instance, u32 part);

// A part's attachment (its +0xc): {u16 face of the host node, u16 back to its angles, then
// floats t, s, lift at +4}; the angles are three floats (x, y, z) at attach - back.

// 0x44b040: the point of face (a, b, c) at t, s (out = lerp(lerp(a, b, t), c, s)), copied
// to onFace before it is lifted by lift along normal.
void __cdecl Face_AttachPoint(const float *a, const float *b, const float *c, const float *normal,
                              const u8 *attach, float *onFace, float *out);

// 0x44a640: m's rotation (rows 0-2, the rest untouched) facing from -> to with up: rows
// up x n, up, n for n = normalize(to - from) (NaN n.x, as for from == to: n is up rotated to
// (up.z, up.x, up.y)), then turned by the attachment's angles (z, then y, then x).
void __cdecl Matrix_AttachAim(const float *from, const float *to, const float *up, const u8 *attach,
                              float *m);

// 0x44ab90: m is node's parent's matrix (by value; parent points at it). Animated nodes
// (0x20000000) of a part whose leaf entry (0x20) has none of 0xe set are skipped; the rest
// take m = node's matrix (+0x10) * m. A part with 0x40040 on a host node (+4) with 0x200000
// is then moved to its attachment on the host's face (in parent's space; aimed too with
// 0x40000), and its entry's slot gets m. Nodes with 0x10000000 and 0x10000 get their normals
// (Node_UpdateNormals); then each child without 0x40000000, from m.
struct Matrix44 {
    float m[16];
};
void __cdecl PartStates_UpdateNode(u8 *node, const u8 *model, Matrix44 m, const float *parent, u8 *entries);
}
