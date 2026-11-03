// The money display's handler (the window the update 0x4a0ab0 paints). The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the handler calls (the benches point these at recorders): the ports.
struct MoneyDisplayHandlerEnv {
    void(__cdecl *update)(u8 *window);                                             // MoneyDisplay_Update 0x4a0ab0
    u32(__cdecl *setCost)(u32 value);                                              // Global81af4c_Set_52fb90
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault 0x65f66e
};
// The handler's calls go to env (nullptr: the defaults).
void MoneyDisplayHandler_SetEnv(const MoneyDisplayHandlerEnv *env);

// 0x4a0db0 (cdecl (window, message, a, b); name invented): 0x15: [0x7cb2dc] and the tick count [0x7cb2e0] 0. 0x1e: the count up by one; when bit 1 of the old count is set, the display updated
// and the count 0; the cost Global81af4c 0. Every message then goes to Window_CallDefault, its answer returned.
s32 __cdecl MoneyDisplay_Handler(u8 *window, u32 message, u32 a, u32 b);
}
