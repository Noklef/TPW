// The GUI library's window subclass with the vtable 0x705a50 (built by 0x6688b3 over Window_Ctor: the handlers 0x668f9c, a text row
// (ui/textpool) at +0x148, the dwords +0x13c 500 and +0x140 125, colour-like bytes +0x14c-+0x14f 0xff; what it is is unconfirmed).
// Reimplementation of 0x6689bc, 0x6693a0 and 0x66852b. +0x138 its wide text, +0x13c the text's limit.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6689bc (thiscall; name unconfirmed): the destructor: the vtable 0x705a50, the text row +0x148 (when set) released
// (TextPool_Release), then Window_Dtor, whose EAX is left.
u32 __fastcall Window705a50_Dtor(u8 *window);

// 0x66852b (thiscall; name unconfirmed): the length of the wide text +0x138 (the C library's wcslen; its low word in AX), 0 for none.
u16 __fastcall Window705a50_TextLength(const u8 *window);

// 0x6693a0 (thiscall (flags), pops 4; slot 2; name unconfirmed): the scalar deleting destructor: Window705a50_Dtor, then with flag 1
// freed (Mem_Free). Returns the window.
u8 *__fastcall Window705a50_DeletingDtor(u8 *window, void *, u32 flags);
}
