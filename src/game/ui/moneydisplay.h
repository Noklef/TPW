// The money display's update (0x4a0ab0): the park's money on a window, its colour by size, the minus child, the cost child.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the update calls (the benches point these at recorders): the ports.
struct MoneyDisplayEnv {
    u32(__fastcall *money)(u8 *world);                                     // GameTable7cfb90_Get1da726_5195d0
    u32(__fastcall *amount)(u8 *money);                                    // Source_Field0c 0x6ad810
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000 0x65c5d5
    void(__fastcall *areaRect)(u8 *window, void *, u8 *rect);              // Window_GetAreaRect 0x65d616
    u32(__fastcall *moveTo)(u8 *window, void *, const s16 *x, const s16 *y);   // Window_MoveTo 0x65ce10
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);     // Window_GetRect 0x65d42e
    void(__fastcall *notify1)(u8 *window, void *, u32 value);              // Window_Notify1 0x65d3a3
    u8 *(__fastcall *modeBase)(u8 *self);                                  // MouseModeOnlineBase_Ctor 0x46c6a0
    void(__fastcall *setMode)(u8 *holder, void *, u8 *mode);               // MouseMode_Set 0x46c350
    u32(__cdecl *editFlag)();                                              // CoasterEdit_Flag1 0x447990
    u32(__cdecl *cost)();                                                  // Global81af4c_Get_52fbc0
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);              // Window_SetShown 0x65d9dd
};
// The update's calls go to env (nullptr: the ports).
void MoneyDisplay_SetEnv(const MoneyDisplayEnv *env);

// 0x4a0ab0 (cdecl (window); name invented, unconfirmed). The money ([[0x7cf6ec]]'s table's Source_Field0c) when not [0x7cb2dc]: its colour index ((money + 999) / 1000
// held to 0-5, 0x7cb27c) when not [0x752e2c] gives the window's colour (Window_Notify40000 of the bytes at 0x752e30 + 4 * index, 0xff), the window's slot 7 the money's
// size; the child 0x31 moved to the area's right (Window_GetAreaRect), the child 0x30 moved to its rect's (+4 - +8 dwords) plus that, the child 0x32 told whether the money is under 0.
// The mouse modes' holder made if there is none (as MouseModes_EnsureHolder inlined). A mode [0x7b05d8] of kind (slot 9) 3 or 4, or 8 with CoasterEdit_Flag1: the cost
// (0x52fbc0) when not [0x752ec0] goes on the child 0x30 if there is one: shown unless -999, hidden and not kept when -9999, else its slot 7 and kept. Another mode: the child 0x30
// hidden and [0x752ec0] -999 unless so already.
void __cdecl MoneyDisplay_Update(u8 *window);
}
