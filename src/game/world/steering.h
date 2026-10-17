// The steering behaviours a nav (world/nav) combines: seek, arrival, separation, avoid_obstacle, repel_obstacle,
// avoid_walls and follow_path. Each is one global object (vtable at +0, its name at +4: the strings at 0x761ee0-0x761f28),
// built by a static initializer and destroyed at exit by the base destructor; slot 0 of each vtable steers (thiscall
// (&out, nav), out a 16.16 force). Reimplementation of the constructors 0x50ce50-0x50d100, the base destructor 0x50cea0,
// the exit thunks that call it, seek's and repel_obstacle's steering (0x50dd80, 0x50e260) and repel's per-neighbour
// callback (0x50e190).
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
}
