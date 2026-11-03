// A window's area object (+0x9c): its rect (+0x20) and its other rect (+0x2c) as GuiRects.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x661810 (thiscall (out), pops 4; name invented, unconfirmed): out = the area's rect +0x20 (GuiRect_Assign; its answer).
u8 *__fastcall WindowArea_GetRect(u8 *area, void *, u8 *out);

// 0x66b438 (thiscall (out), pops 4; name invented, unconfirmed): out = the object's rect +0x2c (GuiRect_Assign; its answer).
u8 *__fastcall WindowArea_GetRect2c(u8 *area, void *, u8 *out);

// 0x65d616 (thiscall (out), pops 4; name invented, unconfirmed): out = the rect of the window's area +0x9c (WindowArea_GetRect), an all-zero rect without one.
void __fastcall Window_GetAreaRect(u8 *window, void *, u8 *out);
}
