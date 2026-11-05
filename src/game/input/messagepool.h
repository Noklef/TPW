// The input messages' pools: a circular list (its sentinel at +8, the count at +0xc) of messages with a cursor at +0x10; a free message has +8 clear. Reimplementation of 0x634a50, 0x634b40, 0x634d00, 0x634de0.
// Names invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x634a50 / 0x634b40 (thiscall): the destructors: vtable 0x703d1c / 0x703d14; the messages freed (Mem_Free, without their destructors), then the nodes and the sentinel; +8 = +0xc = 0.
void __fastcall KeyPool_Dtor(u8 *self);
void __fastcall MousePool_Dtor(u8 *self);

// 0x634d00 / 0x634de0 (thiscall): the next free message from the cursor, round the list; a whole round without one adds a new message (0x14 bytes, vtable 0x703d18 / 0x24 bytes,
// vtable 0x703d10; a failed allocation puts a null in the list and faults) before the sentinel. Returns the message at the cursor.
u32 __fastcall KeyPool_Next(u8 *self);
u32 __fastcall MousePool_Next(u8 *self);

}  // extern "C"
