// A coaster's trains moved on one step.
#pragma once

#include "../../../re/types.h"

struct TrainRunEnv {
    u32(__cdecl *time)(u32 real);                                                 // Motion_Time 0x4709e0
    void(__cdecl *bump)(const u8 *coaster, u8 *train);                            // CoasterCar_Bump 0x43cd60
    void(__cdecl *frameAt)(u8 *coaster, float position, float *out, u8 *orient);  // Coaster_FrameAt 0x43c0a0
    void(__cdecl *orient)(u8 *car, const float *angles, float *v);                // RideObject_Orient 0x43d8f0
    void(__cdecl *setSpeed)(u8 *ride, float speed, u32 index);                    // Layer_SetSpeed 0x474050
    void(__cdecl *bend)(const u8 *coaster, const u8 *car);                        // Coaster_BendMesh 0x438800
    void(__cdecl *setSound)(u8 *train, s32 mode, u32 value);                      // CoasterCar_SetSoundMode 0x439780
};
void TrainRun_SetEnv(const TrainRunEnv *env);

extern "C" {

// 0x43ce20 (cdecl (coaster); name invented, unconfirmed): the time since the last run (Motion_Time (0) less +0x120, in thousandths, into each train's +0x7c; +0x120 set first when flag 0x10 of
// the coaster is clear). Each on car's (+0, bit 0) +0x48..+0x50 take the track node point at its place (+0x38 / +0xac + the train's +0x74, modulo the length +0xa8). Then the trains take turns by
// the time they have left, least... each step takes the train at the head of the list +0x12c: its cars' +0x54..+0x5c the point half a node on, each car's speed from the climb to there (the type's
// +0x24, +0x28 going up, +0x2c going down, at least the chain limit (type +0x44 * +0x2c, halved with train flags 0x80 and 0x20) on a chain piece, else type +0x48 * +0x2c), their mean clamped
// to +0x34; past +0x134 (and without flag 0x40) mixed towards the limit by a fourth power, which also sets flag 0x10; a node with flag 8 scales it by type +0x54. Time to the point half a node on,
// from the lead car's two points: the train is there (half a node on, +0x7c less the time, put back in the list by it) or stops in between (off the list). Then the speed's drag, the extent
// (+0x20, +0x24), CoasterCar_Bump, the place modulo the length, flag 0x40 cleared between +0xfc and +0x134.
// Then each train's cars placed: +0x40, Coaster_FrameAt, RideObject_Orient with the model's +0x18 lifted, the ride's matrix +0x40..+0x48, Layer_SetSpeed unless the model's bit 0; the train's
// +0x48..+0x50 from its lead car (+0x2c), +0x3c..+0x44 from its first (+8); Coaster_BendMesh with the model's bit 0. Sounds: flag 0x10 mode 8, cars on a chain mode 4 or 2 by the speed over +0x34,
// else mode 0x10. +0x124 the train furthest back. Flag 0x80 clears when the node count to the next train takes at least type +0x2ec thousandths at the first node's rate. +0x120 = now.
// sic: flag 0x10's mode 8 reads the fourth power as last computed for any train, or the bits of the end of the train array when none has (as a float, rounded); a train with no cars divides by 0
// to a NaN mean; the zero-divisor faults with a chain car and a train type of +4 0; a NaN place loops for ever; the "last" car of +0x24 is counted from the first (+8), not from car 0; a place
// more than the length below zero gives a negative index, off the node table.
void __cdecl Coaster_RunTrains(u8 *coaster);
}
