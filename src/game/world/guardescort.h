// A guard sent after a visitor (the handle +0x212, the countdown +0x214) and walking it out: reimplementation of 0x4d6260, 0x4d6790 and 0x4d6a50. The names are invented.
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default; the bench's recorders: their own benches have them).
struct GuardEscortEnv {
    u32(__fastcall *tooTired)(const u8 *staff);                              // Staff_IsTooTired
    void(__fastcall *tire)(u8 *staff);                                       // Staff_TireB
    u32(__fastcall *walkNav)(u8 *sprite);                                    // Sprite_WalkNav
    u32(__fastcall *setDestination)(u8 *sprite, void *, u32 cell);           // Sprite_SetDestinationCell
    u32(__fastcall *isCaught)(const u8 *visitor);                            // Visitor_IsState6To10
    void(__fastcall *tally)(u8 *analyser, void *, u32 flag);                 // ParkAnalyser_Tally
    u32(__fastcall *playSound)(const u8 *sprite, void *, u32 category, u32 sound);   // Sprite_PlaySound
    void(__fastcall *killScripts)(u8 *visitor);                              // Visitor_KillScripts
    u16 *(__fastcall *boothPos)(const u8 *map, void *, u16 *out, u32 b);     // Map_TicketBoothPos
    u16 *(__fastcall *entrancePos)(const u8 *map, void *, u16 *out, u32 b);  // Map_EntrancePos
    void(__fastcall *setState)(u8 *guard, void *, u32 state);                // Guard_SetState
    void(__fastcall *arrive)(u8 *visitor, void *, u32 cell);                 // Visitor_Arrive
};
void GuardEscort_SetEnv(const GuardEscortEnv *env);

extern "C" {

// 0x4d6260 (thiscall (handle), pops 4): nothing when Staff_IsTooTired; else the sound 0x88 ([0x803a30]), +0x212 the handle, the destination the thing's cell ((+7 << 7) + +5 + 1); found: Guard_SetState (0x10).
void __fastcall Guard_Send(u8 *guard, void *, u32 handle);

// 0x4d6790 (thiscall): Staff_TireB. With +0x212 and +0x214 (counted down), one step (Sprite_WalkNav): 0 away from the visitor: the destination its cell, found: done for now; not found: given up. 2: given up. On the visitor's cell (from 0 or any other answer, away: nothing more): not Visitor_IsState6To10: given up;
// else +0xc2 = [0x75c7f0], +0xc's +4 = 0x16, ParkAnalyser_Tally (2) of the game's record, the sound 0x89, Visitor_KillScripts, the destination the ticket booth (Map_TicketBoothPos of [0x8023a0], [0x80239c]'s +0x1da70c & 1): found: Guard_SetState (0x12); else +0x188 = 1 and again; else Visitor_Arrive (the visitor, the guard's cell), given up. Given up (also with no handle or countdown): ParkAnalyser_Tally (0), +0xc2 = [0x75c7f0], Guard_SetState (0), +0x212 = 0.
void __fastcall Guard_Chase(u8 *guard);

// 0x4d6a50 (thiscall): one step; at its end (0 or 2): +0xc's +4 = 0x18, Visitor_Arrive (the visitor +0x212, the guard's cell), +0x188 = 0, the destination the entrance (Map_EntrancePos): found: Guard_SetState (0x13), else (0).
void __fastcall Guard_LeaveWithVisitor(u8 *guard);
}
