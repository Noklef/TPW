// The game's set-up of a GUI window as a text button (its 0x705a50 colour dwords set too): a text part as the window's impl, the "!cbut"
// look, a font, a colour and a text. Reimplementation of 0x486660. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x486660 (cdecl (window, text, font, red, green, blue, value); 0x4b4a50 and the window builders of 0x49bdd0 and others): a new text part
// (0x1a4 bytes, GameHeap_Alloc; TextPart_Ctor(0), none when the allocation fails) as the window's impl (Window_SetImpl), the object of
// the GUI window registered as "!cbut" (GuiWindows_FindByName) as its object +0x98 (Window_SetObject98), the bytes 1, 1 sent as 0xc00
// and as 0x3000 (Window_NotifyC00, Window_Notify3000), font `font` (Language_Font, none from 13 up; Window_Set4c), the bytes red,
// green, blue, 0xff sent as 0x40000 (Window_Notify40000) and as one dword (red low) stored at +0x14c and +0x154 (Window705a50_Set14c /
// Set154), the text sent as 0x100 to the impl (Window_NotifyImpl100) and +0x90 = value. Returns value (as the original leaves EAX).
u32 __cdecl ButtonText_Init(u8 *window, u32 text, u32 font, u32 red, u32 green, u32 blue, u32 value);

}
