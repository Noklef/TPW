// A coaster's test run: the train's speed, forces and bank along the track nodes, the ratings, and the trains the run's time
// allows. Reimplementation of 0x43c450 (name invented, unconfirmed).
//
// The coaster's track nodes are 0x38 bytes at +0x9c (+0xa4 of them): +0 flags (8: climbing), +4 / +8 / +0xc the position, +0x10 /
// +0x14 / +0x18 its angles (+0x18 the bank, which the run sets), +0x1c the speed, +0x20 / +0x24 / +0x28 the force, +0x2c the piece
// (its +0x10 the piece's kind: +0 the kind, +8 flags). The trains are 0x80 bytes at +0x128 (+0xf4 of them, at most the type's +0x2e8):
// +0 flags, +8 the first car, +0xc the cars, +0x28 the cars (0x60 bytes: +4 the car's model, its +0x24 / +0x28 the lengths ahead and
// behind; +0x38; +0x40 the position along the track), +0x2c the last car, +0x30 the position.
#pragma once

#include "../../../re/types.h"

// What the test run calls (the ports by default; its bench runs them as recorders).
struct TestRunEnv {
    void(__cdecl *ratingReset)(const u8 *coaster);   // CoasterRating_Reset (0x43dbf0)
    void(__cdecl *ratingStep)(u8 *coaster, u32 node, u32 unused, float speed, float bank, const float *force,
                              float unused2);                                           // CoasterRating_Step (0x43dc50)
    void(__cdecl *ratingFinish)(u8 *coaster, float maxHeight, float topSpeed, float unused);   // CoasterRating_Finish (0x43df90)
    void(__cdecl *setTrainCount)(u8 *coaster, u32 count);                               // Coaster_SetTrainCount (0x43ba20)
    void(__cdecl *wake)(u8 *coaster, u8 *car);                                          // CoasterCar_Wake (0x43be10)
    u32(__cdecl *removeFromRect)(u8 *object, u32 from, u32 to);                         // Obj_RemoveFromRect (0x45be80)
    void(__cdecl *holderSleep)(u8 *holder);                                             // Holder_Sleep (0x45bdd0)
    void(__cdecl *stopSounds)(u8 *coaster, u8 *car);                                    // CoasterCar_StopSounds (0x439670)
    void(__cdecl *spaceCars)(u8 *coaster);                                              // Coaster_SpaceCars (0x43c1f0)
};
// The calls go to env (nullptr: the ports).
void TestRun_SetEnv(const TestRunEnv *env);

extern "C" {
// 0x43c450 (cdecl (coaster); name invented, unconfirmed; 0x43ee5f calls it with [0x792218]): CoasterRating_Reset; v = the type's
// (+4) +0x44, the least speed. For each node i from 1 to the count less 2 (count - 1 unsigned, sic: none, a count of 0 runs on), with
// its neighbours: with the type's flag 0x100 its flag 8 when the climb to the next (dy over the distance) is above 0.01745; the force
// -(d to the previous + d to the next) / +0xac (each axis); v += (its height - the next's) * the type's +0x28 (the next higher) or +0x2c
// (not), times +0x24, at least the least speed (fcom: a NaN takes it); flag 8: v = that times the type's +0x54; at least the least speed;
// equal to it (or a NaN) sets the kind's flag 0x800; at most the coaster's +0x34; the node's speed v and force times v times 0.1f;
// CoasterRating_Step (coaster, i, kind, v, bank, &force, time); the top speed and height (fcom: a NaN takes the node's); time +=
// +0xac / v. The first node: force 0 and the least speed; the last the one before's. +0x134 = (count - 1) - the speed of node
// (+0x58)->+0xfc less 1 / +0xac, at least 0 (a NaN: 0).
// - The type's flag 8: from a last bank of pi, for every node: the type's up (+0x60) plus its force, turned about y by -+0x10, about x
//   by -+0x14 and about z by -+0x18 (the sine table); its angle Vec3_AngleZ; the turn from the last bank, fmod (a - last + 3pi, 2pi)
//   - pi (the doubles 0x6fe048 / 0x6fe050, a float's digits), times the vector's length times the type's +0x6c (at most 1) onto the
//   last bank, wrapped the same way: e; the node's bank -(pi - e), below -pi (or a NaN) plus 2pi; past +0x134 (fild of the index) the
//   bank times 1 - (i - +0x134) / (+0xa8 - +0x134) (held to 0..1: at least 1 or 1, a NaN 0); the last bank e.
// CoasterRating_Finish (coaster, height, top speed, time). With the coaster's flag 0x1000 (cleared) and without 0x4000: the trains,
// __ftol (time / (the type's +0x2ec * 0.001f)) at least 1, at most the type's +0x2e8 (Coaster_SetTrainCount); of the type's +0x2e8
// trains, those below +0xf4 not running (flag 1) gain 5 (with the coaster's 0x800: CoasterCar_Wake), the rest running lose 1 (with 0x800:
// the cars' flag 1 cleared, their strip objects (0x7a4610) taken out (Obj_RemoveFromRect by +0xc0 / +0xc4 with flag 0x4000, else
// Holder_Sleep), CoasterCar_StopSounds and flag 2 cleared). Coaster_SpaceCars; each running train: its cars' positions from the last back
// from the train's, each the next's plus the lengths (the car's ahead and the one before's behind) / +0xac; +0x74 the last car's; +0x20
// the first's +0x38 / +0xac + that + its length ahead / +0xac; +0x24 the last's +0x38 / +0xac + +0x74 - its length behind / +0xac;
// +0x78 the cars' speeds (the nodes' at each position, modf: by its fraction between the node and the next), averaged; flag 4 cleared.
void __cdecl Coaster_TestRun(u8 *coaster);
}
