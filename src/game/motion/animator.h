// A model's animator and its motion layers: reimplementation of Animator_ResetLayers
// (0x4726d0) and the layer helpers at 0x472bc0-0x472f40.
#pragma once

#include "../../../re/types.h"

// The animator:
//   +0     [+8]+4: the rest model (the pose channels are reset to)
//   +4     flags: 8 shows every node, 0x100000 acts as gMotionFlags 0x1 (see
//          Motion_ResetChannelsWith)
//   +8     the holder of the animated model (+4); its flag 4 (byte +0) names the listed
//          nodes of a block by id (node +0x50) instead of by index
//   +0xe   u16 layer count, +0x10 the layers (0x38 bytes each), +0x14 how many are playing
//   +0x18  12 motion banks of 12 bytes: flags byte (& 3: loaded), u32 count +4, entries +8
//          (8 bytes; the block at +4). Bank 0xc is none.
// The layer, which is the state Track_Apply plays:
//   +0     flags: 0x4 (0x10 is set with it on a reset), 0x20 off, 0x40 timed by
//          gMotionRealTime (0x7b4974) instead of gMotionTime (0x7b496c)
//   +4     bank, +8 entry; +0xc speed
//   +0x10  start time, +0x14 time, +0x18 start time again (u32 ms)
//   +0x1c  length, +0x20 frame (floats; 30 frames a second at speed 1)
//   +0x34  the block it last played

// 0x4726d0 with the globals given (gMotionFlags' value; gTrackFlags, the per-node array
// that 0x7aecd4 points to). Motion_ResetChannelsWith for every layer that is not off, its
// last block against its bank entry's block:
//  - animator flag 8 shows every node; flag 0x100000 otherwise acts as gMotionFlags 0x1;
//  - for each layer: the last block's listed nodes and driven nodes are shown (unless
//    flagged 0x80000000), the driven ones lose 0xf40000 (adding, model flag 4: only
//    0x800000), and gTrackFlags[node] = the flags of the track on it (0 for the rest);
//    then the flags of the bank entry's tracks are dropped from gTrackFlags (unless
//    gMotionFlags 0x1 without adding);
//  - for each layer with a loaded bank: unless gMotionFlags 0x1 without adding, the
//    last block's nodes are reset (Motion_ResetNode); with the block's flag 2, without
//    gMotionFlags 0x8, and only once, materials flagged 0x40 with a value (+2) are marked
//    changed (+1 |= 4) with value 0; layer flag 4 sets 0x10; Motion_MarkNodes(the entry's
//    block, the holder).
void Animator_ResetLayersWith(u32 motionFlags, u32 *trackFlags, u8 *animator);

// 0x472bc0 with the clocks given.
void Layer_StartWith(u32 time, u32 realTime, u8 *layer, const u8 *block, s32 restart, float frame);

extern "C" {
// 0x4726d0 (animator) on the game's globals.
void __cdecl Animator_ResetLayers(u8 *animator);

// 0x472bc0 (layer, block, restart, frame): the length becomes the block's (u32 +8 - u32
// +4); restarting, all three times become the clock's; otherwise the start goes back so
// that the time is frame (at most the length) frames in: frame * 33.333332 / speed ms. A
// null block zeroes the length, the frame and the start.
void __cdecl Layer_Start(u8 *layer, const u8 *block, s32 restart, float frame);

// 0x472c60 (layer, now or null): the time (and +0x18) becomes *now, and the frame
// (time - start) * speed * 0.03.
void __cdecl Layer_Advance(u8 *layer, const u32 *now);

// 0x472cb0 (layer, block): the length becomes the block's and the frame
// (time - start) * 0.03 / speed; a null block zeroes the length, the frame and the start.
void __cdecl Layer_SetBlock(u8 *layer, const u8 *block);

// 0x472d10 (&animator, bank, entry): 1 if the bank is loaded and has the entry.
s32 __cdecl Animator_HasMotion(u8 *const *animator, u32 bank, u32 entry);

// 0x472d50: the time left, (length - frame) * 33.333332 ms, as the x87 holds it.
double __cdecl Layer_TimeLeft(const u8 *layer);

// 0x472d70 (block, holder): the nodes the block's tracks drive are flagged 0x40000, and
// 0x200000 for vertices (track flag 0x1000), 0x400000 for UVs (0x10000), 0x100000 for the
// matrix (0x289); the block's listed nodes are hidden (0x10, unless flagged 0x80000000).
// With the holder's flag 4 the listed values are node ids (u16 +0x50), and the first node
// with each is hidden.
void __cdecl Motion_MarkNodes(const u8 *block, const u8 *holder);

// 0x472ee0: count the layers with a bank (+0x14); the model's flag 0x80000 is set when
// there are any, cleared when not.
void __cdecl Animator_CountLayers(u8 *animator);

// 0x472f40: one layer playing (+0x14 = 1), and the model's flag 0x80000 set.
void __cdecl Animator_SetPlaying(u8 *animator);
}
