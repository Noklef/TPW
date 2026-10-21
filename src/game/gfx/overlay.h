// The base of the model-backed screen overlays (vtable 0x6fee10, built at 0x476a90; the bar
// mesh 0x6fee88 and others derive from it; itself a GUI dialog, ui/guidialog): reimplementation of
// 0x476bf0, 0x476e80, 0x476f10 and the destructors 0x476b50 / 0x476b70, 0x477270 / 0x477290 and 0x4775a0 / 0x4775c0.
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

// 0x476f10 (thiscall (flags, desc), pops 8; name unconfirmed): nothing without a holder, a descriptor or flags. Flag 0x40: the
// pivot +0x34 / +0x36 from desc +0xc / +0xe. Then Overlay_ShowFrame; nothing more when it shows no node. Flag 0x10: the rectangle
// +0x18..+0x1e from the four words at desc [+0x14] + 4, copied to +0x24..+0x2a. Flag 4: the angle +0x30 from desc +8. Any of 0x55:
// the node's bounds (+0x78 low, +0x84 high) through its matrix (+0x10) times its parents' (+4 chain), x and y put in order (below
// or unordered swapped, the new low through the x87), the screen matrix (+0x3c) for the rectangle (x, the bottom +0x1e, width,
// height +0x1e - +0x1a) showing low .. high with the pivot and angle (Sprite_ScreenMatrix), and with a rate (+0x80) +0x64 = rate /
// 40 / |high z - low z|. Flag 2: the height +0x38 from desc +6 and the scale (+0x74) 1 - height / 40. With either the scale is
// stored, the matrix set on the holder's object (+0x1c; ObjModel_SetMatrices, 1) and bit 31 of 0x8bd508 set.
void __fastcall Overlay_Apply(u8 *overlay, void *, u32 flags, const u8 *desc);

// 0x477310 (thiscall (flags, desc), pops 8; slot 3 of 0x6fee38; name unconfirmed): nothing without a descriptor. Overlay_ShowFrame;
// for a shown node (the test ((flags & 0x10) | 0x80001) is never 0), flag 0x10 the rectangle as Overlay_Apply has it; then the node's
// quad stretched to the rectangle: a = width (+0x1c - +0x18) - (high x - low x), b = -(height (+0x1e - +0x1a) - (high y - low y))
// (bounds +0x78 low, +0x84 high; on the x87), and of its 16 vertices (+0x60: rows of 0x30 bytes, x[4], y[4], z[4]) each x above 0 gets a,
// each y below 0 (or unordered) gets b; low y += b, high x += a (the other four bounds written back as read). Then Overlay_Apply.
void __fastcall Overlay_ApplyResized(u8 *overlay, void *, u32 flags, const u8 *desc);

// 0x477640 (thiscall (flags, desc), pops 8; slot 3 of 0x6fee60; name unconfirmed): with a descriptor, flag 0x10 and a first node (the
// holder's (+4) model's (+4) list head, +0x78: the holder read even without them, sic), the rectangle as Overlay_Apply has it and the
// first 8 vertices of the node's quad stretched in y only, as Overlay_ApplyResized does (low y += b). Then Overlay_Apply.
void __fastcall Overlay_ApplyResizedHeight(u8 *overlay, void *, u32 flags, const u8 *desc);

// 0x476bf0 (thiscall (shown, a word), pops 4; name unconfirmed): nothing without a holder or when +8 is shown already. Shown: the
// shown node (+0x7c) has 0x8010 cleared and the holder wakes with the screen matrix (+0x3c; Holder_WakeWithMatrix, grid 0); else
// the node gets 0x8010 and the holder sleeps. +8 = shown.
void __fastcall Overlay_SetShown(u8 *overlay, void *, u32 shown);
}
