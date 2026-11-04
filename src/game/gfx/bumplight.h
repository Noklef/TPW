// A height map lit into a texture.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

struct BumpLightEnv {
    double(__cdecl *pow)(double x, double y);   // the game's _CIpow 0x67c5d0
};

// The env's and calls' replacements (tests/fncmp/bumplight.py gives them recorders).
void BumpLight_SetEnv(const BumpLightEnv *env, const AddrCall *table, u32 count);

extern "C" {

// 0x5eb8a0 (thiscall (heights), pops 4; name invented, unconfirmed): no rows (+0x24): nothing. Its image (+0x3c, width at +0x3c, +0x24 rows) shrunk into a block of its own (Image_ShrinkToImage), freed at the end.
// The light's direction from the two angles (+0x1c, +0x20, in degrees), the half vector from it and the view (0, 0, -1). Each pixel of the heights (width +0, rows +4, bytes +0xc; each next one read, and the last row's, past the end, sic) where the mask (+0x38) is set: the normal from the differences
// (scaled by +4 / 255) lights the texel of the block at that place (rows shifted by +0x28, wrapped): the ambient (+0xc), the diffuse (+0x10) when positive, the specular (+0x14; its power +0x18) when positive, grey (sic) and clamped. Out (+0x68): mask byte, the three colours; 0 for all four where the mask is 0.
void __fastcall BumpLight_Render(u8 *self, void *, const u8 *heights);
}
