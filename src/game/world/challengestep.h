// The challenges' step (thing model 19, world/challenges): reimplementation of 0x4d1e90 (name invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct ChallengeStepEnv {
    s32(__cdecl *atExit)(void(__cdecl *f)());                       // 0x67a7d0
    u8(__fastcall *met)(u8 *challenge, void *, s32 *value, u32 flag);   // Challenge_Met
    void(__fastcall *won)(u8 *challenges);                          // Challenges_Won
    void(__fastcall *lost)(u8 *challenges);                         // Challenges_Lost
    void(__fastcall *offerNext)(u8 *challenges, void *, u32 offer); // Challenges_OfferNext
    u32(__fastcall *post)(u8 *centre, void *, u8 *msg);             // MessageCentre_Post
};
// nullptr restores the game's.
void ChallengeStep_SetEnv(const ChallengeStepEnv *env);

extern "C" {

// 0x4d1e90 (thiscall): the mode setting (the guard 0xfb34c3 bit 0, Setting_ReadMode, atexit 0x5d1750) not 0: nothing. The system off (+0x3d8) waits for the world's flag (GameTable7cfb90_Get1da720_519510 looked up, unused; Game_Field1da710IsZero) and the days
// elapsed (FunnyTime_Elapsed, 64-bit signed, the low dword signed) reaching gBalanceLevel's 0x7857c8: then it is on and the next is offered (Challenges_OfferNext (1)). Then a challenge on (+0x3d9), with a current one:
// on a world tick multiple of 4 message 0x19 posted: a = Kind_ToIdB of it, b = 0x160, c = 0, d = +0x14, e = +4, f = +8, g = the days to the next event (+0x3d0), h = the challenges' +0x3dc, i / j = +0xc / +0xe, k = +0x10. Other ticks: the time past (now > the event): Challenge_Met (&+0x3dc, 1) gives Won or Lost; no current one then: nothing (a debug print left out).
// The time not past: Challenge_Met (&+0x3dc, 0) gives Won; else with +4 set and days left * 100 / +4 (signed) below gBalanceLevel's 0x7857d0: message 0x19 with a = 0x17f, b = 0x10f and the rest 0 posted. No challenge on and none offered (+0x3da): the time past: Challenges_OfferNext (0).
void __fastcall Challenges_Step(u8 *challenges);
}
