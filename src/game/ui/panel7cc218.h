// The game panel object at 0x7cc218 (its vtable 0x6ffbe8). Reimplementation of 0x4ad4e0, 0x4ad720, 0x4ad890, 0x4ae8e0, 0x4ae980, 0x4af980, 0x4af440. The
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4ad890 (thiscall; slot 3 of the vtable 0x6ffbe8): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x3e20 text 0x11, 0x3e1a text 0x12, 0x3e1c text 0x13, 0x3e1e text 0x14, 0x3e1f text 0x15, 0x3e1d text 0x16, 0x3e22 text 0x17, 0x3e21 a
// 0x6ff350 part with 0, 0x3e1b no text, 0x3e16 a ridestatbar bar with 0, 0x3e18 a ridestatbar bar with 0, 0x3e19 a ridestatbar bar with 0,
// 0x3e17 a ridestatbar bar with 0, 0x3e23 a 0x6ff3b0 part with 0.
void __fastcall Panel7cc218_Dress(u8 *self);

// 0x4ad720 (thiscall; slot 4 of the vtable 0x6ffbe8): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7cc218_DressTip(u8 *self);

// 0x4ae980 (thiscall; called by 0x4ae8e0): a GuiRect made and destroyed about the rest (GuiRect_Ctor / GuiRect_Dtor); with the window +0x10
// set (read again for each child), its children (Window_FindChild, none checked): the gauges 0x3e2d and 0x3e30 each the range 0..0x78
// (Window705b20_SetRange) and their own child 0x3e2e a "smallguage.wct" mesh (0x124 bytes, MeshOverlay_Construct; none when the allocation
// fails) as its implementation (Window_SetImpl), then its slot 7 given 0; 0x3e2f the range 0..10; then 0x3e31, 0x3e33 and 0x3e32 each a
// TextPart_Ctor part (0x1a4 bytes; none when that fails), the bytes 1, 1 as 0xc00, font 6 (Language_Font; Window_Set4c), the bytes 0, 0,
// 0, 0xff as 0x40000 and no text (Window_NotifyImpl100(0)).
void __fastcall Panel7cc218_SetUpGauges(u8 *self);

// 0x4ae8e0 (thiscall; slot 0 of the vtable 0x6ffbe8): when the things of the panel's group (+4) number under 2 (Things_CountGroup with
// the selected thing [0x7cc208]), the window +0x10's children 0x3e26 and 0x3e27 hidden (not checked); then the gauges set up
// (Panel7cc218_SetUpGauges), the window +0x10 shown (Window_SetShown) and the hover label 0x3e28 bound (HoverLabel_Bind).
void __fastcall Panel7cc218_Open(u8 *self);

// 0x4af980 (thiscall (thing), pops 4; slot 10 of the vtable 0x6ffbe8; called by 0x4869cb / 0x4869f1 on 0x7cc218): the selected thing
// [0x7cc208] = thing, the window built (WindowAt10_Create: the script 0x755150, the handler 0x4af600, the children 0x3e24, 0x3e15 and
// 0x3e25), +0x24 = 0, and the advisor posted 0x129 when the low byte of the thing's float +0x44 (__ftol) is 1-20, else 0xcf
// (Advisor_PostId).
void __fastcall Panel7cc218_Show(u8 *self, void *, u8 *thing);

// 0x4af440 (thiscall; slot 15 of the vtable 0x6ffbe8; name invented): the shown thing [0x7c2658] given the panel's settings: the speed
// +0x2c (Obj_SetSpeed), the capacity byte +0x30 (Object_SetCapacity) and, when its definition's +0x70 is set, the duration byte +0x34
// (Obj_SetDuration).
void __fastcall Panel7cc218_ApplyToShown(u8 *self);

// 0x4ad4e0 (thiscall; called from 0x4ae0fb and by slot 1, 0x4ae940): with the thing [0x7c2658] (nothing without one) and its definition
// (Object_GetDef), the window +0x10's children (Window_FindChild, none checked): with the mode +8 3 all of 0x3e38, 0x3e34, 0x3e2a, 0x3e36,
// 0x3e2c, 0x3e37, 0x3e35 hidden; else shown (Window_SetShown) in that order, with: 0x3e38 (a 0x705a50 button) its slot 5 given 1 and the
// state 1 when the thing's +0x68 is 0 (Is68_Zero_4e0440), else 0, notifying (Window705a50_SetState), when the mode is not 0x19 / 0x1a and
// the thing either has +0x68 0 and can open (Object_CanOpen) or not 0 and Const1_424ec0 answers, else its slot 5 given 0; 0x3e2a shown
// only when the definition's track type +0x9c is not 0; 0x3e36 (a 0x705a50 button) the state 1 for the modes 0x11-0x15, else 0,
// notifying; 0x3e2c's slot 5 whether the track's neighbours are right (Object_TrackNeighboursOk, its low byte).
void __fastcall Panel7cc218_UpdateButtons(u8 *self);
}
