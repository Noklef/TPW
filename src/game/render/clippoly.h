// The clipper of a polygon of vertex indices against the six planes of the view volume (0x562c90, 0x562ed0): reimplementation. The vertices
// are as src/game/render/clipedge has them (x, y, z, w floats first), stride bytes apart; new ones are appended to the buffer.
#pragma once

#include "../../../re/types.h"

extern "C" {

// A polygon: count indices (u16) at idx.
struct ClipPolygon {
    s32 count;
    u16 *idx;
};

// 0x562c90 (thiscall on the block at 0x879a20, which it does not use; name unconfirmed): for each plane in turn while flags is not 0 (flags
// shifted right, signed, after each): bit 0 set clips poly by it, Sutherland-Hodgman: a vertex is out when its distance (((w pw + y py) + z pz)
// + x px, the plane's (px, py, pz, pw) from the table at 0x766188 down by 16: z, w - z, x + w, w - x, y + w, w - y) is below 0 or NaN; walking
// the indices with the previous one (the last at first; idx[-1] for an empty polygon, sic): both in keeps the index, an edge crossing gets the
// new vertex (the table at 0x7661a4 down by 4: ClipEdge_Z0, ZW, XNegW, XW, YNegW, YW, as (out, the outside one, the inside one, stride)) at
// vertices + *vertexCount * stride and its index (*vertexCount, then counted), before the inside index when that one comes back in. The result
// goes to a 12-index local polygon or back to poly, alternating (no bound, sic). A result left in the local is copied to poly. Returns
// poly's count - 2.
s32 __fastcall ClipPoly_Indexed(void *self, void *edx, s32 flags, u8 *vertices, ClipPolygon *poly, u32 *vertexCount, s32 stride);

// 0x562ed0 (stdcall; name unconfirmed): the triangle of the three indices before end clipped by ClipPoly_Indexed (with the block at 0x879a20 as
// its this), then fanned out in place from end - 3 into count - 2 triangles (0, j - 1, j), the last first. Returns the end of the triangles
// (end - 3 + count when the result has 3 or fewer indices).
u16 *__stdcall ClipPoly_TriangleFan(s32 flags, u16 *end, u8 *vertices, u32 *vertexCount, s32 stride);

// 0x562a70 (stdcall; flags, dest, source, count pointer, stride; name unconfirmed): ClipPoly_Indexed's clipping on a polygon of *count vertices
// themselves, read from source: each pass writes the kept vertices (copied, rep movsd / movsb) and the new ones into the other buffer (dest
// first, then source, alternating), *count its count; the distances are summed in other orders (the vertex reached w pw + ((pz z + x px) +
// y py), the one before it x px + ((z pz + y py) + w pw)). 0 as soon as a pass leaves 2 or fewer (or the count was 0); else 1, the result
// copied to dest when it ended in source.
s32 __stdcall ClipPoly_Vertices(s32 flags, u8 *dest, u8 *source, u32 *count, s32 stride);
}
