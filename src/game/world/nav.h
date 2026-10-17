// A moving person's steering and pathing state (Nav, the sprite's +0xd4, serialized at 0x50d150):
// reimplementation of 0x5100e0, 0x510160, its constructor 0x50ffe0 and destructor 0x50f200 (thiscall). +4 radius, +8 pos and +0x10 vel (each an
// x, y pair, 16.16 fixed point), +0x18 max_force, +0x1c max_speed.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x5100e0 (x, y, vx, vy): pos and vel set.
void __fastcall Nav_SetPosVel(u8 *nav, void *, u32 x, u32 y, u32 vx, u32 vy);

// 0x510160 (&x, &y): pos read (x then y).
void __fastcall Nav_GetPos(const u8 *nav, void *, u32 *x, u32 *y);
// 0x50ffe0 (thiscall): constants and zeroes into the first 0xbc bytes (the final values: +0 0x10000, +4 and +0x1c 0x3333, +0x18 0x6666, the byte at +0x60 1; +0x48 is left
// alone), and the five 8-byte elements at +0x64 constructed (IntPair_Ctor, through CRT_VectorCtorIterator). Returns the object.
u8 *__fastcall Nav_Ctor(u8 *object);

// 0x50f200 (thiscall; also reached by the jumps 0x4f7c30, with this + 0xd4, and 0x4f8ad0): the five integer pairs at +0x64 destroyed, the last first, with the
// destructor that does nothing (CRT_VectorDtorIterator, Stub_Ret); nothing else.
void __fastcall Nav_Dtor(u8 *object);

// 0x4f7c30 (thiscall; an adjustor): Nav_Dtor on the block at this + 0xd4.
void __fastcall Nav_DtorAtD4(u8 *owner);

// 0x50f280 (&out, &point): point - pos in the nav's local frame (16.16 products, local_xaxis at +0x20, local_yaxis at
// +0x28): out = (d . xaxis, d . yaxis), y written first. Returns out.
s32 *__fastcall Nav_WorldToLocal(const u8 *nav, void *, s32 *out, const s32 *point);

// 0x50f870 (&out, time): out = pos + vel * time (IntPair_MulFixed, IntPair_Add, through temporaries). Returns out.
s32 *__fastcall Nav_PredictPos(const u8 *nav, void *, s32 *out, s32 time);

// 0x50fd40: how far along its path the nav is, 16.16: 1.0 when the path is finished (byte +0x60 is 1) or its total
// (path_total_dist, +0xa0) is 0, else 1.0 - (+0xa4 + +0xa8 + the octagonal distance to the path point +0x5c of the
// five at +0x64) << 16 / total, one IDIV (it faults, sic, when the quotient passes 31 bits).
s32 __fastcall Nav_PathProgress(const u8 *nav);

// 0x5102f0 (cdecl (nav, radius, visit)): the people near the nav. For each map cell from (pos >> 16) - c to + c in y,
// then x, with c = (radius + 0xffff) >> 16 (arithmetic), that is on the map (Map_InBounds): the things listed in the
// cell (the u16 at +0x2b8 of the 0x44-byte cell [[0x7cf6ec]] + id * 0x44, id = (y << 7) + x + 1 cut to 16 bits; the
// next at the thing's +0xa; the thing at [0x7cfb90 + id * 20]) whose kind (+2) is 1, 4, 5, 6, 7, 8 or 0x12 and whose
// nav (+0xd4) is not this one get visit(nav, their nav) when d = dx * dx + dy * dy (the thing's s16 +4 / +6 less
// pos >> 8, 32-bit products) is not 0 and below (radius >> 8) squared (signed). It stops after the fifth visit.
void __cdecl Nav_VisitNeighbours(const u8 *nav, s32 radius, void(__cdecl *visit)(const u8 *nav, const u8 *other));
// 0x50f3b0 (thiscall; name unconfirmed): one step of the nav. The force is the sum over the behaviour list (0x7cf078 to 0x7cf07c, {weight, behaviour}
// entries) of each behaviour's slot 0 (thiscall (&out, nav)) cut to max_force (+0x18, IntPair_Truncate) times its weight (IntPair_MulFixed); the force
// cut to max_force again, IntPair_DivFixed by 1.0 (the mass), is added to vel (+0x10), which is cut to max_speed (+0x1c). The function the pointer at
// 0x762190 holds is asked (cdecl (from, to, the dword at +0xb4)) whether the map id of pos (+8, the 16.16 parts) can lead to that of pos + vel: when it says 0
// the pos is kept and the history bits (+0xb0) take a 1, else pos = pos + vel. The local x axis (+0x20) is vel normalized (IntPair_Normalize) and the
// y axis (+0x28) its perpendicular; the history bits shift once more, with a 1 when the path progress (Nav_PathProgress) is not above the last (+0xac),
// which it replaces.
void __fastcall Nav_Update(u8 *nav);

// 0x50d150 (thiscall; stream, save, version, unused): the nav saved or loaded as a serializer does (Save_Field; the load path through the
// readers): force (+0x40), formation_pos (+0x38), local_xaxis (+0x20), local_yaxis (+0x28), the dword at +0, max_force, max_speed, the
// dwords at +0xb8 and +0xb4, +0x58, +0x5c, the byte at +0x60, the dwords at +0xac, +0xb0, +0xa4, +0xa8, the pair at +0x4c, the dwords at
// +0x48, +0x54 and +0xa0, pos and radius; then the 5 subpath points (pairs from +0x64) each with its distance (dwords from +0x8c), and
// vel. Loading clears the dword at +0x34 before local_xaxis and the one at +0x30 before vel.
u32 __fastcall Nav_Serialize(u8 *nav, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
