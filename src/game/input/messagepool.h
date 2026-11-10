// The input messages' pools: a circular list (its sentinel at +8, the count at +0xc) of messages with a cursor at +0x10; a free message has +8 clear. Reimplementation of 0x634a50, 0x634b40, 0x634d00, 0x634de0.
// Names invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x634a50 / 0x634b40 (thiscall): the destructors: vtable 0x703d1c / 0x703d14; the messages freed (Mem_Free, without their destructors), then the nodes and the sentinel; +8 = +0xc = 0.
void __fastcall KeyPool_Dtor(u8 *self);
void __fastcall MousePool_Dtor(u8 *self);
// 0x634b00 / 0x634b20 (thiscall (flags), pops 4): the destructor; with bit 0, freed. Returns self.
u8 *__fastcall KeyPool_DeletingDtor(u8 *self, void *, u32 flags);
u8 *__fastcall MousePool_DeletingDtor(u8 *self, void *, u32 flags);

// 0x6345d0 (thiscall): a device filter with a key pool at +0x34 and a mouse pool at +0x48, one message in each; vtables 0x703cf8, 0x703ce8 (+0x10). 0x634760 (thiscall): undone, the
// mouse pool first. 0x634740 (thiscall (flags), pops 4): with bit 0, freed. Returns self.
u8 *__fastcall MessageSource_Ctor(u8 *self);
void __fastcall MessageSource_Dtor(u8 *self);
u8 *__fastcall MessageSource_DeletingDtor(u8 *self, void *, u32 flags);
// 0x634ee0 (thiscall): a device filter with a mouse-sized pool at +0x34 (vtable 0x703d20), +0x14 = +0x18 = 6; vtables 0x703d38, 0x703d28 (+0x10). 0x635000 (thiscall): undone.
// 0x634fe0 (thiscall (flags), pops 4): with bit 0, freed. Returns self.
u8 *__fastcall KeySource_Ctor(u8 *self);
void __fastcall KeySource_Dtor(u8 *self);
u8 *__fastcall KeySource_DeletingDtor(u8 *self, void *, u32 flags);

// 0x634900 (thiscall (count), pops 4): count records of 0x10 bytes from the block at +0x30 (type, value, time) sent to the object at +0xc: types 0, 4, 8 as a key message from the pool
// at +0x34 (slot 0x14), 0xc-0xf as a mouse message from the pool at +0x48 with the button 0xf0-0xf3 and the value's low byte and bit 7 (as 8) (slot 0x18). Other types: nothing.
void __fastcall MessageSource_Deliver(u8 *self, void *, u32 count);
// 0x6350f0 (thiscall (count), pops 4): as 0x634900 for a source with only the mouse pool's kind at +0x34: every record a message with the low byte of its type.
void __fastcall MessageSource_DeliverKeys(u8 *self, void *, u32 count);

// 0x634d00 / 0x634de0 (thiscall): the next free message from the cursor, round the list; a whole round without one adds a new message (0x14 bytes, vtable 0x703d18 / 0x24 bytes,
// vtable 0x703d10; a failed allocation puts a null in the list and faults) before the sentinel. Returns the message at the cursor.
u32 __fastcall KeyPool_Next(u8 *self);
u32 __fastcall MousePool_Next(u8 *self);

// 0x634a30 (thiscall; result out, pops 4): DeviceFilter_Open into a local, copied to out. Returns out.
u32 *__fastcall MessageSource_Open(u8 *self, void *, u32 *result);

}  // extern "C"
