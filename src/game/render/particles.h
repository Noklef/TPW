// The particle systems' handle accessors: reimplementation of 0x51fa80, 0x51fd80, 0x51feb0,
// 0x51ff10, 0x51ff70, 0x51ffd0, 0x520470, 0x5222e0 and 0x522320. Nothing works before the
// particles are set up (0x816d40 set). A system handle is serial << 16 | slot: slot (u16) below
// 0x78 in gParticleSystems (0x80cefc, 0x140 bytes each, copies of an effect definition), its
// serial the system's u16 +0xa; 0 is none. A system's +0x20 is its age (-2 ends it) and +0xd4 its
// lifetime. Effectors (0x14 slots of 0x68 bytes) have their serial at 0x816504 + slot * 0x68.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5222e0: 1 when the handle names a live particle system, else 0.
u32 __cdecl Particle_IsAlive(u32 handle);

// 0x522320: 1 when the handle names a live effector (slot below 0x14), else 0.
u32 __cdecl ParticleEffector_IsAlive(u32 handle);

// 0x51fd80: the reduced-effects setting 0x80ced8 (optional effects, record byte 0xc1, are not
// spawned while it is set).
void __cdecl Particles_SetReduced(u32 reduced);

// 0x51fa80: effect definition number effect's (below 0x69) byte +0x118 cleared.
void __cdecl ParticleEffect_Clear118(u32 effect);

// 0x51feb0 / 0x51ff70: a live system's age set to -2, which ends it, / to age.
void __cdecl ParticleSystem_End(u32 handle);
void __cdecl ParticleSystem_SetAge(u32 handle, u32 age);

// 0x51ff10 / 0x51ffd0: a live system's u16 +0x2a / +0xa2 set.
void __cdecl ParticleSystem_Set2a(u32 handle, u32 value);
void __cdecl ParticleSystem_SetA2(u32 handle, u32 value);

// 0x520470: system slot's signed byte for the quarter of its life it is in (age * 4 / lifetime,
// signed): +0x6b in the first, +0x6a, +0x69, else +0x68 (also with no lifetime). The original
// faults dividing INT_MIN by -1.
s32 __cdecl ParticleSystem_QuarterByte(u32 slot);
}
