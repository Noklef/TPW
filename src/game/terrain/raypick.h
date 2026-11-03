// The ray from the camera through the mouse against models.
#pragma once

#include "../../../re/types.h"

struct RayPickMatrix {
    float m[16];
};

struct RayPickEnv {
    void(__cdecl *transformNode)(const u8 *node, const float *m);   // WorldVerts_TransformNode 0x466c80
};
void RayPick_SetEnv(const RayPickEnv *env);

extern "C" {

// 0x45d090 (cdecl (node, matrix by value); name invented, unconfirmed): 1 when the ray from the point [0x7a1cb0] along [0x7a1cd8] hits a face of the node or of those under it, else 0. The matrix
// is multiplied by the node's own (+0x10). A node without flags 0x8210 has its vertices transformed (WorldVerts_TransformNode); its groups (+0x6c, u16 +0x5a of them, 0x10 bytes each: u16 +4 the
// first face, u16 +6 how many) of faces (+0x70, 8 bytes each: u16 +0 a normal (+0x64, 12 bytes each; low 15 bits), three u16 into the index +0x94, in turn into the world vertices [0x7aecd4]).
// The ray is cut with the plane of the turned normal; the point is in the face when the angles it sees between the three corners come to at least 6.28218555. The first face hit stores the point
// at 0x7a1cc8, the normal at 0x7a1c80, and returns 1 at once. Otherwise, without flag 0x20, the children (+0xc, then +8 each) until one answers.
// sic: the plane's other side is hit too (the distance is not looked at); a zero normal or corner gives NaNs, no hit.
u32 __cdecl RayPick_Node(u8 *node, RayPickMatrix m);
}
