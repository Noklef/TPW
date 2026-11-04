// An image {width, height, bytes per pixel, pixels} read from a file through the game's image codec. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "imagecodec.h"

struct ImageLoadEnv {
    s32 *(__fastcall *infoFromFile)(ImageCodec *c, void *, s32 *out, const char *name, const char *container, SurfaceInfo *info);  // ImageCodec_InfoFromFile (0x5fa8a0)
    s32 *(__fastcall *alloc)(SurfaceInfo *info, void *, s32 *out);                                                                  // SurfaceInfo_Alloc (0x5fa6f0)
    s32 *(__fastcall *loadFromFile)(ImageCodec *c, void *, s32 *out, SurfaceInfo *dst, const char *name, const char *container, SurfaceInfo *info);  // ImageCodec_LoadFromFile (0x5faa00)
    void(__fastcall *free)(SurfaceInfo *info);                                                                                      // SurfaceInfo_Free (0x5fa740)
};
void ImageLoad_SetEnv(const ImageLoadEnv *env);

// 0x5d3890 (thiscall (folder, name), pops 8; returns AL; name invented, unconfirmed): the two strings joined (a 256-byte buffer, unchecked, sic), the file's info read (negative: 0, sic: the surface's
// not freed), the surface's bits allocated, the pixels read (negative: bits freed, 0). The old pixels (+0xc) freed, the new block allocated: width * height when the surface is paletted (+0x14 == 1), four times that
// else; none: 0 with the surface's bits kept (sic), +0xc null. Paletted: the bytes copied, else each pixel's four bytes reversed. +0, +4 = width, height, +8 = 1 or 4, the surface's bits freed, 1.
extern "C" u8 __fastcall Image_LoadFile(u8 *image, void *, const char *folder, const char *name);
