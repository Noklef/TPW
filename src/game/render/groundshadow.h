// A flat quad laid over the height grid: reimplementation of 0x57be90.
#pragma once

#include "../../../re/types.h"

// What GroundShadow_Draw calls (the port by default; the bench runs it as a recorder).
struct GroundShadowEnv {
    u32(__fastcall *drawQuad)(u8 *object, void *, u8 *spec, u32 mask);  // Batch_DrawQuad (0x565f90)
    u32(__fastcall *draw)(u8 *object, void *);                          // GroundShadow_Draw (0x57be90)
};
// The call goes to env (nullptr: the port).
void GroundShadow_SetEnv(const GroundShadowEnv *env);

extern "C" {

// 0x57be90 (thiscall; name invented, unconfirmed). The object's +0x13c is a rectangle (none: returns 1): +0 / +0xc x, +0x14 / +8 z, +0x18 a texture or null. Its corners (x0 z0, x0 z1, x1 z1, x1 z0) stand on the height
// grid [0x879d60] plus [0x8bd4ec] (the first two inlined in the original, HeightGrid_HeightAt in effect), go through the transformer [0x87b1d8]'s slot 1 (cdecl: this, the matrix [0x87b088], out, in, 0x10, 0xc, 4, 0) and
// are a Batch_DrawQuad with mask -1, colour 0x80404040, batch flags 0xd56: its answer is returned. The polygon flags start as stack garbage (0 here), 4 with a texture's 0x100000.
// Texture_FormatBits' answer is overwritten in the original: left out.
u32 __fastcall GroundShadow_Draw(u8 *object, void *);

// 0x57c2a0 (thiscall on a count and an array from +4 of objects; name invented, unconfirmed): each drawn (GroundShadow_Draw); 0 at the first that answers 0, the count left; else the count cleared, 1.
u32 __fastcall GroundShadows_DrawAll(u8 *list);
}
