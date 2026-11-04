// The park's gates (the arrival rate, the bus and whether the gates are open): reimplementation of their serializer 0x4cf050, their step 0x4cf3e0, the size of an arrival 0x4cf5b0 and the visitor brought
// 0x4cf720. The field names are the save's; the functions' names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct Stream;

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct ParkGatesEnv {
    u32(__fastcall *since)(const u32 *stamp);                          // Game1da70c_MinusAt0Quarters
    u32(__fastcall *stamp)(u32 *out);                                  // Out_SetFromGame1da70c
    u32(__fastcall *table)(u8 *world);                                 // GameTable7cfb90_Get1da720_519510
    u32(__fastcall *record)(u8 *world);                                // GameTable7cfb90_Get1da724_519590
    s32(__fastcall *take)(u8 *world);                                  // Game_TakeRideElement1
    u32(__fastcall *blocked)(u8 *world);                               // World_BusStopBlocked
    void(__fastcall *arrival)(u8 *world, void *, u8 *out, u32 kind);   // 0x51a2f0 (agent A's)
    u32(__fastcall *notSize1)(const u8 *world);                        // World_ArrivalVehicleNotSize1
    u16 *(__fastcall *busStop)(const u8 *map, void *, u16 *out, u32 b);   // Map_BusStopPos
    u8 *(__fastcall *visitorCtor)(u8 *visitor, void *, u32 cell);      // Visitor_Ctor
    u32(__fastcall *count)(u8 *analyser, void *, u32 maxAge);          // ParkAnalyser_CountKind1OnPlainCell
    s32(__fastcall *upkeep)(void *analyser);                           // RideUpkeep_Total
    u8 *(__fastcall *readMode)(u8 *setting);                           // Setting_ReadMode
    s32(__cdecl *atExit)(u32 f);                                       // 0x67a7d0
};
void ParkGates_SetEnv(const ParkGatesEnv *env);

extern "C" {

// 0x4cf050 (thiscall; stream, save, version, unused; pops 16): 0 for versions other than 0 and 1. Otherwise as a serializer does
// (Save_Field; the last field's load through Save_ReadU8): mArrivalRate (+0, 4 bytes), then the time stamp at +4 (TimeSig_Serialize, given
// the same arguments), mTargetVehicleCapacity (+8, 4), mPeopleOnBus (+0xc, 4), mOffloading (+0x10, 1) and mGatesOpen (+0x11, 1). 1 at the
// first that fails (the debug print left out), else 0.
u32 __fastcall ParkGates_Serialize(u8 *gates, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4cf5b0 (thiscall; `this` not read): 0 with the world's +0x1da738 at 4. Else the people on plain cells (age 0x14) P of the analyser; the analyser's RideUpkeep_Total plus [0x78531c], times 0.8 when the
// record (Get1da724)'s +0x30 is positive, else 1.2 (x87, __ftol), over [0x785320] (at least 1), and at least [0x785310]; the mode setting (0xfb3b7c, the guard 0xfb34c2 & 1: Setting_ReadMode, atexit 0x5d1750)
// 1 holds size + P (unsigned) to 0x1f4 and any other to 0x5dc.
s32 __fastcall ParkGates_ArrivalSize(u8 *gates);

// 0x4cf720 (thiscall; `this` not read; returns 1): the bus stop's position (Map_BusStopPos of [0x8023a0], b 1), less 0x100 with World_ArrivalVehicleNotSize1, and a visitor of 0x22c bytes there (Visitor_Ctor,
// when allocated).
u32 __fastcall ParkGates_SpawnVisitor(u8 *gates);

// 0x4cf3e0 (thiscall; returns 0): closed (+0x10 0) and past the wait [0x785314] since the stamp +4 (quarters, unsigned): +0xc = ParkGates_ArrivalSize, +0x10 = 1, and the size in the analyser's ring +0x216d8.
// Open: Game_TakeRideElement1 -1 or 6: 0x51a2f0 of 1 (up to 0x23 people), 2 (up to 0x3c) or 3. 2: with people, one visitor brought (ParkGates_SpawnVisitor), +0xc less one and the analyser's +0x20cc0 one
// more; with none, the stamp set, +0x10 = 0 and 0x51a2f0 of 0. Then, with World_BusStopBlocked, the state (taken anew) -1, 0, or 2 with no people left: 0x51a2f0 of 0; without, the state 4: the same.
u32 __fastcall ParkGates_Step(u8 *gates);
}
