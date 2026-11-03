// A dialog group's last tab opened again. Reimplementation of 0x4a0940; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct DialogTabsEnv {
    u32(__cdecl *hasSource)();                       // Global7c2534_HasSource
    void(__cdecl *open7cc1f8)(u32 kind, u32 selected);  // Dialog7cc1f8_Open
    void(__cdecl *open7ca308)(u32 kind);             // Dialog7ca308_Open
    void(__cdecl *open7cb398)();                     // Dialog7cb398_Create
    void(__cdecl *open7ca168)(u32 tab, u32 selected);   // Dialog7ca168_Open
    void(__cdecl *open7ca150)(s32 tab, u32 selected);   // Dialog7ca150_Open
    void(__cdecl *open7ca13c)(u32 selected);         // Dialog7ca13c_Open
    void(__cdecl *open7ca2e8)();                     // Dialog7ca2e8_Open
    void(__cdecl *open7cb25c)();                     // Dialog7cb25c_Open
    void(__cdecl *open7cc2ec)();                     // Dialog7cc2ec_Open
    void(__cdecl *open7ca210)();                     // Dialog7ca210_Open
};
void DialogTabs_SetEnv(const DialogTabsEnv *env);

extern "C" {

// 0x4a0940 (cdecl (group)): nothing with Global7c2534_HasSource. Group 1 by [0x7cb28c]: 1 Dialog7cc1f8_Open (-1, 0; only the word of the second is set), 2 Dialog7ca308_Open (-1). Group 2 by [0x7cb2a0]: 3 Dialog7cb398_Create, 4 Dialog7ca168_Open (-1, 0), 5 Dialog7ca150_Open (-1, 0), 6 Dialog7ca13c_Open (0).
// Group 3 by [0x7cb290]: 7 Dialog7ca2e8_Open, 8 Dialog7cb25c_Open, 9 Dialog7cc2ec_Open, 10 Dialog7ca210_Open. Any other nothing.
void __cdecl DialogTabs_Reopen(s32 group);
}
