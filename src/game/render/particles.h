// The particle systems' handle accessors: reimplementation of 0x51fa80, 0x51fd80, 0x51fe30, 0x51fd90, 0x51feb0,
// 0x520030, 0x5200b0, 0x5204c0, 0x521810, 0x521850, 0x51ff10, 0x51ff70, 0x51ffd0, 0x520470, 0x5222e0, 0x522320, 0x520e00, 0x520560, 0x521e60, 0x5214a0 and 0x521930. Nothing works before the
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

// 0x51fe30 / 0x520030 (handle, x, y, z): a live system's three dwords at +0x14 / +0x44 set to the
// arguments divided by 16 (arithmetic shift).
void __cdecl ParticleSystem_SetVectorA(u32 handle, s32 x, s32 y, s32 z);
void __cdecl ParticleSystem_SetVectorB(u32 handle, s32 x, s32 y, s32 z);

// 0x51fd90 (handle, x, y, z): a live system's dwords at +0x38, +0x3c, +0x40 set to the arguments
// times its +0xb0, over 1024 (rounded towards zero).
void __cdecl ParticleSystem_SetScaled(u32 handle, s32 x, s32 y, s32 z);

// 0x5200b0: with the particles set up, every system on the live list (the slot at 0x816d1c, each
// one's next in the word at +0xd0, below 0 ends it) whose byte +5 is set gets age -2 and its
// word +0x2a = 1; and every effector on its list (0x816d1e, next in +0x60) whose byte +0x45 is set
// gets +0x4c = -2.
void __cdecl Particles_EndAll();

// 0x5204c0 (slot; negative does nothing): takes the system off the live list (head 0x816d1c; the
// next in the word +0xd0 and the previous in +0xd2, negative for none), clears its words +0 and
// +0xa (the serial) and puts it on the free list (head: the word 0x816d20; the system's +0xd0 =
// the old head, +0xd2 = -1).
void __cdecl ParticleSystem_Release(s32 slot);

// 0x521810 / 0x521850 (cdecl; n): the particles' own random numbers. The seed at 0x80cee0 steps (seed *
// 214013 + 2531011) and the result is (seed >> 16, arithmetic) / (seed >> 16 & 0x7fff) modulo n, a
// signed remainder; 0 for n = 0 (the seed then stays).
s32 __cdecl Particle_RandSigned(s32 n);
s32 __cdecl Particle_Rand(s32 n);

// 0x51ff10 / 0x51ffd0: a live system's u16 +0x2a / +0xa2 set.
void __cdecl ParticleSystem_Set2a(u32 handle, u32 value);
void __cdecl ParticleSystem_SetA2(u32 handle, u32 value);

// 0x520470: system slot's signed byte for the quarter of its life it is in (age * 4 / lifetime,
// signed): +0x6b in the first, +0x6a, +0x69, else +0x68 (also with no lifetime). The original
// faults dividing INT_MIN by -1.
s32 __cdecl ParticleSystem_QuarterByte(u32 slot);

// 0x520e00 (cdecl; particle): one step of a particle's motion. Each velocity component (+0x2c, +0x30, +0x34) loses
// v * drag / 1024 (drag the s16 +0x24, truncated toward 0), the y one also the s16 gravity +0x28; the position (+0x14,
// +0x18, +0x1c) moves by it. When the s16 +0x76 is set, the ground height under (x, z) is the callback 0x80cef4
// (x << 4, z << 4) >> 4; a particle below it bounces: when y - ground is below the y velocity it steps back by its x
// and y velocity (z stays) and keeps half its y velocity with x and z halved and reversed, else it is put on the
// ground with its y velocity halved and reversed and x, z halved (all halving truncated toward 0).
void __cdecl Particle_Move(u8 *p);

// 0x520560 (cdecl; slot): emits a particle from system slot: a record taken off the free list (FreeList_Take; its index is returned, negative when none is free)
// at [0x80cef8] + index * 0x34. The emit count 0x80ced0 and the system's word +8 count up; the record's words +0x24 = 2, +0x26 the slot, +0x28 the system's +0x74,
// +0x2a its signed byte +4, +0x2c = 0, +0x2e a random 15 bits when the byte +0x6d is set (else 0); its lifetime +0x20 (+0x1c too, 1 for 0) +0x78 plus
// RandSigned(+0x78 >> 2); its +0x18 one of the 16 dwords at +0xd8 (Rand(16)) when +0x58 is set, else +0x114. The position (+0, +4, +8): the system's (+0x14)
// when its extents +0x44/+0x48/+0x4c are all 0; else a point in a box (byte +0xc2: RandSigned of each extent; y Rand when the word +0xa0 is set) or at a
// random angle (12 bits) on an ellipse's edge (word +0xa8) or within it, x by the cosine and z by the negated sine (the sine table at [0x80ceec], angles in
// eighths of an entry, products over 256), turned by the angle +0xac when set, plus the system's. A push (the s16 +0xc4) along the angle +0x54 adds to x and
// z. The velocity (+0xc): with +0x5c set a random angle and Rand(+0x5c) (y RandSigned(+0x5c)), else +0x38 plus RandSigned of the s16 +0x50 for each; plus
// the system's +0x2c when the byte +0x6c is set. The random numbers are Particle_Rand / Particle_RandSigned's, a step of the seed for each angle.
s32 __cdecl Particle_Emit(u32 slot);

// 0x521e60 (cdecl; effect, x, y, z): starts a particle system of effect definition `effect` (0x804370 + effect * 0x140) at x, y, z (over 16). -1 before the set-up,
// for an effect outside 0 to 0x68 or with no free system; 0 for an optional effect (byte +0xc1) while the effects are reduced (0x80ced8). The system at the free
// head (0x816d20; links: words +0xd0 next, +0xd2 previous) moves to the head of the live list (0x816d1c) and takes the definition's 0x140 bytes (its links
// kept); Voice_ScaleLevels unless its byte +0xc0 is set; its lifetime +0xd4 = +0x20; the bounds +0x128 to +0x13c empty (0x7fffffff three times, 0x80000000
// three times; zeros with the byte +5); words +8 = 0, +0xc = the effect, +0 = 1; the velocity +0x2c gets RandSigned of the s16 +0x12 in each component; the
// serial +0xa from the u16 0x80cee8 (then incremented); +6 = -1, byte +0xab = 0, byte +0x6f whether any of +0x84/+0x88/+0x8c is set. With the word +0xe of
// 1 its +0 = 1 and +0xcc = 0; with +0xe 0 and +0 above 0 it emits +0x60 particles (Particle_Emit). A link +0xb4 (not negative) starts a voice (byte +0xba:
// Voice_Start, at the offset of the voice template's +0x18 with the word +0xb8; then the byte +0xa of the instance the whole handle indexes is set, sic)
// or the linked effect (Particle_Spawn, at its definition's +0x2c offset with +0xb8, -1 when the definition links itself; then the new system's +0xab is
// set), the result kept in +0xb4. Returns the handle (serial << 16 | slot).
s32 __cdecl Particle_Spawn(s32 effect, s32 x, s32 y, s32 z);

// 0x521930 (cdecl; effect, x, y, z, vx, vy, vz; name unconfirmed): Particle_Spawn with a velocity: the system's +0x38/+0x3c/+0x40 = vx, vy, vz times its
// +0xb0 over 1024 (truncated toward 0) in place of the jitter of +0x2c. (Its linked effect is started by Particle_Spawn.)
s32 __cdecl Particle_SpawnWithVelocity(s32 effect, s32 x, s32 y, s32 z, s32 vx, s32 vy, s32 vz);

// 0x5214a0: one step of every particle of each live system (from 0x816d1c, next in the word +0xd0) whose word +2 is 0, along its record list (the word +6,
// next in the record's +0x30). Each record's age +0x20 counts down; below 0 it ends: the emit count 0x80ced0 and its system's (the record's +0x26) word +8
// count down, its system's death effect +0x98 (not negative) is spawned at its position (times 16) and it goes back on the free list (FreeList_Return).
// Otherwise (age -1 when the system is ending, +0x20 negative, and its byte +0xaa set): the sprite +0x18 (one of +0xd8's 16 at random for an odd +0x58,
// else by age * 15 / life +0x1c when +0x78 is above 0, else the first), the frame +0x2c counting down from the word +0x96 over the life (0 past it), the
// size +0x28 from +0x74 at the start to +0xa6 at the end (+0x74 without +0x78), the colour +0x2e plus +0xbe (plus Rand(+0xc6 - +0xbe) when they differ),
// a random kick (byte +0x6f: RandSigned of +0x84/+0x88/+0x8c), drag (v * +0x7c / 1024 off each component), gravity (+0x80 off y); a fresh particle (+0x24
// of 2) becomes 1 and stays, others move by the velocity. Unless the system's byte +5 is set its bounds (+0x128 least, +0x134 most) take the particle's
// position less / plus its size / 1024. (The original's print of a frame past the count is left out.)
void __cdecl Particles_Update();
}
