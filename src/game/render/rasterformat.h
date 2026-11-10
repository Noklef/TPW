// The software rasteriser's pixel format and buffers (0x678e70, called by the device's two set-ups at 0x573aaf and 0x573bf4). Its calls into
// agent B's unported 0x679150-0x679230 go through RasterFormatEnv (by address by default), so that the bench (tests/fncmp/rasterformat.py) can
// hand each a recorder.
#pragma once

#include "../../../re/types.h"

struct RasterFormatEnv {
    u32(__cdecl *bitCount)(u32 mask);                // 0x679150: the bits set in bits 0-30 of the mask
    u32(__cdecl *lowBit)(u32 mask);                  // 0x679170: the position of the mask's lowest set bit
    void(__cdecl *blendTables)();                    // 0x679190
    u32(__cdecl *zBuffer)(u32 width, u32 height);    // 0x679230: answers the block once (0xfac534, aligned at 0xfac538), then 0
    u32(__cdecl *frameBuffer)(u32 width, u32 height);  // 0x6791f0: likewise (0xfac52c, 0xfac530)
};

void RasterFormat_SetEnv(const RasterFormatEnv *env);  // nullptr: the game's

extern "C" {

// 0x679150: the bits set in bits 0-30 of the mask. 0x679170: the position of the lowest set bit, by signed shifts (no bit at all never ends, sic).
u32 __cdecl RasterFormat_BitCount(u32 mask);
u32 __cdecl RasterFormat_LowBit(u32 mask);
// 0x679190: two tables of 0x40 eight-byte rows, the four words of each the same: at 0xfbe2e0 i * 0x10000 / 255, at 0xfbe4e0 0x4000 less the sum of those so far.
void __cdecl RasterFormat_BlendTables();
// 0x6791f0: with no block yet ([0xfac52c]) one of width * height * 4 + 0x6423 bytes, its address kept, and the address 0x3220 on rounded down to 0x20 kept at 0xfac530 and answered (a failed malloc too); else 0.
// 0x679230: likewise: width * height * 2 + 0x3223 bytes, 0xfac534, 0x1920 on, 0xfac538.
u32 __cdecl RasterFormat_FrameBuffer(u32 width, u32 height);
u32 __cdecl RasterFormat_ZBuffer(u32 width, u32 height);
// 0x679290, 0x6792c0: the block, if any, freed and both its globals cleared. 0x679270: the z buffer's, the frame buffer's, and the count 0xfac514 down.
void __cdecl RasterFormat_FreeFrameBuffer();
void __cdecl RasterFormat_FreeZBuffer();
void __cdecl RasterFormat_FreeBuffers();

// 0x678e70 (cdecl (surface, z buffer out, width, height); name invented, unconfirmed). With a surface whose pixel format (DDPIXELFORMAT, 0x20
// bytes, its size set; IDirectDrawSurface slot 21) is not 8 bits: the channels' bit counts into 0xfbe054 / 58 / 5c, their sum 0xfbe060, the masks'
// lowest bits 0xfbe048 / 4c / 50; the masks and their complements 0xfbe064-0xfbe078, all three in both halves 0xfbe07c and its complement
// 0xfbe090, green | blue 0xfbe094 and its complement 0xfbe098; (m >> n) & m of the three or'ed for n 1-4 0xfbe080-0xfbe08c (signed shifts);
// both lines to OutputDebugStringA (formatted by the strings 0x77e5ec and 0x77e5cc); the weights 0xfbe190-0xfbe1a4, the shift 0xfbe1a8 (by green's
// 5 or other bits), 0xfbe1ac = 0. In any case 0xfbe008 = 0xffff, 0x679190, 0xfac514 counts up, with an out 0x679230's answer through it,
// and the answer is 0x6791f0's.
u32 __cdecl RasterFormat_Init(u8 *surface, u32 *zBufferOut, u32 width, u32 height);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gRasterFormat_fac514, 0x00fac514, 0x4);
GAME_DATA(u32, gRasterFormat_fac52c, 0x00fac52c, 0x4);
GAME_DATA(u32, gRasterFormat_fac534, 0x00fac534, 0x4);
GAME_DATA(double, gRasterformat_fbe008, 0x00fbe008, 0x8);
GAME_DATA(u32, gRasterformat_fbe048, 0x00fbe048, 0x4);
GAME_DATA(u32, gRasterformat_fbe04c, 0x00fbe04c, 0x4);
GAME_DATA(u32, gRasterformat_fbe050, 0x00fbe050, 0x4);
GAME_DATA(u32, gRasterformat_fbe054, 0x00fbe054, 0x4);
GAME_DATA(u32, gRasterformat_fbe058, 0x00fbe058, 0x4);
GAME_DATA(u32, gRasterformat_fbe05c, 0x00fbe05c, 0x4);
GAME_DATA(u32, gRasterformat_fbe060, 0x00fbe060, 0x4);
GAME_DATA(u32, gRasterformat_fbe064, 0x00fbe064, 0x4);
GAME_DATA(u32, gRasterformat_fbe068, 0x00fbe068, 0x4);
GAME_DATA(u32, gRasterformat_fbe06c, 0x00fbe06c, 0x4);
GAME_DATA(u32, gRasterformat_fbe070, 0x00fbe070, 0x4);
GAME_DATA(u32, gRasterformat_fbe074, 0x00fbe074, 0x4);
GAME_DATA(u32, gRasterformat_fbe078, 0x00fbe078, 0x4);
GAME_DATA(u32[5], gRasterformat_fbe07c, 0x00fbe07c, 0x14);
GAME_DATA(u32, gRasterformat_fbe090, 0x00fbe090, 0x4);
GAME_DATA(u32, gRasterformat_fbe094, 0x00fbe094, 0x4);
GAME_DATA(u32, gRasterformat_fbe098, 0x00fbe098, 0x4);

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x20], gText_Rasterformat_77e5cc, 0x0077e5cc, 0x20);
GAME_DATA(char[0x18], gText_Rasterformat_77e5ec, 0x0077e5ec, 0x18);
