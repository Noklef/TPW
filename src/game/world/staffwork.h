// What an idle handyman, mechanic or entertainer does next: reimplementation of 0x4d7100, 0x4da5b0 and 0x4d46d0. The names are invented. (The original's debug prints are left out.)
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default, 0x506a40 and 0x4f9490 by address; the bench's recorders: their own benches have them).
struct StaffWorkEnv {
    u32(__fastcall *busy)(u8 *staff);                                                       // 0x506a40
    u32(__fastcall *tooTired)(const u8 *staff);                                             // Staff_IsTooTired
    u32(__fastcall *randomDest)(u8 *person);                                                // 0x4f9490
    u16 *(__fastcall *nearestCell)(u8 *world, void *, u16 *out, u32 type, u32 cell, s32 radius);   // World_NearestCellWithType
    u16 *(__fastcall *findJob)(u8 *staff, void *, u16 *out);                                // Handyman_FindJob
    u32(__fastcall *setDestination)(u8 *sprite, void *, s32 x, s32 y);                      // Sprite_SetDestination
    u32(__fastcall *stampCell)(u8 *cell, void *, u32 word);                                 // Obj_StampAt30
    void(__fastcall *handymanState)(u8 *handyman, void *, u32 state);                       // Handyman_SetState
    u16 *(__fastcall *nearestFree)(u8 *self, void *, u16 *out, u32 person);                 // Objects_NearestFreeFor
    u32(__fastcall *setDestinationCell)(u8 *sprite, void *, u32 cell);                      // Sprite_SetDestinationCell
    u32(__fastcall *stampObject)(u8 *object, void *, u32 word);                             // Obj_StampAt5e
    void(__fastcall *mechanicState)(u8 *mechanic, void *, u32 state);                       // Mechanic_SetState
    void(__fastcall *staffState)(u8 *staff, void *, u32 state);                             // Staff_SetState
    s32(__stdcall *anyAround)(u32 kind, u32 cell, s32 radius);                              // Things_AnyOfKindAround
    u32(__cdecl *actionCount)(u32 folder, u32 sprite);                                      // Sprite_ActionCount
};
void StaffWork_SetEnv(const StaffWorkEnv *env);

extern "C" {

// 0x4d7100 (thiscall): nothing while 0x506a40 answers. Not too tired (Staff_IsTooTired): +0x218 the nearest cell (World_NearestCellWithType on the game's record, type the
// handyman's handle, about his cell, the radius of the table 0x7853f0 by +0x1e4). A cell: the world's generator seeded with it, a point in it two draws & 0x7f clamped to
// 1-9 times 13, in 1/256 cells (bytes; the upper halves are the stack's in the original); sent there with the answer 1: the cell stamped (Obj_StampAt30, handle), state 8.
// No cell: +0x21a the next job (Handyman_FindJob), one: state 0xa. All else (too tired too): state 1 when 0x4f9490 answers, else 0 (Handyman_SetState).
void __fastcall Handyman_PickWork(u8 *handyman);

// 0x4da5b0 (thiscall): nothing while 0x506a40 answers. Not too tired: +0x218 the nearest free object (Objects_NearestFreeFor on the world's +0x1da71a record). One the
// mechanic can be sent to (+0x36, Sprite_SetDestinationCell): stamped (Obj_StampAt5e, his handle); with neither its +0x64 nor +0x19c 2, the advisor message 0x46 (AdvisorMsg_Init:
// type the object, a its word +0xe, b -1, flag 0, no score; on the stack, bytes +2, +3 and +0x17 never written) posted (Advisor_Post on the world's +0x1da71c record); state 0xc.
// All else: state 1 when 0x4f9490 answers, else 0 (Mechanic_SetState).
void __fastcall Mechanic_PickWork(u8 *mechanic);

// 0x4d46d0 (thiscall): nothing while 0x506a40 answers. Not too tired, a draw that divides by 3 and things of kind 1 around his cell (Things_AnyOfKindAround, the radius of the
// table 0x7853a0 by +0x1e4; the game's record asked first for the stdcall's ECX, left out): a draw, the action 0xd + that draw modulo Sprite_ActionCount - 1 (one action: 0xd; unsigned)
// into +0xc's +4, the state 0xe, +0x214 the world's tick. All else: on a tick with low two bits set, when 0x4f9490 answers, a draw (unused), state 1 (Staff_SetState, then +0x19c);
// when it does not, nothing; on the other ticks a draw (unused), state 0.
void __fastcall Entertainer_PickWork(u8 *entertainer);
}
