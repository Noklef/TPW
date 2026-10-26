// The lobby's models (the front end's globe and its scenery, 0x5d84a0 on): an object whose +4
// is an index in the 0x7a4610 table (0 for none) and +8 a flag set once 0x5d8130 has set the
// model up. Reimplementation of its methods 0x5d8190 and 0x5d8380-0x5d8440 (thiscall).
//
// The model's matrix is the entry's +8 -> +4 -> +0x78, +0x10 (as Holder_SetScale finds it).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d8190 (v): the matrix's translation (+0x30) = v[0], v[1], v[2] (each through the x87,
// read before any is stored).
void __fastcall LobbyModel_SetPosition(const u8 *model, void *, const float *v);

// 0x5d8380 (angle): the matrix turned about y by angle (Matrix_RotateEulerFast, order "Y").
void __fastcall LobbyModel_TurnY(const u8 *model, void *, float angle);

// 0x5d83b0 (x, y, z): Holder_SetScale of the entry's +8 by (x, y, z).
void __fastcall LobbyModel_SetScale(const u8 *model, void *, float x, float y, float z);

// 0x5d83f0 (entry, now): when there is a model, Animator_Play(entry of bank 5, mode now ? 1 :
// 0, speed 1, layer 0) and the ms until it ends; else 0.
u32 __fastcall LobbyModel_Play(const u8 *model, void *, u32 entry, u32 now);

// 0x5d81c0 (v; name unconfirmed): the matrix made the identity, then its first row (z, 0, -x) of v over its length, its second
// that row crossed with v over its length and its third v itself (its fourth left the identity's); the lengths stored as floats.
void __fastcall LobbyModel_AlignTo(const u8 *model, void *, const float *v);

// 0x5d8440: when there is a model, the ms left of its layer 0 (Layer_GetPlayback); else 0.
u32 __fastcall LobbyModel_TimeLeft(const u8 *model);

// 0x5d9b50 (thiscall; name unconfirmed): one step of a lobby flyer (the members of the list +0x58 of the lobby class 0x702af0, each
// stepped in turn at 0x5da0fa), x87 throughout. The wanted heading (+0x54) = the target (+0x3c) less the position (+0x30), over its
// length; the heading (+0x48) turned toward it by [0xf82884] +8 (the frame time) * 0.1 and made unit again; the position moved along it
// by the speed (+0x28) * the frame time. Within reach of the target (the squared distance below +0x2c, or unordered) a new target: per
// axis the centre (+0x10) plus the extent (+0x1c) * (r * 2^-30 - 0.5), r the scene's generator ([0xf82884] +0x34: * 0x19660d +
// 0x3c6ef35f, turned right 13) & 0x3fffffff. Then the lobby model at +8 aligned to the heading (LobbyModel_AlignTo) and put at the
// position (LobbyModel_SetPosition).
void __fastcall LobbyFlyer_Step(u8 *flyer);
}
