// Copying one bitmap's pixels into another (0x5fc320-0x5fc528): a straight row copy when the two
// share a format, otherwise through the game's one static PixelConversion (0xfa7d18, set up
// before main). Convert is a thiscall whose object goes unused.
//
// The originals' quirks are kept: the conversion is set up with the width as its flags (so an
// odd width keeps 24/32-bit sources off the 16-bit converter); converting in place first cuts
// the destination's width to what its pitch holds of the source's rows, in destination pixels;
// a straight copy re-reads the pitches every row and copies forwards in dwords, then bytes.
#pragma once

#include "../../../re/types.h"
#include "pixconv.h"
#include "surface.h"

struct ConvertEnv {
    PixelConversion *conversion;  // 0xfa7d18
    PixelConversion *(__fastcall *ctor)(PixelConversion *);  // 0x630880
    void(__fastcall *dtor)(PixelConversion *);                // 0x630c30
    void(__fastcall *setup)(PixelConversion *, void *, const u32 *target, const u32 *source, const u32 *palette,
                            u32 flags);  // 0x630890
    void(__fastcall *rect)(PixelConversion *, void *, u8 *dst, const u8 *src, s32 dstPitch, s32 srcPitch, u32 rows,
                           u32 width);  // 0x630ba0
    s32(__cdecl *atExit)(void(__cdecl *)());  // 0x67a7d0
    void(__cdecl *onExit)();                  // 0x5fc350
};
// nullptr restores the game's.
void Convert_SetEnv(const ConvertEnv *e);

extern "C" {
void __cdecl Convert_StaticInit();     // 0x5fc320: Construct, RegisterExit
PixelConversion *__cdecl Convert_Construct();  // 0x5fc330: the conversion's constructor
void __cdecl Convert_RegisterExit();   // 0x5fc340
void __cdecl Convert_AtExit();         // 0x5fc350: its destructor
// 0x5fc360 (&out, dst, src, palette): dst's bits from src's, over the smaller of the two; -1 when
// either has no bits.
s32 *__fastcall SurfaceInfo_Convert(void *self, void *edx, s32 *out, SurfaceInfo *dst, const SurfaceInfo *src,
                                    const u32 *palette);
}
