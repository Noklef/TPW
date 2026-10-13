// Key search for .md2 motion tracks: reimplementation of 0x470b60 and 0x470c90.
#pragma once

#include "../../../re/types.h"

// One entry of a model's material animation table: keys are u32 (frame | value << 16).
struct MaterialAnim {
    u16 material;  // index into the model's 8-byte material records (model +0x50)
    u16 keys;
    const u32 *key;
};

extern "C" {
// 0x470b60: the keys around the motion state's frame (state +0x20). Keys start with a u16
// frame; kind 1, 2 or 4 means 4-, 20- or 16-byte keys (anything else returns 0). i is the
// last key at or before the whole frame ((u32)(s64)frame); none means 0 and nothing is
// written. j is i + 1; at the last key, j = 0 with that key's frame (loop) or j = i with
// frame + 1. t = (frame - frame_i) / (frame_j - frame_i). Returns 1.
s32 __cdecl Motion_FindKeys(const void *state, const u16 *keys, u32 count, s32 kind, s32 loop, u32 *i, u32 *j,
                            float *t);

// 0x470c90: (model, table header: u16 count at +0x18, -> MaterialAnim at +0x30; state)
// sets each animated material's value (record +2) from the last key at or before the
// whole frame, flagging the record (+1 |= 4) when it changes.
void __cdecl Motion_ApplyMaterial(void *model, const void *table, const void *state);
}
