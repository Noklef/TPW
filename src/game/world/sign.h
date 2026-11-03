// A sign in the world: the one record at 0x7afd08 and the ride it shows. Reimplementation of 0x4689f0 (name invented, unconfirmed).
//
// The record: +0 set, +4 the ride, +8 its root's matrix (0x40 bytes), +0x48 an angle, +0x4c the caller's value, +0x50 to +0x58 three floats, +0x5c half width,
// +0x60 half depth, +0x64 a ratio, +0x68 and +0x6c the two distances, +0x74 to +0x84 the texture entries and the type.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs them as recorders).
struct SignEnv {
    u32(__cdecl *createRide)(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);   // Ride_Create (0x463060)
    void(__cdecl *findEntries)(u8 *model, u32 *out1, u32 *out2);                                                                      // Model_FindSignEntries (0x468950)
    void(__cdecl *setScale)(const u8 *holder, const float *scale);                                                                    // Holder_SetScale (0x45bf20)
    void(__cdecl *markModel)(const u32 *index);                                                                                       // Ride_MarkModel (0x461ed0)
    void(__cdecl *flagModel)(const u8 *const *ref);                                                                                   // Holder_FlagModel (0x45bed0)
    void(__cdecl *wake)(u8 *holder, const u32 *matrix, u32 grid);                                                                     // Holder_WakeWithMatrix (0x45bd30)
    u32(__cdecl *play)(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index);                                          // Animator_Play (0x4732a0)
};
// The calls go to env (nullptr: the ports).
void Sign_SetEnv(const SignEnv *env);

extern "C" {
// 0x4689f0 (cdecl (value, a, height, b, c, type); name invented, unconfirmed; 0x4864d8 calls it); answers 0x7afd08.
// A ride of the type (Ride_Create (type, 0xc01; -1)), into 0x7afd0c. With the type's model's +0xd0: the sign entries of that model and of the ride's
// (Model_FindSignEntries); when all four are found, they and the type go to 0x7afd7c to 0x7afd8c and the ride's model's faces (groups at each node, u16
// +0x5a, 0x10 bytes at +0x6c) that used the ride's two take the type's.
// 0x7afd54 = value, 0x7afd58 to 0x7afd60 = a, b, c. From the box at the instance's (+0xcc) +0x18 and +0x24: the width, the depth, the half diagonal, the
// distances of the far corner from the centre (0x7afd74) and from the near corner on the floor (0x7afd70), the ratio at 0x7afd6c. The scale is the less of
// c / that distance and b * 2 / 3 / the half diagonal (unordered: the first); Holder_SetScale with it, and the width, depth and two distances scaled (half the
// width and depth). The root's matrix (the model's +0x78, +0x10) to 0x7afd10, Ride_MarkModel, Holder_FlagModel, the instance's +0xc8 pointing at the local
// identity matrix with height as +0x38 (sic: left dangling), Holder_WakeWithMatrix (1), Animator_Play (5, 0, 1, 1.0, 0) unless the ride is 0. 0x7afd08 = 1
// and 0x7afd50 taken modulo the double 0x6fe7d8.
u8 *__cdecl Sign_Create(u32 value, float a, u32 height, float b, float c, u32 type);
}
