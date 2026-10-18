// The base of the model-backed screen overlays (vtable 0x6fee10, built at 0x476a90; the bar
// mesh 0x6fee88 and others derive from it; itself a GUI dialog, ui/guidialog): reimplementation of
// 0x476e80 and the destructors 0x476b50 / 0x476b70, 0x477270 / 0x477290 and 0x4775a0 / 0x4775c0.
// +4 its holder (whose +4 model's +0x78 starts the node list, linked through +0xc), +0xa / +0xc
// (u16) the shown node's index as base + frame, +0x10 (u16) set when the holder is only
// destroyed, not disposed of, +0x7c the shown node.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x476e80: with flag 0x80000 takes the base from desc +0xa, with flag 1 the frame from desc
// +4 (else, with neither, nothing); then shows node base + frame (u16; fewer when the list ends
// first, null past it): the old one gets 0x8010 set in its flags (+0), the new one cleared.
// Returns the shown node (+0x7c).
const u8 *__fastcall Overlay_ShowFrame(u8 *overlay, void *, u32 flags, const u8 *desc);

// 0x476b70 (name unconfirmed): the vtable back to 0x6fee10; a holder (+4) is disposed of (Holder_Dispose; the original also
// passes [0x7c249c] and 0, unread) when the word +0x10 is 0, else destroyed (Holder_Destroy); then GuiDialog_Dtor.
void __fastcall Overlay_Dtor(u8 *overlay);

// 0x476b50 (thiscall (flags), pops 4; slot 0 of 0x6fee10; name unconfirmed): Overlay_Dtor, the overlay freed (Mem_Free) for
// flags bit 0; returns it.
u8 *__fastcall Overlay_ScalarDtor(u8 *overlay, void *, u32 flags);

// 0x477290 / 0x4775c0 (names unconfirmed): the destructors of the overlays with vtables 0x6fee38 and 0x6fee60, whose own parts
// leave nothing to undo: each is Overlay_Dtor.
void __fastcall Overlay_Dtor_477290(u8 *overlay);

void __fastcall Overlay_Dtor_4775c0(u8 *overlay);

// 0x477270 / 0x4775a0 (thiscall (flags), pops 4; slot 0 of 0x6fee38 / 0x6fee60; names unconfirmed): their scalar deleting
// destructors, as Overlay_ScalarDtor.
u8 *__fastcall Overlay_ScalarDtor_477270(u8 *overlay, void *, u32 flags);

u8 *__fastcall Overlay_ScalarDtor_4775a0(u8 *overlay, void *, u32 flags);
}
