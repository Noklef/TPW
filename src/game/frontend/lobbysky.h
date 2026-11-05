// The lobby's sky and camera set up (names invented, unconfirmed; "Data\Levels\fantasy" is the only clue to the subject): the object's place at +8, its target at +0x14, the renderer's flags and fog kept at +0x34.
#pragma once

#include "../../../re/types.h"

// Its calls (the bench's recorders). The sky's turn is B's, unported.
struct LobbySkyEnv {
    void(__fastcall *loadSky)(void *landscape, void *, const char *folder);                                 // Landscape_LoadSky
    void(__fastcall *turnSky)(void *landscape, void *, u32 x, u32 y, u32 z);                                // 0x585690, three floats by value
    void(__fastcall *light)(u8 *slot, void *, u32 on, const u32 *colour, const u32 *direction, u32 value);  // LightSlot_Set
    void(__fastcall *lookAt)(u8 *camera);                                                                   // Camera_SetLookAt
    u32(__fastcall *resetDome)(u8 *landscape);                                                              // Landscape_ResetDomeCentre
};
void LobbySky_SetEnv(const LobbySkyEnv *env);  // nullptr: the game's

extern "C" {

// 0x5d8b50 (thiscall; name invented, unconfirmed): +0x34..+0x40 = the renderer's flags 0x8bcbc8, fog colour 0x8bd47c, fog start and end 0x8bd490, 0x8bd494. The sky of "Data\Levels\fantasy" loaded into the landscape 0x8c1828 and turned (0, 180, 0).
// The flags lose 0x10000 and 0x2000 and gain 0x4000000, the fog = 0xff44ddff, 50, 300. +4 = 115, the place (500, 100, 500), the target (500, 0, 500); the vector 0x879dd8 = (1, 0, 0, 0); LightSlot_Set off; +0x28..+0x30 = 0; Camera_SetLookAt; +0x44..+0x60 = 0.
void __fastcall LobbySky_Enter(u8 *self);
// 0x5d8d70 (thiscall; name invented, unconfirmed): the vector 0x879dd8 = (1, 0, 0, 0); LightSlot_Set off; the renderer's flags and fog back from +0x34..+0x40; Landscape_ResetDomeCentre on 0x8c1828 and its answer.
u32 __fastcall LobbySky_Leave(u8 *self);
}
