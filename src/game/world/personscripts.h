// A person's sprite script and ride timers: reimplementation of 0x4f9ed0, 0x4fdc90, 0x4fe900, 0x4fe950, 0x50be40, 0x50c0d0 and 0x50be80 (a thought's
// bubble).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f9ed0 (thiscall): the script slot at +0xc is killed and cleared (Obj_KillSlot), the one at +0x30
// too (Person_KillScript84), and for a person of kind 1 (the byte +2) whose slot at +0x210 is set
// that one is jumped to its entry point (Person_JumpScript210).
void __fastcall Person_ReleaseScripts(u8 *person);

// 0x4fe900 (thiscall; a, b, c, d, pops 16): with the script at +0x210 set, SpriteScript_SetPosition on it
// with the position +0x218, b and +0x21c and r15 = d; then +0x218 = a and +0x21c = c.
// Nothing at all when the slot is 0.
void __fastcall Person_MoveScript(u8 *person, void *, u32 a, u32 b, u32 c, u32 d);

// 0x50be40 (thiscall): when the world's clock (the dword at +0x1da70c of the world that [0x7cf6ec] points
// through) is above +0x8c + 12 (unsigned) and the script slot at +0x84 is set, it is killed and cleared.
void __fastcall Person_KillScript84After(u8 *person);

// 0x4fdc90 (thiscall on a visitor): every 20th tick of the world's clock (the dword at +0x1da70c of
// [0x80239c], unsigned remainder 0) Visitor_PushPreviousRide with a ride whose low word is 0 and whose
// high word is the visitor's address's (sic: the argument is the address with its low word cleared).
void __fastcall Visitor_PreviousRideTick(u8 *visitor);

// 0x4fe950 (thiscall): when the script slot at +0x210 is set it is sent to the entry point 0x74f4c0
// (SpriteScript_Jump) and +0x210 cleared.
void __fastcall Person_JumpScript210(u8 *person);

// 0x50c0d0 (thiscall): when the script slot at +0x84 is set it is killed (SpriteScript_Kill) and +0x84
// cleared.
void __fastcall Person_KillScript84(u8 *person);

// 0x50bab0 (thiscall): like Person_KillScript84 but +0x84 is left as it is.
void __fastcall Person_KillScript84Keep(u8 *person);

// 0x4f9e60 (thiscall (x, y, z, r15, anim), pops 20; name unconfirmed): the person's script reference at +0xc, when its slot is 0, started
// at entry point anim (ScriptRef_Start with the folder +0x24 and sprite +0x20); then the script placed (SpriteScript_SetPosition) at x,
// y less the ground height at (x, z) (Terrain_HeightAt; fsubr, stored as a float), z, with r15; then, unless it is at that entry point
// (ScriptRef_IsAt), the reference's +4 = anim (Set4_4217f0). Returns 0.
u32 __fastcall Person_PlaceScript(u8 *person, void *, u32 x, float y, u32 z, u32 r15, u32 anim);

// 0x50be80 (thiscall on a person's thoughts (the person's +0x30): thought, force (a byte), pops 8; name invented, unconfirmed): the thought
// shown. +0 = thought; nothing more with any of 0x790ab0's bits 0x16, or with the mode setting 0xfb3b7c (a function-local static, guard bit 1
// of 0xfb34aa: Setting_ReadMode and atexit 0x5d1750 the first time) 1 and no force. The bubble script at +0x84 is killed and cleared
// (SpriteScript_Kill); thoughts 1 to 0x16 then have a bubble (the entry points 0x74f190 - 0x74f388) and a wait of 0 to 3 times 20 ticks:
// when the world's clock ([[0x7cf6ec]] + 0x1da70c) is at or past +0x8c + the wait (unsigned), +0x8c = the clock and +0x84 =
// SpriteScript_Start (the entry point, folder 9, 0...). Another thought: the empty Debug_Printf only.
void __fastcall Thoughts_Show(u8 *thoughts, void *, u32 thought, u8 force);
}

// What Thoughts_Show calls in agent B's code, so that it can run outside the game (the benches point it at recorders).
struct ThoughtsEnv {
    u8 *(__fastcall *readMode)(u8 *setting);                                                // Setting_ReadMode 0x550ca0
    s32(__cdecl *atExit)(void(__cdecl *f)());                                               // atexit 0x67a7d0 (the CRT's)
    void(__cdecl *kill)(s32 slot);                                                          // SpriteScript_Kill 0x475550
    s32(__cdecl *start)(u32 code, u32 folder, u32 sprite, u32 anim, u32 x, u32 y, u32 z);  // SpriteScript_Start 0x475a10
};
// nullptr restores the game's.
void Thoughts_SetEnv(const ThoughtsEnv *env);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gPersonScripts_fb34aa, 0x00fb34aa, 0x1);   // a function-local static's guard
