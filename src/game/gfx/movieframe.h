// The video library's frame buffers and block frames. Reimplementation of 0x6777d0 and 0x677310; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's heap and recorder).
struct MovieFrameEnv {
    void *(__cdecl *alloc)(u32 size);                            // malloc (0x67dcd0)
    void(__cdecl *refpack)(const u8 *src, u8 *dst, s32 decode);  // RefPack_DecodeIf
};
void MovieFrame_SetEnv(const MovieFrameEnv *env);

extern "C" {

// 0x6777d0 (fastcall (player, data)): the buffers +0x648, +0x64c, +0x654 and +0x650 freed where set and made again, by the settings' (player's dword +0) +0x60 and +0x64: both frames their product up to a multiple of 16, +0x654
// +0x60 * 8 bytes, +0x650 a dword per 16 bytes of the frame (signed). Any failed: 0 (the others kept). Else +0x654 has +0x60 * 2 dwords, -(+0x60 * +0x64) then rising by +0x64, the first frame is what
// RefPack_DecodeIf makes of data + (+0x62c + 4) * 3, and the answer 1.
u32 __fastcall MovieFrame_Allocate(u8 *player, const u8 *data);

// 0x677310 (fastcall (player, data)): one frame. +0x648 and +0x64c swapped; four words of data to +0x630 (the moved cells), +0x634 (the plain blocks), +0x638 (the four-colour blocks), +0x63c (the index bits). The cell buffers +0x65c
// (8 a cell) and +0x660 (4 a cell), their capacity +0x640, made again when too small; the block buffer +0x658 (16 a block), capacity +0x644, the same. A failed one: 0, with the frame's fields set. From data + 0xc: the cells' two fields
// of 10 bits each (MoviePlayer_UnpackFields) and +0x660[i] = the row offset +0x654[+0x60 + y] + x; the plain blocks copied to +0x658, then the four-colour ones expanded after them (MoviePlayer_ExpandBlocks), the indices
// (MoviePlayer_UnpackIndices, when count * bits + 0x1f has a bit over 0x1f; count (+0x60 * +0x64) >> 4) into +0x650, MoviePlayer_DrawBlocks (+0x650, +0x64c, +0x648, +0x630, +0x660, +0x658, +0x64, +0x64 >> 2, +0x60 >> 2). The answer 1.
u32 __fastcall MovieFrame_Decode(u8 *player, const u8 *data);
}
