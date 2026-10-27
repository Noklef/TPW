// An object's status (what its panel says of it, an index into the text ids of 0x74fb50, Table74fb50_Get): reimplementation of 0x485f60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x485f60 (cdecl; object, queue count; name unconfirmed): the object's status. 3 when mState (+0x19c) is 4. For a definition of track type
// (+0x9c) 1 or 2 without the dword +0x2c, or of type 3 whose coaster link is not open (Object_LinkOpen), 0x19; type 3 with a link that has
// contacts (Object_LinkBare 0) 0x1a. Then in state 2 (closed, waiting) the world's word +0x1da73e: 5 when it is 0, 7 when the game table entry
// of +0x1da718 has its indexed dword 1 (Obj_GetIndexed28) set, 6 without a tracked person (Obj_GetTrackedHandle), else 8, or 9 when that person
// is in its state (Obj_TrackedPersonInState). With the dword +0x64 set the same for the word +0x1da73e (0x11, 0x13, 0x12, 0x14 / 0x15) unless
// bit 1 of the byte +0x32, then for +0x1da73c and indexed dword 0 (0xb, 0xd, 0xc, 0xe / 0xf). In state 1, 0x15 with the tracked person in its
// state, else 2. With bit 8 of +0x32 and the entrance off the path (Object_EntranceOnPath 0), 0x17 when +0x68 is 0, else 0x16. A definition of
// kind (+0x4ac) 0 with no word at +0x284 (Object_DefWord284), +0x288 not 2 and the exit not open (Object_ExitOpen) gives 0x18. Else 1 for a
// count of 0 or one above 1 (signed) with +0x68 0, otherwise 0.
u32 __cdecl Object_Status(u8 *object, s32 count);

// 0x4861e0 (cdecl (window, status); name unconfirmed; called from 0x48d18d): the status shown on a window: its colour
// (the table 0x74fb50, the status's dword: bytes 0, 1, 2 and 0xff) told (Window_Notify40000), then its text (the record of [0x78ba94] whose
// index is the table 0x74fbc0's dword, widened: Language_WideText; 0 when there is none) set by the window's slot 9, whose EAX is returned.
// No bound on the status (past 27 the text ids' table is read as colours, sic).
u32 __cdecl ObjectStatus_Show(u8 *window, u32 status);

// 0x48d160 (thiscall; slot 9 of the panels' vtables (0x6ff740 and nine more); name unconfirmed): with a shown thing ([0x7c2658]) and the
// panel's window +0x1c, that window shown or hidden as the panel's status +8 is set or 0 (Window_SetShown) and the status shown on it
// (ObjectStatus_Show), whose EAX is returned (else [0x7c2658] or 0, as EAX was left).
u32 __fastcall ObjectStatus_ShowOnPanel(u8 *panel);
}
