// Applying .md2 motion to a model's nodes at the state's frame: reimplementation of
// Motion_Apply (0x4721f0) and Track_Apply (0x471860), which drive everything else in
// src/game/motion, and of Motion_CopyVertices (0x472270) and Motion_ResetChannels (0x472310).
#pragma once

#include "../../../re/types.h"
#include "../math/sintable.h"

// The globals Track_Apply reads and writes, so that it can run outside the game too.
struct MotionEnv {
    u32 *rotationKeyCount;  // gRotationKeyCount (0x7b4988), set for Motion_RotationAt
    u32 *curvePointCount;   // gCurvePointCount (0x7b49e8), set for the Curve_* functions
    const u32 *motionFlags; // gMotionFlags (0x7b4980): 0x2 lerps rotations, 0x10 (see below)
    SinTable sines;
};

// The track (64 bytes; see tools/md2motion.py), by flag (+4):
//   0x20000  visibility keys (s16, |key| = frame, key <= 0 hides: node flag 0x10), +0x16 count, +0x30
//   0x1000   vertex animation (+0x28): packed (Motion_UnpackVertices) or, with 0x4000, unpacked
//            {remap (or null), vertices a frame, float[3] vertices}
//   0x8      rotation keys (RotationKey, +0x1c, count +0x10), eased through 8-byte tables (+0x34)
//   0x80     scale keys (16 bytes: frame, x, y, z; +0x20, count +0x12)
//   0x1      path (+0x18): {flags, u16 +4 points, u16 +6 keys, points +8, keys +0xc}; with 0x200
//            (and no 0x1), a ride path: the model's paths (+0xac, 16 bytes each) picked by the
//            node's object (+4, flag 8; path at +0x52), timed by the range at +0x24 {start,
//            length, ?, per-frame percent table}
//   0x400    the node faces along the path (with no rotation track, its y axis is reset first)
//   0x10000  UV animation (Motion_ApplyUVs)
// The model's flag 4 (+0x30) adds the results onto the node instead of setting them.
// Returns 0 (doing nothing) when the frame (+0x20) is past the state's end (+0x1c).
s32 Track_ApplyWith(const MotionEnv &env, const void *state, u8 *model, u32 *node, const u8 *track);

// Every track of the block (u16 count +0x12, 64-byte tracks at +0x2c, each driving the node
// at track +0x3c) on the holder's model (+4), then, with block flag 2 and without
// gMotionFlags 0x8, the block's material animation. Always returns 1.
s32 Motion_ApplyWith(const MotionEnv &env, const void *state, const void *holder, const u8 *block);

// A model's node by index (u16 +0x42 in all): the first u16 +0x44 are 0xa0-byte nodes at
// +0x70, the rest 0x58-byte ones at +0x74.
u8 *Model_Node(const u8 *model, u32 index);

// Reset a node's channels that the track flags name from the rest model's node: the
// 3x4 matrix (0x289), the positions ((to's u16 +0x58 + 3) / 4 blocks of 48 bytes, when both
// have some) and box (0x1000), the UVs (to's u16 +0x5e, 32-byte blocks; 0x10000).
void Motion_ResetNode(u8 *to, const u8 *from, u32 flags);

// 0x472310 with the globals given (gMotionFlags' value; gTrackFlags, the per-node array
// that 0x7aecd4 points to). The animator: rest model at [[+0]+8]+4, flags +4, target model
// at [+8]+4.
//  - animator flag 8 shows every node (clears 0x10); flag 0x100000 otherwise acts as
//    gMotionFlags 0x1;
//  - the block's listed nodes (u16 count +0x1a, u16 indices +0x38) and those its tracks
//    drive are shown unless flagged 0x80000000, and the driven ones lose 0xf40000 (adding,
//    target model flag 4: only 0x800000);
//  - gTrackFlags[node] = the flags of the block's track on it (0 for the others);
//  - unless gMotionFlags 0x1 without adding: the channels prev's tracks drove are dropped,
//    and the rest are reset from the rest model (Motion_ResetNode);
//  - with block flag 2 and without gMotionFlags 0x8, the target's materials flagged 0x40
//    are marked changed (+1 |= 4) with value (+2) 0.
void Motion_ResetChannelsWith(u32 motionFlags, u32 *trackFlags, const void *animator, const u8 *block,
                              const u8 *prev);

extern "C" {
// 0x472310 (animator, block, prev block or null) on the game's globals.
void __cdecl Motion_ResetChannels(const void *animator, const u8 *block, const u8 *prev);

// 0x471860 (state, model, unused, node, track); the original passes [unused + 8] on to
// Motion_UnpackVertices, which ignores it, and uses its stack slot for a key index.
s32 __cdecl Track_Apply(const void *state, u8 *model, u32 unused, u32 *node, const u8 *track);

// 0x4721f0 (state, holder, block): Motion_ApplyWith on the game's globals; a null block does
// nothing. Motion_Apply passes the block as Track_Apply's unused argument.
s32 __cdecl Motion_Apply(const void *state, const void *holder, const u8 *block);

// 0x472270 (from, to, all): copy the vertex positions ((count + 3) / 4 blocks of 48 bytes)
// and bounding box of each node of from's model into to's (models at holder +4; nodes are
// 0xa0 bytes at model +0x70, count u16 at to's model +0x44). Nodes without vertices are
// skipped, and unless all is set so are those without flag 0x200000; the positions are
// copied only when both sides have some, the box always.
void __cdecl Motion_CopyVertices(const void *from, const void *to, s32 all);
}
