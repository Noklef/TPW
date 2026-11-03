// One textured quad as a batch of its own: reimplementation of 0x565f90.
#pragma once

#include "../../../re/types.h"

// What Batch_DrawQuad calls (the ports by default; the benches run them as recorders).
struct BatchQuadEnv {
    u8 *(__fastcall *begin)(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra);     // Batch_Begin (0x565240)
    u32(__fastcall *addPolygon)(u8 *object, void *, u8 *vertices, u32 count, u32 stride, const u8 *codes, u32 flags);  // DrawBatch_AddPolygon (0x565bb0)
    void(__fastcall *end)(u8 *buffer, void *, u32 vertexEnd, u32 indexEnd);                                 // DrawBuffer_End (0x582780)
    u32(__fastcall *flush)(u8 *object, void *);                                                             // Batch_Flush (0x565590)
    u32(__fastcall *drawQuad)(u8 *object, void *, u8 *spec, u32 mask);                                      // Batch_DrawQuad (0x565f90)
    u32(__fastcall *fillItem)(u8 *frame, void *, u8 *item);                                                 // SpriteFrame_FillItem (0x589b50)
};
// The calls go to env (nullptr: the ports).
void BatchQuad_SetEnv(const BatchQuadEnv *env);

extern "C" {

// 0x565f90 (thiscall on the object Batch_Begin keeps its batch in; spec, mask; pops 8; name invented, unconfirmed). The spec: +0 polygon flags, +4 four points of 0x10
// bytes, +8 colour, +0xc batch flags, +0x10 extra, +0x18 / +0x1c first u and v, +0x20 / +0x24 second u and v, +0x28 the transformer's argument. Fog in the render flags
// (0x8bcbc8 bit 0x2000) sets the batch flags' 0x100000. The points' clip codes with the mask: all off one side, 1 is returned. A batch (mode 2, texture 4, kind 0x1c4), 1
// without one. The colour is met with [[0x87a248] + 0x2c], and stored in the spec. The polygon's codes are the clip result's dword (the AND, the OR, then the object at
// 0x879a20's high half, sic), not the points' codes; its answer 0 returns 1 (sic). Then fog, the transformer, the polygon's flag 0x400 on the batch's +0; a batch with
// vertices and indices is ended and flushed, Batch_Flush's answer returned; else 1. Vertices' dword +0x14 is unset in the original too.
u32 __fastcall Batch_DrawQuad(u8 *object, void *, u8 *spec, u32 mask);

// 0x57ed50 (thiscall; name invented, unconfirmed): +4 = 0; a quad over the whole clip space (x, y -1 to 1, z 0.001, w 1) drawn on [0x879ab0] with mask -1: polygon flags 0, colour -1, batch flags 0x9a1, the rest 0.
// Batch_DrawQuad's answer. The spec's +0x14 is stack garbage in the original, not read; 0 here.
u32 __fastcall Batch_DrawScreenQuad(u8 *self);

// 0x589f90 (thiscall on a sprite frame; name invented, unconfirmed): a spec on the stack, its points inside it, filled (SpriteFrame_FillItem); its answer 0 returns 0, else drawn on [0x879ab0] with mask -1 and Batch_DrawQuad's
// answer. What the fill leaves unset is stack garbage in the original, 0 here.
u32 __fastcall SpriteFrame_DrawQuad(u8 *frame);
}
