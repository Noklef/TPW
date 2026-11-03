// A staff member's step by its state (+0x19c): the guard's, handyman's, entertainer's and mechanic's. Reimplementations of 0x4d6410, 0x4d73c0, 0x4d4810 and 0x4da490; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x5056e0, 0x5061d0, 0x506a40 and 0x4f9490 agent A's, by address until ported; the bench's recorders: their own benches have them).
struct StaffThinkEnv {
    s32(__fastcall *rand)(u8 *world);                                            // World_Rand
    u32(__fastcall *playSound)(const u8 *sprite, void *, u32 category, u32 sound);   // Sprite_PlaySound
    u32(__fastcall *walk)(u8 *sprite);                                           // Sprite_WalkNav
    void(__fastcall *tireA)(u8 *staff);                                          // Staff_TireA
    void(__fastcall *tireB)(u8 *staff);                                          // Staff_TireB
    void(__fastcall *toRest)(u8 *staff);                                         // Staff_WalkToRestArea
    void(__fastcall *atRest)(u8 *staff);                                         // 0x5061d0 (agent A's)
    void(__fastcall *other)(u8 *staff);                                          // 0x5056e0 (agent A's)
    u32(__fastcall *busy)(u8 *staff);                                            // 0x506a40 (agent A's)
    u32(__fastcall *patrol)(u8 *staff);                                          // 0x4f9490 (agent A's)
    void(__fastcall *handymanWork)(u8 *handyman);                                // Handyman_PickWork
    void(__fastcall *toLitter)(u8 *handyman);                                    // Handyman_WalkToLitter
    void(__fastcall *sweep)(u8 *handyman);                                       // Handyman_SweepLitter
    void(__fastcall *toJob)(u8 *handyman);                                       // Handyman_WalkToJob
    void(__fastcall *resetCondition)(u8 *object);                                // Object_ResetCondition
    void(__fastcall *close)(u8 *object);                                         // Object_Close
    u32(__fastcall *set4)(u8 *o, void *, u32 v0);                                // Set4_4217f0
    void(__fastcall *handymanState)(u8 *handyman, void *, u32 state);            // Handyman_SetState
    void(__fastcall *guardState)(u8 *guard, void *, u32 state);                  // Guard_SetState
    void(__fastcall *chase)(u8 *guard);                                          // Guard_Chase
    void(__fastcall *leave)(u8 *guard);                                          // Guard_LeaveWithVisitor
    u16 *(__fastcall *entrance)(const u8 *map, void *, u16 *out, u32 b);         // Map_EntrancePos
    u32(__fastcall *destination)(u8 *sprite, void *, u32 cell);                  // Sprite_SetDestinationCell
    void(__fastcall *entertainerWork)(u8 *entertainer);                          // Entertainer_PickWork
    void(__cdecl *position)(s32 slot, u32 *x, u32 *y, u32 *z, u32 *r15);         // SpriteScript_GetPosition
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);   // Sound_PlayAt
    void(__fastcall *mechanicWork)(u8 *mechanic);                                // Mechanic_PickWork
    void(__fastcall *toObject)(u8 *mechanic);                                    // Mechanic_WalkToObject
    void(__fastcall *repair)(u8 *mechanic);                                      // Mechanic_Repair
};
void StaffThink_SetEnv(const StaffThinkEnv *env);

extern "C" {

// All thiscall (names invented, unconfirmed). In each: state 1 a sound 1 time in 16 (category [0x803a30]) while walking (Sprite_WalkNav not 0 or 2: Staff_TireA) and on arrival the work; state 0 the same sound and, past the wait (the dword at 0x785330 + the level +0x1e4 * 16, over +0x200), the work; states 2, 3 Staff_WalkToRestArea, 0x5061d0; a state of no case 0x5056e0.
// 0x4d6410: the guard (sounds 0xa6, 0xa7). The work: nothing while 0x506a40 answers, state 0 on 1 tick in 4, else 0x4f9490's answer as the state 1 or 0. 0x10 Guard_Chase, 0x12 Guard_LeaveWithVisitor, 0x13 on arrival the entrance (the tick's bit 0) as the destination, its answer as the state 1 or 0.
void __fastcall Guard_Step(u8 *guard);
// 0x4d63d0: the guard's work alone (its copy inlined in Guard_Step).
void __fastcall Guard_PickWork(u8 *guard);
// 0x4d73c0: the handyman (sounds 0xa0, 0xa1; Handyman_PickWork). 8 Handyman_WalkToLitter, 9 Handyman_SweepLitter, 0xa Handyman_WalkToJob, 0xb Staff_TireB and, past the wait (0x7853ec + level * 12, over +0x214), the object of the handle +0x21a reset and closed, +0xc's 3 set and state 0.
void __fastcall Handyman_Step(u8 *handyman);
// 0x4d4810: the entertainer (sounds 0xa4, 0xa5; Entertainer_PickWork). 0xe Staff_TireB and, past the wait (0x785398 + level * 16, over +0x214), the sound 0x87 at its sprite's place and the work.
void __fastcall Entertainer_Step(u8 *entertainer);
// 0x4da490: the mechanic (sounds 0xa2, 0xa3; Mechanic_PickWork). 0xc Mechanic_WalkToObject, 0xd Mechanic_Repair.
void __fastcall Mechanic_Step(u8 *mechanic);
}
