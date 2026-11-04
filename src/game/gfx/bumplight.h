// A height map lit into a texture.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

struct BumpLightEnv {
    double(__cdecl *pow)(double x, double y);   // the game's _CIpow 0x67c5d0
};

// The env's and calls' replacements (tests/fncmp/bumplight.py gives them recorders).
void BumpLight_SetEnv(const BumpLightEnv *env, const AddrCall *table, u32 count);

// The callees of Bump_Build and Bump_FilterInto (tests/fncmp/bumpbuild.py gives them recorders).
struct BumpStepsEnv {
    u8(__fastcall *realloc)(u8 *object, void *, u32 a, u32 b, u32 c);                              // Array3D_Realloc
    void(__fastcall *ramp)(void *object, void *, const u32 *a, u32 count, const u32 *b);           // Image_ErosionRamp
    void(__fastcall *blur)(void *object, void *, const u32 *image, const u32 *mask, float sigma);  // Image_GaussianBlur
    void(__fastcall *render)(u8 *self, void *, const u8 *heights);                                 // BumpLight_Render
    u8(__cdecl *crop)(u8 *destination, u8 *source, s32 x, s32 y);                                  // Image_CropInto
    u32(__cdecl *erode)(u32 *image, s32 radius);                                                   // Image_Erode
    u32(__cdecl *dilate)(u32 *image, s32 radius);                                                  // Image_Dilate
    u8(__cdecl *lines)(const u32 *image, double sigma);                                            // Image_GaussianBlurLines
    void(__fastcall *free)(u8 *object);                                                            // Array3D_Free
};
void BumpSteps_SetEnv(const BumpStepsEnv *env);  // nullptr: the game's

extern "C" {

// 0x5eb8a0 (thiscall (heights), pops 4; name invented, unconfirmed): no rows (+0x24): nothing. Its image (+0x3c, width at +0x3c, +0x24 rows) shrunk into a block of its own (Image_ShrinkToImage), freed at the end.
// The light's direction from the two angles (+0x1c, +0x20, in degrees), the half vector from it and the view (0, 0, -1). Each pixel of the heights (width +0, rows +4, bytes +0xc; each next one read, and the last row's, past the end, sic) where the mask (+0x38) is set: the normal from the differences
// (scaled by +4 / 255) lights the texel of the block at that place (rows shifted by +0x28, wrapped): the ambient (+0xc), the diffuse (+0x10) when positive, the specular (+0x14; its power +0x18) when positive, grey (sic) and clamped. Out (+0x68): mask byte, the three colours; 0 for all four where the mask is 0.
void __fastcall BumpLight_Render(u8 *self, void *, const u8 *heights);

// 0x5eb820 (thiscall; name invented, unconfirmed): nothing unless the arrays at +0x2c and +0x3c have bytes (+0x38, +0x48). The arrays at +0x4c (one byte per pixel) and +0x5c (four) sized as the
// one at +0x2c; Image_ErosionRamp of +0x2c into +0x4c (count +0); Image_GaussianBlur of +0x4c masked by +0x2c when the float at +8 is above 0; then BumpLight_Render of +0x4c.
void __fastcall Bump_Build(u8 *self);

// 0x5ebf10 (thiscall; source image, destination array, pops 8; AL; name invented, unconfirmed): 0 when the source has no bytes. The destination sized (one byte per pixel) and cropped from the
// source at (+0xc, +0x10); a radius at +4 erodes (below 0) or dilates (above 0) it; a result other than 1 frees the destination and gives 0. A float at +8 that is neither 0 nor a NaN then
// blurs it (Image_GaussianBlurLines), whose result is returned, the destination kept whatever it says.
u8 __fastcall Bump_FilterInto(u8 *self, void *, u32 *source, u8 *dest);
}
