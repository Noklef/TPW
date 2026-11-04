// Two thiscall virtual methods of classes with a list of windows (no direct call; vtables hold them: 0x47de00 in eight from 0x705990 to 0x705c68, 0x47e250 in the one at 0x705a00): reimplementation of 0x47de00 and 0x47e250. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Their call by address (the bench's recorder).
void WindowGroup_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x47de00: the object +0x9c, if any, told its slot +8 (0, this + 0x2c, the word +0xe0 + 1).
void __fastcall WindowGroup_TellObject(u8 *self);

// 0x47e250: with the array +0x148, Window_OpenTree on each of its (signed word +0x172) windows, the count read afresh each time.
void __fastcall WindowGroup_OpenAll(u8 *self);
}
