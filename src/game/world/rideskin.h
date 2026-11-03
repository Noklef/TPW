// A ride's skins put back before it goes: reimplementation of 0x4690a0.
#pragma once

#include "../../../re/types.h"

// The call goes to env (nullptr: the port).
struct RideSkinEnv {
    void(__cdecl *destroy)(u32 ride);  // Ride_Destroy (0x463c80)
};
void RideSkin_SetEnv(const RideSkinEnv *env);

extern "C" {

// 0x4690a0 (cdecl (record); called by 0x4864a4 and 0x48651a; name invented, unconfirmed): nothing for no record. With a skin kind at +0x84 whose entry of the table 0x7ae6a8 has +0xd0 set: in the model of the ride's instance
// (+4 in the table 0x7a4610, its +8's +4; words +0x44 parts of 0xa0 bytes at +0x70, each with a word count at +0x5a and at +0x6c (unaligned) an array of 0x10-byte entries) the first dword of each entry equal to +0x7c
// set to +0x74, then those equal to +0x80 set to +0x78, and +0x84 cleared. Then Ride_Destroy (+4).
void __cdecl Ride_RestoreSkinsAndDestroy(u8 *record);
}
