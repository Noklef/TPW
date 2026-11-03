// The handyman's and mechanic's update and work states: reimplementation of 0x4d7060, 0x4d7580, 0x4d76a0, 0x4da740 and 0x4da830. The names are invented, unconfirmed. (The original's debug prints are left out.)
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default; the bench's recorders: their own benches have them).
struct StaffStepsEnv {
    void(__fastcall *tire)(u8 *staff);                                                       // Staff_TireB
    u32(__fastcall *walkNav)(u8 *sprite);                                                    // Sprite_WalkNav
    void(__fastcall *placeAt)(u8 *thing, void *, u32 x, u32 y);                              // Thing_PlaceAt
    void(__fastcall *handymanState)(u8 *handyman, void *, u32 state);                        // Handyman_SetState
    void(__fastcall *removeLitter)(u8 *litter);                                              // Litter_Remove
    void(__fastcall *handymanWork)(u8 *handyman);                                            // Handyman_PickWork
    void(__fastcall *mechanicWork)(u8 *mechanic);                                            // Mechanic_PickWork
    void(__fastcall *mechanicState)(u8 *mechanic, void *, u32 state);                        // Mechanic_SetState
    void(__fastcall *clearElement)(const u8 *object);                                        // Obj_ClearRideElement4
    u32(__fastcall *elementPositive)(const u8 *object);                                      // Obj_RideElement7Positive
    void(__fastcall *mechanicDone)(u8 *object, void *, u32 handle);                          // Object_MechanicDone
    void(__fastcall *tick)(u8 *person);                                                      // Person_TickSpeed
    void(__fastcall *killAfter)(u8 *person);                                                 // Person_KillScript84After
    u16 *(__fastcall *nearestCell)(u8 *world, void *, u16 *out, u32 type, u32 cell, s32 radius);   // World_NearestCellWithType
    u16 *(__fastcall *tracked)(u8 *object, void *, u16 *out);                                // Obj_GetTrackedHandle
};
void StaffSteps_SetEnv(const StaffStepsEnv *env);

extern "C" {

// 0x4d7060 (thiscall): Person_TickSpeed. In state 0 (+0x19c), on the ticks [0x80239c]+0x1da70c that divide by the table 0x785330 of +0x1e4 (16 bytes each): World_NearestCellWithType on the record of +0x1da720, type his handle, about his cell, the radius of the table 0x7853f0
// of +0x1e4 (12 bytes each); a cell: Handyman_PickWork. Last Person_KillScript84After of +0x30.
void __fastcall Handyman_Update(u8 *handyman);

// 0x4d7580 (thiscall; the walk to the litter): the word +0x30 of the litter +0x218 (records of 0x44 bytes from [0x8023a0], the first number 1) not his handle: +0x218 cleared, Handyman_PickWork. Else one step (Sprite_WalkNav). Arrived (0): the litter's +0x2c 0: as before;
// else Thing_PlaceAt (his words +0x18, +0x1a), state 9 (Handyman_SetState). 2: as before. Others wait.
void __fastcall Handyman_WalkToLitter(u8 *handyman);

// 0x4d76a0 (thiscall; the sweeping): Staff_TireB. Past the time (the ticks above the table 0x7853ec of +0x1e4 (12 bytes each) + +0x214, unsigned): with +0x2c set of the litter of his cell (sic: the one at his feet, not +0x218's), Litter_Remove of the litter +0x218; then +0x218 cleared, Handyman_PickWork.
void __fastcall Handyman_SweepLitter(u8 *handyman);

// 0x4da740 (thiscall; the walk to the object): the object +0x218 (the table 0x7cfb90, 20 bytes each) not tracking his handle (Obj_GetTrackedHandle): +0x218 cleared, Mechanic_PickWork. Else one step (Sprite_WalkNav). Arrived (0): in state 1 or 2 (+0x19c) or with +0x64 set:
// state 0xd (Mechanic_SetState), else Mechanic_PickWork with +0x218 kept. 2: as before. Others wait.
void __fastcall Mechanic_WalkToObject(u8 *mechanic);

// 0x4da830 (thiscall; the repair): Staff_TireB. +0x214 set: counted down. Else the object +0x218: in state 1 (+0x19c), Obj_ClearRideElement4, and while Obj_RideElement7Positive nothing more. Then Object_MechanicDone (his handle), +0x218 cleared, Mechanic_PickWork.
void __fastcall Mechanic_Repair(u8 *mechanic);
}
