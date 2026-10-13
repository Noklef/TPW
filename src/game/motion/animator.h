// A model's animator and its motion layers: reimplementation of Animator_ResetLayers
// (0x4726d0), the layer helpers at 0x472bc0-0x472f40, Layer_Play (0x472f60),
// Animator_Play (0x4732a0), Motion_UpdateClocks (0x473440), Animator_IsIdle (0x473490),
// Animator_RefreshNodes (0x473550) and Layer_Update (0x4735d0).
#pragma once

#include "../../../re/types.h"

// The animator:
//   +0     [+8]+4: the rest model (the pose channels are reset to)
//   +4     flags: 8 shows every node (and wakes the holder to play, puts it to sleep when a
//          motion ends), 0x10 holds the last frame, 0x4000 sleeps with +0xc0/+0xc4,
//          0x8000 posed by hand (no clock), 0x100000 acts as gMotionFlags 0x1 (see
//          Motion_ResetChannelsWith), 0x400000 applies every layer this tick, 0x40000000
//          nodes to refresh (Animator_RefreshNodes)
//   +8     the holder of the animated model (+4); its flag 4 (byte +0) names the listed
//          nodes of a block by id (node +0x50) instead of by index
//   +0xe   u16 layer count, +0x10 the layers (0x38 bytes each), +0x14 how many are playing
//   +0x18  12 motion banks of 12 bytes: flags byte (& 3: loaded), u32 count +4, entries +8
//          (8 bytes; the block at +4). Bank 0xc is none, 0 the base pose, 5 the idle motions.
//   +0xa8  nonzero once it has motions to update
// The layer, which is the state Track_Apply plays:
//   +0     flags: 1 loops, 2 stopped, 4 run to the end (0x10 is set with it on a reset, to
//          apply it once more), 8 bank 0 ended, 0x20 off, 0x40 timed by gMotionRealTime
//          (0x7b4974) instead of gMotionTime (0x7b496c)
//   +4     bank, +8 entry; +0xc speed
//   +0x10  start time, +0x14 time, +0x18 start time again (u32 ms)
//   +0x1c  length, +0x20 frame (floats; 30 frames a second at speed 1)
//   +0x24  the motion queued while it plays: bank (0xc none), +0x28 entry, +0x2c mode,
//          +0x30 speed
//   +0x34  the block it last played

// What the layer functions read of the game, so that they can run outside it.
struct AnimatorEnv {
    u32 motionFlags;                              // gMotionFlags' value
    u32 *trackFlags;                              // gTrackFlags (0x7aecd4)
    u32 time, realTime;                           // gMotionTime, gMotionRealTime
    void(__cdecl *wake)(u8 *holder, u32 value, s32 on);  // 0x45bd30
    // Track_Apply (0x471860) and Motion_ApplyMaterial (0x470c90), for Layer_UpdateWith
    s32(__cdecl *track)(const void *layer, u8 *model, u32 block, u32 *node, const u8 *track);
    void(__cdecl *material)(void *model, const void *block, const void *layer);
    void(__cdecl *idle)(u8 *holder);                    // 0x45bdd0
    void(__cdecl *idleAt)(u8 *holder, u32 a, u32 b);    // 0x45be80
};

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

// 0x472f60 (animator, bank, entry, layer, mode, speed): switch the layer to the bank's
// entry. Mode 1 sets layer flag 1 (else clears it), mode 4 flag 0x20 (off); mode 8 skips
// Motion_MarkNodes, and modes 4 and 8 the reset of the channels the last motion drove
// (Motion_ResetChannels against the new block). Banks 0xd and 0xe stop the layer where it
// is (flag 2, or 0x14 and running to the end); an unloaded bank or 0xc empties it.
// Otherwise the layer is (re)started: from the start, or the frames it ran past the end
// of its last motion into the new one; unless animator flag 0x8000 the frame is then
// recomputed from the times. Unless animator flag 0x1000000, Animator_CountLayers.
// Returns the new motion's length in ms (0 for none).
u32 Layer_PlayWith(const AnimatorEnv &env, u8 *animator, u32 bank, u32 entry, u8 *layer, u32 mode, float speed);

// 0x4732a0 (animator, bank, entry, mode, speed, layer index): with animator flag 8 and
// holder flag 1 clear, wake the holder (0x45bd30 with animator +0xc8) unless the bank is
// 0xc. While the layer plays a motion to its end (no flag 2 or 4) the new one is queued
// unless mode 2 (bank 0xd and 0xe never queue), and the ms until it would end are
// returned: what is left plus its length (1000 for none). Otherwise mode 2 empties the
// queue and the layer plays it now (Layer_PlayWith), returning its length or 1000.
u32 Animator_PlayWith(const AnimatorEnv &env, u8 *animator, u32 bank, u32 entry, u32 mode, float speed,
                      u32 index);

// 0x473440 with the game clock's read given: gMotionTime (0x7b496c) becomes now, and
// gMotionTimeStep (0x7b497c) the seconds since its last value.
void Motion_UpdateClocksWith(u32 *time, float *step, u32 now);

// 0x473490 with the clocks given: 1 when every layer has played its motion out (or has
// none; and for a null animator), 0 without motions (+0xa8) or when posed by hand (0x8000).
// The original never steps to the next layer, so it is the first layer that is looked at
// (and has its time and frame updated) once per layer.
s32 Animator_IsIdleWith(u32 time, u32 realTime, u8 *animator);

// 0x473550 with the flags word given (0x8bd508): nodes of the holder's model flagged 0x100
// lose 0x800, and if there were any the word's top bit is set; animator flag 0x40000000 is
// cleared.
void Animator_RefreshNodesWith(u8 *animator, u32 *sceneFlags);

// 0x4735d0 (animator, layer, block): one tick of the layer. Posed by hand (animator flag
// 0x8000) its bank entry (or the block, when the bank has no entries) is applied as it
// stands. Otherwise the time and frame follow the clock, and at the end of the motion:
//  - the queued motion plays (mode | 8 without a block);
//  - with no queue and animator flags 8 and 0x10 clear: a looping layer (flag 1) plays it
//    again, at the queued speed if nonzero, and the others stop at the end (bank 0xe);
//  - with flag 8 the layer is emptied and the holder, if awake, put to sleep;
//  - with flag 0x10 the last frame is held: the animator gets 0x40400000, and after the
//    apply the layer is emptied (mode 4).
// A layer flagged 8 (its bank 0 motion ended) that is now on another bank loses the flag and
// sets animator flag 0x40000000. The bank
// entry is then applied (and kept at +0x34) when animator flag 0x400000 is set, or with a
// block unless the layer is run to the end (flag 4 without 0x10).
void Layer_UpdateWith(const AnimatorEnv &env, u8 *animator, u8 *layer, const u8 *block);

extern "C" {
// 0x473490, 0x473550, 0x4735d0 on the game's globals.
s32 __cdecl Animator_IsIdle(u8 *animator);
void __cdecl Animator_RefreshNodes(u8 *animator);
void __cdecl Layer_Update(u8 *animator, u8 *layer, const u8 *block);

// 0x472f60, 0x4732a0 on the game's globals.
u32 __cdecl Layer_Play(u8 *animator, u32 bank, u32 entry, u8 *layer, u32 mode, float speed);
u32 __cdecl Animator_Play(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index);

// 0x473270 (animator, layer index): empty the layer's queue; 1 if there was one.
s32 __cdecl Layer_ClearQueue(u8 *animator, u32 index);

// 0x473440: gMotionTime from 0x402d70, then gMotionRealTime from 0x402d80 (methods of the
// object at 0x785970).
void __cdecl Motion_UpdateClocks();

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
