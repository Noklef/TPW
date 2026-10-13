// UV animation of .md2 motion tracks (channel 0x10000): reimplementation of 0x4745c0.
#pragma once

#include "../../../re/types.h"

// The track's UV block (track +0x2c; see tools/md2motion.py).
struct UvRange {
    u16 first;  // first key in frames[] / uvs[]
    u16 keys;
};
struct UvTrack {
    u32 count;               // UVs animated: the node's first count UVs
    const UvRange *ranges;   // per UV
    u32 unknown08;
    const u16 *frames;       // key frames
    const float *uvs;        // (u, v) per key
};

// Sets (add == 0) or adds to the node's UVs, stored as the node's 32-byte blocks
// {u[4], v[4]} (node +0x68), the key pair around frame lerped. The pair is found by the
// whole frame, (u32)(s64)frame: the first key above it and the one before. Past the last
// key the last pair is used at t = 1; a frame before the first key reads the key before it,
// as the original does.
void Motion_UVsAt(const UvTrack *track, float frame, float *blocks, s32 add);

extern "C" {
// 0x4745c0: (motion state, frame at +0x20; the 64-byte track, UVs at +0x2c and the driven
// node at +0x3c; add).
void __cdecl Motion_ApplyUVs(const void *state, const void *track, s32 add);
}
