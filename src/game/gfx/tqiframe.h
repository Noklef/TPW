// The movie player's second DCT video: a whole frame's macroblocks. Reimplementation of 0x671800 and 0x671940; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The hand-written macroblock decoders (the ports by default; the bench's recorders: their own benches have them).
struct TqiFrameEnv {
    void(__cdecl *rgb16)(u8 *out, u32 pitch, u8 *state);    // TqiVideo_DecodeMacroblockRgb16
    void(__cdecl *table16)(u8 *out, u32 pitch, u8 *state);  // TqiVideo_DecodeMacroblockTable16
    void(__cdecl *yuy2)(u8 *out, u32 pitch, u8 *state);     // TqiVideo_DecodeMacroblockYuy2
};
void TqiFrame_SetEnv(const TqiFrameEnv *env);

extern "C" {

// 0x671800 (fastcall (player, value; out, unused, pitch in bytes), pops 12): TgqVideo_SetBlockBase (value, player); then rows while y (from 0, by 16) is below the settings' +0x60, in each while x is below +0x64 (unsigned, read again each pass; the settings are the player's dword +0): a macroblock at
// out (TqiVideo_DecodeMacroblockRgb16 with the player's +0x40 set, else TqiVideo_DecodeMacroblockTable16; pitch, the player's +0x408), out 0x20 bytes right, each row (pitch >> 1) << 5 bytes below the last.
void __fastcall TqiVideo_DecodeFrame16(u8 *player, u32 value, u8 *out, u32 unused, u32 pitch);

// 0x671940 (fastcall, as TqiVideo_DecodeFrame16): the same with the player's +0x508, +0x50c and +0x510 made 0 by it too (SetBlockBase did), each macroblock TqiVideo_DecodeMacroblockYuy2.
void __fastcall TqiVideo_DecodeFrameYuy2(u8 *player, u32 value, u8 *out, u32 unused, u32 pitch);
}
