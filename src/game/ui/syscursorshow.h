// The system cursor's calls to Windows (ui/cursor has the class). Reimplementation of 0x66197a, 0x6619a6, 0x6619c0 and 0x6619da. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66197a (thiscall (handle, byte), pops 8): +0x14 the handle, SetCursor on it, +7 the byte.
void __fastcall SysCursor_SetHandle(u8 *self, void *, u32 handle, u32 byte);

// 0x6619a6 (thiscall): ShowCursor(1), +4 1.
void __fastcall SysCursor_Show(u8 *self);

// 0x6619c0 (thiscall): ShowCursor(0), +4 0.
void __fastcall SysCursor_Hide(u8 *self);

// 0x6619da (thiscall (word), pops 4): SetCursor on +0x14 with the word set, else on 0.
void __fastcall SysCursor_Apply(u8 *self, void *, u32 on);
}
