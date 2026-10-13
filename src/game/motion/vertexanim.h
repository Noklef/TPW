// Packed vertex animation of .md2 motion tracks (channel 0x1000 without 0x4000):
// reimplementation of Motion_UnpackVertices 0x4714a0 and its lerps 0x470e90 and 0x4711d0.
#pragma once

#include "../../../re/types.h"

// A vertex group of a packed vertex-animation block (20 bytes; the groups follow the
// block header). Keys are 10:10:10 signed ints (x in bits 0-9, y 10-19, z 20-29), count
// per frame, frame after frame; each component is value * scale + offset (block +0x20,
// +0x14).
struct PackedGroup {
    u16 unknown00;
    u16 count;          // vertices a frame
    const u16 *slots;   // position slot of each vertex (blocks of 4)
    const u16 *frames;  // key frames (read by Motion_UnpackVertices)
    const u32 *keys;
    u32 key;            // current key: lerps run from key to key + 1
};

extern "C" {
// 0x470e90: lerp each vertex of the group between its keys at t and set (add == 0) or add
// it into the position blocks {X[4], Y[4], Z[4]} (48 bytes each).
void __cdecl Motion_LerpPackedVertices(const void *block, float *positions, const PackedGroup *group, float t,
                                       s32 add);
// 0x4711d0: the same for the first two entries of the group, a bounding box: min - (scale
// + 0.25) into box[0..2], max + scale + 0.25 into box[3..5].
void __cdecl Motion_LerpPackedBox(const void *block, float *box, const PackedGroup *group, float t, s32 add);

// 0x4714a0: unpack a vertex-animation block into the node at the frame. The block: flags
// (byte +0; 2: group 1 holds raw keys, unpacked once), group count (u16 +2), groups (+0xc),
// offsets (+0x14), scales (+0x20). The node: flags (u32 +0), positions (+0x60), box (+0x78).
// Group 0 is the bounding box, the rest are vertices. Each group's key advances while the
// next key's frame is before the frame; t is the fraction between the two. Until the node
// has flag 0x800000 the keys restart at 0 and the raw group is written (the flag is set
// unless adding). Flag 0x10000 is always set. The fifth argument is unused.
void __cdecl Motion_UnpackVertices(u32 *node, const u8 *block, float frame, u32 unused, s32 add);
}
