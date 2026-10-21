// The game panel object at 0x7cc258 (its vtable 0x6ffc68). Reimplementation of 0x4afaa0, 0x4afc10, 0x4b0640. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4afc10 (thiscall; slot 3 of the vtable 0x6ffc68): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0xc075 text 0x20, 0xc071 text 0x21, 0xc072 text 0x22, 0xc073 text 0x23, 0xc074 text 0x24, 0xc07a text 0x25, 0xc07b text 0x26, 0xc070 no
// text, 0xc076 a 0x6ff3b0 part with 0, 0xc077 a 0x6ff3b0 part with 0, 0xc078 a 0x6ff3b0 part with 0, 0xc079 a ridestatbar bar with 0x32,
// 0xc07d a 0x6ff3b0 part with 0, 0xc07c a happygrad bar with 0x64.
void __fastcall Panel7cc258_Dress(u8 *self);

// 0x4afaa0 (thiscall; slot 4 of the vtable 0x6ffc68): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7cc258_DressTip(u8 *self);

// 0x4b0640 (thiscall; slot 0 of the vtable 0x6ffc68): with the selected thing [0x7cc288] (read again where used) and the window +0x10's
// children (Window_FindChild): when the things of the panel's group (+4) number under 2 (Things_CountGroup with the thing), 0xc069 and
// 0xc06a hidden (not checked); a GuiRect made and destroyed about the rest (GuiRect_Ctor / GuiRect_Dtor); then each when found: the
// sliders 0xc06b and 0xc086 given the ranges 0..2 and 0..100 (Window705b20_SetRange); 0xc085 a 0x6ff3b0 text part (0x1a4 bytes,
// GameHeap_Alloc, its constructor with 0; none when that fails), the range 0..500 and step 1 (Window705af0_SetRange / SetStep), font 6
// (Language_Font; Window_Set4c), the bytes 1, 1 as 0xc00 and 0, 0, 0, 0xff as 0x40000; 0xc07e, 0xc084 and 0xc083 each a TextPart_Ctor
// part the same way (0xc00 before the font) and shown the language text 0x27, the thing's ingredient's text (Thing_IngredientRecord,
// Language_Widen) and the text 0x28 (Window_NotifyImpl100); then the hover label 0xc06e bound (HoverLabel_Bind) and +0x28 = 0.
void __fastcall Panel7cc258_Open(u8 *self);
}
