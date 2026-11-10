// The movie player's DCT video frames of kinds 5, 6 and 7. Reimplementation of 0x6771a0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorder and stand-ins: their own benches have them).
struct MadVideoEnv {
    void *(__cdecl *calloc)(u32 count, u32 size);                                  // 0x67fcb0
    void(__fastcall *beginFrame)(const u8 *data, u32 hasLast, s32 quality);        // DctVlc_BeginFrame
    void(__fastcall *macroblock)(u8 *reference, u8 *out, s32 pitch);               // DctVideo_DecodeMacroblock
    void(__cdecl *fromYuv)(const u8 *source, u8 *out, u32 count);                  // MoviePixels_FromYuv
};
void MadVideo_SetEnv(const MadVideoEnv *env);

extern "C" {

// 0x6771a0 (fastcall (player, data; out, pitch, kind), pops 12, returns 0). The first frame of all (the flag 0xfaac64 is the game's, so never again for another player): the player's +0x664,
// +0x668, +0x66c each 1 by (settings' height +0x60 * width +0x64 * 4) from calloc. The bit reader from data + 0x10 with the quality data[0xd] (signed), a last frame unless kind 5. The macroblocks
// of 16 by 16 decoded into +0x66c for kind 7, else +0x668, each with its reference in +0x664 at the same place ((y * width + x) >> 1 dwords, the >> 1 of the sum). The rows then turned into pixels:
// height - 1 of them (at least once, so a height of 0 runs on), width >> 1 dwords from each, out on by pitch. Unless kind 7, +0x664 and +0x668 swap.
u32 __fastcall MadVideo_DecodeFrame(u8 *player, const u8 *data, u8 *out, u32 pitch, u32 kind);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gMadVideo_DecodeFrame_faac64, 0x00faac64, 4);
