// The steering behaviours a nav (world/nav) combines: seek, arrival, separation, avoid_obstacle, repel_obstacle,
// avoid_walls and follow_path. Each is one global object (vtable at +0, its name at +4: the strings at 0x761ee0-0x761f28),
// built by a static initializer and destroyed at exit by the base destructor; slot 0 of each vtable steers (thiscall
// (&out, nav), out a 16.16 force). Reimplementation of the constructors 0x50ce50-0x50d100, the base destructor 0x50cea0,
// the exit thunks that call it, seek's, arrival's and avoid_obstacle's steering (0x50dd80, 0x50ddf0, 0x50e350), the
// per-neighbour and per-obstacle callbacks (0x50e190, 0x50e2b0) and the circle hit (0x50cba0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x50ce50, 0x50ced0, 0x50cf40, 0x50cfb0, 0x50d020, 0x50d090, 0x50d100 (thiscall): the vtable (0x7009b8, 0x7009c0, 0x7009c4,
// 0x7009c8, 0x7009cc, 0x7009d0, 0x7009d4) at +0 and the behaviour's name copied to +4 with its terminator. Return the object.
u32 *__fastcall SteerSeek_Ctor(u32 *self);
u32 *__fastcall SteerArrival_Ctor(u32 *self);
u32 *__fastcall SteerSeparation_Ctor(u32 *self);
u32 *__fastcall SteerAvoidObstacle_Ctor(u32 *self);
u32 *__fastcall SteerRepelObstacle_Ctor(u32 *self);
u32 *__fastcall SteerAvoidWalls_Ctor(u32 *self);
u32 *__fastcall SteerFollowPath_Ctor(u32 *self);

// 0x50cea0 (thiscall): the base vtable (0x7009bc) back at +0; nothing else.
void __fastcall Steer_Dtor(u32 *self);

// 0x50ce90, 0x50cf10, 0x50cf80, 0x50cff0, 0x50d060, 0x50d0d0, 0x50d140 (the atexit entries): Steer_Dtor on the global
// behaviour (seek 0x7cf050, arrival 0x7cf0b0, separation 0x7cf0d8, avoid_obstacle 0x7cf360, repel_obstacle 0x7cf338,
// avoid_walls 0x7cf088, follow_path 0x7cf308).
void __cdecl SteerSeek_Destroy();
void __cdecl SteerArrival_Destroy();
void __cdecl SteerSeparation_Destroy();
void __cdecl SteerAvoidObstacle_Destroy();
void __cdecl SteerRepelObstacle_Destroy();
void __cdecl SteerAvoidWalls_Destroy();
void __cdecl SteerFollowPath_Destroy();

// The length of a 16.16 pair below is the one the behaviours inline: Math_ISqrt of x * x + y * y while both magnitudes
// are under 0x1000, else of the magnitudes shifted right by 8 (under 0x400000) or 18, shifted back.
// The 16.16 divisions are single IDIVs that fault (sic) on 0 or a quotient past 31 bits, as the originals'.

// 0x50dd80 (seek's slot 0; thiscall (&out, nav), this unused): out = (*target (+0x30) - pos) - vel. Returns out.
s32 *__fastcall SteerSeek_Steer(void *self, void *, s32 *out, const u8 *nav);

// 0x50ddf0 (arrival's slot 0; thiscall (&out, nav), this unused): d = *target - pos; (0, 0) when its length is 0, else
// speed = max_speed (+0x1c) if that is under length / 2.0 (16.16, the slowing distance __ftol(2.0 * 65536.0)), else
// length / 2.0; out = d * (speed / length) - vel. Returns out.
s32 *__fastcall SteerArrival_Steer(void *self, void *, s32 *out, const u8 *nav);

// 0x50e190 (cdecl (nav, other); the per-neighbour callback of repel_obstacle's slot 0, 0x50e260, which is hooked as
// TextExtent_Zero (gfx/text): it clears this force and copies it out, the visit being compiled out): away = pos - *other (a pair at other + 0); when
// its octagonal length is under nav radius + other's (+8) + 1.0, the force (0x7cf100) += away * (2^32 / Math_ISqrt(that
// length)).
void __cdecl SteerRepel_Neighbour(const u8 *nav, const u8 *other);

// 0x50cba0 (cdecl (&local, radius)): where a ray along local x first meets a circle of the radius about the local point:
// (-(sqrt(disc) - 2x)) >> 1 (logical) for disc = 4 r*r - 4 (x >> 18)^2 - 4 (y >> 18)^2 + (2x)^2 (16.16 products where the
// original takes them), or 0x80000000 when disc <= 0.
s32 __cdecl Steer_CircleHit(const s32 *local, s32 radius);

// 0x50cc20 (cdecl (&from, &dir, &centre, radius)): where the ray from + t dir first meets the circle, all 16.16: with
// d = from - centre, a = dir.dir, b = 2 dir.d, c = d.d - r*r, disc = b*b - 4 a c (16.16 products); 0x80000000 when
// disc <= 0, else the smaller of -(sqrt(disc) + b) and sqrt(disc) - b, halved (arithmetically).
s32 __cdecl Steer_RayCircleHit(const s32 *from, const s32 *dir, const s32 *centre, s32 radius);

// 0x50e2b0 (cdecl (nav, obstacle); avoid_obstacle's per-obstacle callback; obstacle: a pair, then +8 its radius): the
// obstacle in the nav's frame; when ahead (x > 0), within reach (x - radius <= 0x7cf048) and across the path
// (|y| < nav radius + radius), and its hit (Steer_CircleHit) is >= 0 and nearer than 0x7cf32c: that becomes the
// nearest and the force's y (0x7cf104) = -y.
void __cdecl SteerAvoidObstacle_Obstacle(const u8 *nav, const s32 *obstacle);

// 0x50e350 (avoid_obstacle's slot 0; thiscall (&out, nav)): the force cleared, the nearest hit 0x7fffffff, the reach
// (0x7cf048) = radius + the length of Nav_PredictPos(10.0) - pos; (the obstacles' visit is compiled out); then out = (0, 0)
// while no hit was found, else the force through Matrix2_Transform on the nav. Returns out.
s32 *__fastcall SteerAvoidObstacle_Steer(void *self, void *, s32 *out, const u8 *nav);

// 0x50dfc0 (cdecl (nav, other); separation's per-neighbour callback, other another nav): nothing when other is nav; else
// away = pos - other's pos; when its octagonal length is not 0 and under other's radius + nav radius + 1.0, the force
// (0x7cf100) += away / that length (IntPair_DivFixed: it faults, sic, on a quotient past 31 bits).
void __cdecl SteerSeparation_Neighbour(const u8 *nav, const u8 *other);

// 0x50e0e0 (separation's slot 0; thiscall (&out, nav), this unused): when the nav is at its last path point (+0x5c is
// +0x54 - 1) or its path is done (byte +0x60 not 0) and the octagonal distance from pos to that point (of those at +0x64)
// is at most radius + 1.0: out = (0, 0). Else the force is cleared, Nav_VisitNeighbours with radius * 2
// and SteerSeparation_Neighbour adds to it and out = the force. Returns out.
s32 *__fastcall SteerSeparation_Steer(void *self, void *, s32 *out, const u8 *nav);
}
