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

// 0x678e70 (cdecl (surface, z buffer out, width, height); name invented, unconfirmed). With a surface whose pixel format (DDPIXELFORMAT, 0x20
// bytes, its size set; IDirectDrawSurface slot 21) is not 8 bits: the channels' bit counts into 0xfbe054 / 58 / 5c, their sum 0xfbe060, the masks'
// lowest bits 0xfbe048 / 4c / 50; the masks and their complements 0xfbe064-0xfbe078, all three in both halves 0xfbe07c and its complement
// 0xfbe090, green | blue 0xfbe094 and its complement 0xfbe098; (m >> n) & m of the three or'ed for n 1-4 0xfbe080-0xfbe08c (signed shifts);
// both lines to OutputDebugStringA (formatted by the strings 0x77e5ec and 0x77e5cc); the weights 0xfbe190-0xfbe1a4, the shift 0xfbe1a8 (by green's
// 5 or other bits), 0xfbe1ac = 0. In any case 0xfbe008 = 0xffff, 0x679190, 0xfac514 counts up, with an out 0x679230's answer through it,
// and the answer is 0x6791f0's.
u32 __cdecl RasterFormat_Init(u8 *surface, u32 *zBufferOut, u32 width, u32 height);
}
