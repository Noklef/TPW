// A staff member held by a building and the world's staff slots. Reimplementation of 0x509af0 and 0x5083b0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct StaffHeldEnv {
    void(__fastcall *place)(u8 *staff, void *, u32 cell);        // Staff_PlaceInCell
    void(__fastcall *changed)(u32 dialog, void *, u32 index);    // 0x481550, its ECX the dialog 0x7b51f0
};

void StaffHeld_SetEnv(const StaffHeldEnv *env);

// 0x509af0 (thiscall (cell), pops 4; called from 0x46cce5, 0x46ce6c): the thing in the slot of the handle +0x60 (a null one too) placed in
// the cell (Staff_PlaceInCell); the handle cleared.
void __fastcall Holder_PlaceStaff(u8 *holder, void *, u32 cell);

// 0x5083b0 (thiscall (keep), pops 4; called from 0x46c8bd on the world [0x7cf83c]): each of the 0x20 slots (0x14 bytes) whose byte +0xb is set
// has it cleared; without keep also its byte +0xa, and the dialog told of the index.
void __fastcall World_ClearStaffSlots(u8 *world, void *, u32 keep);
