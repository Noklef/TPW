// A track ride's cars set on the ground: each car's four wheels are placed, the floor is found under each, and its models get their positions
// and rotations.
#pragma once

#include "../../../re/types.h"

// Its calls (a bench's recorders in their place).
struct TrakPoseEnv {
    u8 *(__cdecl *rideAt)(u32 x, u32 y);                  // Terrain_RideAt (0x450ac0)
    void(__cdecl *beginView)(const u8 *view);             // Model_BeginView (0x450ea0)
    void(__cdecl *floorAt)(float x, float z, float *out); // View_FloorAt (0x4511a0)
    s32(__cdecl *spawn)(s32 effect, s32 x, s32 y, s32 z); // Particle_Spawn (0x521e60)
};
void TrakPose_SetEnv(const TrakPoseEnv *env);

extern "C" {
// 0x546280 (cdecl (handle, a); name invented, unconfirmed; 0x5190d1 calls it): a is clamped to -1..1 (a NaN is 1). With the module
// loaded ('JEFG' at 0x877b58) the ride is [0x877b60] + (handle & 0xff) * 0xd0, else null; the handle has to be the index | the ride's +0 << 8 (0 with no module). For each of
// the first [0x877b6c] used cars (bit 0) of the pool [0x877b68] on the ride (+0x9c): its position +0x34, +0x38 less -a times its velocity +0x3c, +0x40, over 307.2; its angle
// +0x54 less the truncated +0x5c * -a; four wheels at (s, c), the sine and cosine times the ride's +8 scaled (x + s, y + c; x - s, y - c; x - c, y + s; x + c, y - s), each with the car's
// height +0xa0 and the ride at its cell (over 10). Then each ride in turn: the view made current, its wheels' floor found (-100 keeps the car's height). Then each car: with
// bit 24 the wheels' heights sway (+0x90 by 6, 4, 3, 5 through the sine table 0x877358), the pitch and the roll by the differences; +0xa0 the height, the mean of two wheels
// with bit 11, or of the four; bit 10 a sway on it, bit 12 a tilt by +0x5c and +0x60. The model at +8 (0x7a4610 -> +8 -> +4 -> +0x78; its matrix +0x10) gets them; bit 26
// (cleared) spawns effect 15 there; bit 13 sets the model at +0xc likewise, at the four's mean; bit 25 sets that model back along the angle by +0x4c, 0.7 up (the position kept).
// Sic: the wheels' buffer is 0x2000 bytes with no check: past 64 cars on a ride the original overruns its frame, this its own.
void __cdecl TrakRide_PoseCars(u32 handle, float a);
}
