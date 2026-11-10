// The DCT video's macroblock: six blocks, each decoded or copied from the last frame, into the frame. Reimplementation of 0x676e40; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls the bench records (the ports by default; their own benches have them).
struct DctMacroblockEnv {
    s32(__cdecl *decode)();                                                        // DctVlc_DecodeBlock
    void(__cdecl *idct)(s32 *out, s32 pitch);                                      // DctIdct_Block
    void(__cdecl *bytes)(const u32 *luma, const u32 *chroma, u32 *out, s32 pitch); // TqiVideo_BlockBytes32
};
void DctMacroblock_SetEnv(const DctMacroblockEnv *env);

extern "C" {

// 0x676e40 (fastcall (reference, out; pitch in bytes), pops 4). half = pitch >> 1 (signed). With a last frame [0xfaac60], by the bit reader's top bits: both clear: 2 bits dropped, nothing copied; the top one set: 1 dropped, all copied; else 8 dropped and the top byte says which blocks are copied; and when
// any: two codes (DctVlc_ReadCode) x and y move reference on by (y * half + (x >> 1)) dwords and x & 1 is the copies' swap. Then the four luma blocks 0xfaa860, 0xfaa880, 0xfaaa60, 0xfaaa80 (mode bits 0-3) and the two chroma ones 0xfaa660, 0xfaa760 (bits 4, 5): a copied one RefBlock_Copy16 (reference + 0x10 for
// the odd ones, + half << 5 for the lower two) or RefBlock_Copy8 (swapped 1 and 0) with half and a third code - 0x40 as its bias; any other decoded (DctVlc_DecodeBlock), 1 its DC filled (DctVlc_FillDc), else DctIdct_Block, pitch 0x10 or 8. At the end TqiVideo_BlockBytes32 into out, half * 4.
void __fastcall DctVideo_DecodeMacroblock(u8 *reference, u8 *out, s32 pitch);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32[64], gDctmacroblock_faa660, 0x00faa660, 0x100);
GAME_DATA(u32[64], gDctmacroblock_faa760, 0x00faa760, 0x100);
GAME_DATA(u32[256], gDctmacroblock_faa860, 0x00faa860, 0x400);
