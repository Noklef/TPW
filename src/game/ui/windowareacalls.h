// The window's area object (+0x98) asked or told. Reimplementation of 0x65d58d and 0x65d717. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the ports by default; the bench's recorder: its own bench has it).
struct WindowAreaCallsEnv {
    u8 *(__fastcall *getRect)(u8 *area, void *, u8 *out);   // WindowArea_GetRect
};
void WindowAreaCalls_SetEnv(const WindowAreaCallsEnv *env);

extern "C" {

// 0x65d58d (thiscall (out), pops 4): with an area +0x98 its rectangle into the out (WindowArea_GetRect); without, the out assigned an empty rectangle (a local GuiRect 0, 0, 0, 0; GuiRect_Assign).
void __fastcall Window_AreaRectOrEmpty(u8 *self, void *, u8 *out);

// 0x65d717 (thiscall): with an area +0x98 its slot 6 (0x18; thiscall, no arguments).
void __fastcall Window_CallAreaSlot6(u8 *self);
}
