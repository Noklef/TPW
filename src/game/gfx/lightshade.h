// The lighting of a height image: each marked pixel's normal from its neighbours, its colour from a tiled image, diffuse and specular light added (x87 in the original's order). Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct LightShadeEnv {
    double(__cdecl *pow)(double x, double y);  // the game's _CIpow 0x67c5d0
};
void LightShade_SetEnv(const LightShadeEnv *env);

// 0x5eb8a0 (thiscall (heights: an image {width, height, ?, bytes}), pops 4; 1412 B; name invented, unconfirmed). The object: +4 the height scale, +0xc, +0x10, +0x14 the ambient, diffuse and specular levels,
// +0x18 the specular power, +0x1c, +0x20 the light's angles in degrees, +0x24 the tile's rows (0: nothing is done), +0x28 the row offset, +0x38 the mask (a byte a pixel), +0x3c the colour image {width, height, ...},
// +0x68 the output (4 bytes a pixel). The colour image is shrunk to a work image of (its width, +0x24 rows). A pixel with mask 0 gives four zero bytes; else the mask byte and the three colours, each scaled
// by 255, clamped to 0..255 and truncated. (The first byte of the work image's pixel is not read.)
void __fastcall LightShade_Apply(u8 *self, void *, const u8 *heights);
