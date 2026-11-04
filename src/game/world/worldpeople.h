// The world's people at the start and at each tick: the staff pool (+0), the calendar clock (+0x2a0), the unlocks (+0x2c0), the park gates (+0x2c4). Reimplementation of 0x4d7af0 and 0x4d7b20; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct WorldPeopleEnv {
    void(__fastcall *staffStart)(u8 *pool);     // StaffPool_Start
    void(__fastcall *clockSync)(u8 *clock);     // FunnyTime_SyncMonthYear
    u32(__fastcall *gatesStep)(u8 *gates);      // ParkGates_Step
    void(__fastcall *staffUpdate)(u8 *pool);    // StaffPool_Update
    void(__fastcall *clockUpdate)(u8 *clock);   // FunnyTime_Update
    void(__fastcall *unlockStep)(u8 *self);     // ObjectUnlock_Step
};
void WorldPeople_SetEnv(const WorldPeopleEnv *env);

extern "C" {

// 0x4d7af0 (thiscall): StaffPool_Start, FunnyTime_SyncMonthYear. The original's call of the empty 0x5da3c0 left out.
void __fastcall WorldPeople_Start(u8 *world);

// 0x4d7b20 (thiscall): the gates' step, StaffPool_Update, FunnyTime_Update; with the ticks [[0x7cf6ec]] +0x1da70c a multiple of 100 (unsigned), ObjectUnlock_Step.
void __fastcall WorldPeople_Step(u8 *world);
}
