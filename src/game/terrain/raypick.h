// The ray from the camera through the mouse against models.
#pragma once

#include "../../../re/types.h"

struct RayPickMatrix {
    float m[16];
};

struct RayPickEnv {
    void(__cdecl *transformNode)(const u8 *node, const float *m);   // WorldVerts_TransformNode 0x466c80
    double(__cdecl *surfaceHeight)(s32 x, s32 y, u32 *slot);        // Terrain_SurfaceHeight 0x452ae0
    u8 *(__cdecl *rideTrain)(u32 index, u32 *cars);                 // Ride_Train 0x447790
    u32(__cdecl *carValue)(const u8 *train, u32 n);                 // Train_CarValue 0x4477d0
    void(__cdecl *updateNormals)(u8 *model);                        // Model_UpdateNormals 0x473c30
    u32(__cdecl *node)(u8 *node, RayPickMatrix m);                  // RayPick_Node 0x45d090
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

// 0x45cdc0 (cdecl; name invented, unconfirmed): the ride holder the ray hits on the map cell (x, y), held in the map ([0x7a0314] by [0x7a0374]), or 0. The cell's slot and height from
// Terrain_SurfaceHeight; no slot, 0. A ride with a train (Ride_Train) goes through its cars, last first (Train_CarValue: the ride's number, its own surface height), others once, their model's
// normals brought up to date when it has flags 0x40000 and 0x20000 (Model_UpdateNormals). The ray's place along its way within a cell (alongX: atX, else atZ): stepping up (step above 0) the
// fraction of a tenth (modf); else one less a tenth of the rest of tens (fmod). The span of heights the ray passes over that cell, from atY: atY - place * drop to atY + (1 - place) * drop. When its
// lower end is not above the height and its upper end not below the model's +0x84: the ray ([0x7a1cd8] direction, [0x7a1cb0] origin) against the model's nodes (+0x78, RayPick_Node, identity matrix).
// A hit: the point copied to 0x7a1cb8, the normal to 0x7a1ca0, that ride returned. The first ride's holder comes from the slot, each car's from the table 0x7a4610.
// sic: atZ is only read as a place, the unused of step's sign; a ride's holder of null faults at its +8.
u8 *__cdecl RayPick_Column(s32 x, s32 y, u32 alongX, float atX, float atY, float atZ, float drop, const float *direction, const float *origin, float step);
}
