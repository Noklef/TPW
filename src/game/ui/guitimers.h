// The GUI library's timed messages: a table of +0 the count and +4 the entries (0x14 bytes each: window, message, a, b, the time due
// by the GUI clock (GuiClock_Now), 0 for a free entry). Reimplementation of 0x669c62, 0x669c8d, 0x669d77, 0x669e3a, 0x669eb3 and
// 0x669efb.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x669efb (thiscall (count), pops 4; name unconfirmed): 0 in AX when the entries are set or the count is 0x200 or more (signed), or
// their allocation (Mem_Alloc, count * 0x14 bytes) fails (+4 then 0). Otherwise the count stored and each entry free; 1 in AX. Only AX is
// set.
u32 __fastcall GuiTimers_Init(u8 *table, void *, s32 count);

// 0x669c62 (thiscall; name unconfirmed): the entries (when set) freed (Mem_Free); +4 is left as it is.
void __fastcall GuiTimers_Free(u8 *table);

// 0x669c8d (thiscall (window, message, a, b, delay), pops 0x14; name unconfirmed): with entries and a delay of 1-59999 (unsigned),
// the first free entry gets the four and the time due GuiClock_Now() + delay; 1 in AX. Otherwise (none free too) 0 in AX. Only AX is set.
u32 __fastcall GuiTimers_Add(u8 *table, void *, u32 window, u32 message, u32 a, u32 b, u32 delay);

// 0x669d77 (thiscall; name unconfirmed): with entries, each one due (GuiClock_Now() asked for each set one) at or before now (unsigned)
// is sent (Window_CallHandler) and freed; their number in AX (0 without entries). Only AX is set.
u32 __fastcall GuiTimers_Fire(u8 *table);

// 0x669e3a (thiscall (window), pops 4; name unconfirmed): each entry (free ones too) whose window is the one given freed; their number
// in AX, -1 without entries. Only AX is set.
u32 __fastcall GuiTimers_Cancel(u8 *table, void *, u32 window);

// 0x669eb3 (thiscall; name unconfirmed): with entries, each one freed.
void __fastcall GuiTimers_Reset(u8 *table);
}
