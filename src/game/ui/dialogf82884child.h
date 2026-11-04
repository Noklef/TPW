// A child of the window at [0xf82884]+4 shown by the user slots. Reimplementation of 0x4bbbd0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders: their own benches have them).
struct DialogF82884ChildEnv {
    u8 *(__cdecl *slots)();                                     // UserSlots_Instance
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);    // Window_FindChild
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);   // Window_SetShown
};
void DialogF82884Child_SetEnv(const DialogF82884ChildEnv *env);

extern "C" {

// 0x4bbbd0 (cdecl): the child 0xbf432 of [[0xf82884] + 4] shown when the user slots' +0xc4 is set, hidden when not. Returns what Window_SetShown returns.
u32 __cdecl DialogF82884_ShowChild();
}
