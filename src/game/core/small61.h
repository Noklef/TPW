// A thing's placement, a free slot read and a sprite choice: reimplementation of 0x4fa930, 0x51fa20 and 0x4fcac0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4fa930 (thiscall on the thing; x, y, pops 8): the navigation block at +0xd4 gets the position (x, y as 16-bit values shifted left 8) and no speed (Nav_SetPosVel), the
// position is read back to the words at +0x190 and +0x194 (Nav_GetPos), then the thing moves (Thing_SetPos with the same x and y).
void __fastcall Thing_PlaceAt(u8 *thing, void *, u32 x, u32 y);

// 0x51fa20 (cdecl; stream): -1 when [0x816d40] is 0; otherwise the first of the 0x140-byte slots from 0x804488 whose first byte is 0 gets 0x140 bytes read into the slot's 0x140-byte
// record at 0x804370 (Object_CallSlot8Product with the stream) and its index is returned; -1 when none is free below 0x80c7c8.
s32 __cdecl Slot_ReadIntoFree(u8 *stream);

// 0x4fcac0 (thiscall on the object): with sprites in folder 3, the dword at +0x20 (with the top bit set when the dword at +0x24 is 2). Without, the world's random seed is set to
// the thing's handle (World_SetSeed on [0x80239c]) and a random sprite of folder 1 returned.
u32 __fastcall Thing_SpriteChoice(u8 *thing);

}  // extern "C"
