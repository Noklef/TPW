// The vehicle that brings the visitors. Reimplementation of 0x51a2f0; the name is invented, unconfirmed (the original's debug prints say "the bus or the plane" and "vehicle_name").
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct ArrivalVehicleEnv {
    u8 *(__fastcall *construct)(u8 *block, void *, u32 cell, u32 id, s32 angle, u32 ride);   // 0x4db090 (agent C's object constructor, pops 0x10)
    u8 *(__cdecl *findRide)(s32 id);                                                         // Rides_FindById
    void(__cdecl *setElement)(u8 *ride, s32 index, u32 value);                               // Rides_SetElement
};

void ArrivalVehicle_SetEnv(const ArrivalVehicleEnv *env);

// 0x51a2f0 (thiscall on the game object (out, which), pops 8; an SEH frame, dropped; the empty Debug_Printf left out): *out = the word +0x1da72a, the handle of the vehicle. Set already: element 0 of
// its ride record (the thing table 0x7cfb90, 20 bytes a row, its +0x24) = 1. Not set: the kinds tried in turn, the first by which (0 the game's random number: its absolute value modulo 3; else
// which - 1: other than 0, 1, 2 reads address 0, sic), the rest in the order 2, 1, 0: kind 0 the name 0x802370 and the word +0x1da72c, 1 0x801d58 and +0x1da72e, 2 0x7cf3f0 and +0x1da730. A word already set:
// that handle is the vehicle, element 0 = 1. Not set: the object definition of that name (wcsicmp, +0x390): none, the next kind; found, a new object (0x450 bytes, not checked; cell 1, the
// definition's id) whose handle goes to the word and the vehicle, element 1 = 1. All kinds tried: the handle stays 0. Returns out.
u16 *__fastcall Game_ArrivalVehicle(u8 *game, void *, u16 *out, u32 which);
