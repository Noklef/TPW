// A car's mesh bent along the track.
#pragma once

#include "../../../re/types.h"

struct BendMeshEnv {
    void(__cdecl *nodeAngles)(const u8 *path, const u8 *car, float *out1, float *out2, float *out3, u32 unused, float position, u32 *count);   // Coaster_NodeAngles 0x438ab0
    void(__cdecl *partStates)(u8 *holder);                                                                                                      // PartStates_Update 0x44ab30
};
void BendMesh_SetEnv(const BendMeshEnv *env);

extern "C" {

// 0x438800 (cdecl (path, car); called from 0x43d730; name invented, unconfirmed): nothing for a car with no ride (+0xc). Coaster_NodeAngles over the car's place (+0x40) gives three tables of up
// to 0x80 floats and their count. With a holder in the ride table 0x7a4610, the model's frame numbered by the car's type's +0x20 ([model+0x70] 0xa0 bytes each below the model's u16 +0x44, then
// [model+0x74] 0x58 bytes each), unless it has flag 0x200: each of its vertices (u16 +0x58; packs of 4, x y z over 12 floats) from the base model's same-numbered frame ([model+0x9c] -> +0x70,
// +0x60) to the frame's +0x60, at (count * spacing + z) / spacing along the tables: y raised by the third table's value there, the vector turned by the second's (Vec3_RotateX), then the first's
// (Vec3_RotateY). Then PartStates_Update of the holder's +8.
// sic: a ride with no holder faults at the end reading +8 of null; a whole part outside the filled tables reads the frame's stack, unset, which the port's tables do not repeat.
void __cdecl Coaster_BendMesh(const u8 *path, const u8 *car);
}
