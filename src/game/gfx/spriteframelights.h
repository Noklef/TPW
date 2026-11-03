// A sprite frame's ground quads, one for each light: reimplementation of 0x589770.
#pragma once

#include "../../../re/types.h"

// What SpriteFrame_DrawLightQuads calls (the ports by default; the bench runs them as recorders).
struct SpriteFrameLightsEnv {
    u8 *(__fastcall *first)(u8 *list);                                                         // LightList_First (0x584e30)
    u8 *(__fastcall *next)(u8 *list);                                                          // LightList_Next (0x584e50)
    u32(__fastcall *groundQuad)(u8 *frame, void *, float *quad, u32 matrix, float width, float height);  // SpriteFrame_GroundQuad (0x589410)
    u32(__fastcall *drawQuad)(u8 *object, void *, u8 *spec, u32 mask);                         // Batch_DrawQuad (0x565f90)
};
// The calls go to env (nullptr: the ports).
void SpriteFrameLights_SetEnv(const SpriteFrameLightsEnv *env);

extern "C" {

// 0x589770 (thiscall; name invented, unconfirmed). A frame with flags 0x200 and 0x10000000 (else returns 1): the light list at 0x8c1658, a ground quad at each light of size 1 by 1 (its answer the colour), a Batch_DrawQuad
// with mask -1 (answer 0 returns 0; else 1). The texture is the panel's first (+0x28) with batch flags 0x800 and the frame's 0x20000, or with render flag 0x100000 by light: the table at [[+0x34] + 4], 0x18 bytes each, at
// the frame's +0x38 plus its byte +0x3c + the light's number, without them. Batch flag 0x200 is taken off. Polygon flag 4 for a texture's 0x100000 stays for the later lights (sic).
// The spec's +0x28, the transformer's argument, is unset stack in the original: 0 here.
u32 __fastcall SpriteFrame_DrawLightQuads(u8 *frame, void *);
}
