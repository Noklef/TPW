// The game options (0x78d8d8, the options screen's): reimplementation of 0x423ad0-0x423ba0
// (thiscall). +0..+0x3c are the options in force (+0 procedural terrain, +4 the resolution when
// the balance file gives none), +0x3c..+0x78 a copy to go back to; +0x94 a level of change (2 when
// the display must be reset) and +0x98 flags whose low three bits mark changes.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x423ad0: the copy back over the options; +0x94 = 0, the change bits cleared.
void __fastcall Options_Revert(u8 *options);
// 0x423b00: the options over the copy; +0x94 at least 0, the change bits cleared.
void __fastcall Options_Keep(u8 *options);

// 0x423b40: +0x30 = value.
void __fastcall Options_Set30(u8 *options, void *, u32 value);
// 0x423b60 / 0x423ba0 / 0x423b80: +4 / +8 / +0xc = value (written only when it differs).
void __fastcall Options_Set4(u8 *options, void *, u32 value);
void __fastcall Options_Set8(u8 *options, void *, u32 value);
void __fastcall Options_SetC(u8 *options, void *, u32 value);
}
