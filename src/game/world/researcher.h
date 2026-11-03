// A researcher's tick. Reimplementation of 0x502960.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct ResearcherEnv {
    u32(__fastcall *mode)(const u8 *object);                // Mode19c_ToIndex
    u32(__fastcall *lab)(u8 *game);                         // GameTable7cfb90_Get1da722_519550
    void(__fastcall *spend)(u8 *lab, void *, s32 amount);   // ResearchLab_Spend
    void(__fastcall *tick)(u8 *person);                     // Person_TickSpeed
};

void Researcher_SetEnv(const ResearcherEnv *env);

// 0x502960 (thiscall; Thing_Update case 8): out of the modes 3-5 and every twentieth tick ([0x80239c]+0x1da70c), the lab spends the
// wage of the researcher's level (the dword of the 12-byte row +0x1e4 at 0x78545c; the handle's debug print left out). Always Person_TickSpeed.
void __fastcall Researcher_Update(u8 *person);
