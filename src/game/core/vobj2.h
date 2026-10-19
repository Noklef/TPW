// Methods that hand their members to virtual methods and little else (COM releases, deleting destructors, drains of lists of objects, a loop over the slots of
// pointed-to objects): reimplementation of 0x47e460, 0x5e4140, 0x421520, 0x421720, 0x5b7990, 0x5cea40, 0x4f0650, 0x579c80, 0x58c370, 0x5e1fa0 and 0x5e4020,
// the second saved pairs' release (0x57b2e0) and the block at +0x13c made and freed (0x57b490, 0x57b5b0).
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47e460 (thiscall): the dwords at +0xa8 and +0xa4 are cleared; the COM objects at +0xac and +0xb0 (when set) are released (slot 8), the pointers cleared.
void __fastcall Obj_ReleaseTwoComMembers(u8 *object);

// 0x5e4140 (thiscall): the objects of the list at +0x18 (nodes {object, next} from the head at +4) each get their slot 0x10 called; then the members at +0x14,
// +0x10, +0xc, +8 and +4 (in that order) are deleted (slot 0 with 1) when set and cleared.
void __fastcall Obj_DeleteFiveMembers(u8 *object);

// 0x421520 (thiscall on a pointer to a list): while the list (+4 head, +8 tail, +0xc count; the count compared unsigned) has entries the head node is popped, freed and
// the object it held deleted (slot 0 with 1); then the list itself is deleted (slot 0 with 1) and the pointer cleared.
void __fastcall ListRef_DrainAndDelete(u8 **ref);

// 0x421720 (thiscall on a pointer to a list; a, b, pops 8): over the nodes of the list (head at +4) the held object slot 8 is called with a, b and 0x80000001; 1 as
// soon as one returns 1, otherwise 0.
u32 __fastcall ListRef_AnyHandles(u8 **ref, void *, u32 a, u32 b);

// 0x5b7990 (thiscall; flag, text, table, pops 12): the flag is stored at +0x108; when it is not 0 the member object at +0x10c gets its slot 0x14 called with the text
// (the string at 0x786de4 when 0) and the 256 words at the table are copied to +0x120.
void __fastcall Obj_SetNameAndWords(u8 *object, void *, u32 flag, u8 *text, const u16 *table);

// 0x5cea40 / 0x4f0650 (thiscall): the vtable pointer 0x709914 / 0x7099a4 is set; the counted object at +0x34 loses a use (the count at +4 is lowered unless it is 0 or
// -1) and is deleted (slot 0 with 1) when the count is then 0.
void __fastcall Class709914_Dtor(u8 *object);
void __fastcall Class7099a4_Dtor(u8 *object);

// 0x579c80 (thiscall; returns 1): for each of the +0x400 pointers from the start of the object the flag 0x10 is set in the dword at +4 of the thing it points to and the
// virtual base of that thing (the offset in dword +4 of its vtable) gets its slot 4 called.
u32 __fastcall Pointers_MarkAndCallSlot4(u8 *object);

// 0x58c370 (thiscall on the field 0x3c into an object; flags, pops 4: bit 0 also frees the object): the virtual base of the object is given the vtable pointer
// 0x7021f0, the word 4 below it the offset of the field, and its slot 4 is called; the vtable pointer of the object at +0x3c is set to 0x702198. Returns the object.
u8 *__fastcall Class702198_DeletingDtor(u8 *field, void *, u32 flags);

// 0x5e1fa0 (thiscall; flag, pops 4): +0xc is set to the first object of the list at +8 -> +4; with the flag the object at 0xf85480 gets its slot 0x14 called with the
// string at 0x786de4, otherwise +0xc is set to each object of the list whose name at +0x30 equals the one the dword at 0xf85488 points to (the last one wins).
void __fastcall Obj_PickByName(u8 *object, void *, u32 flag);

// 0x5e4020 (thiscall): the vtable pointer 0x702da0 is set, +8 = 0x702f10, +0x10 and +0xc cleared and the chain of nodes (next at +4) that was at +0xc freed; +0x14 is
// cleared and the vtable pointer 0x702d48 set; the object the dword at 0xf85614 points to gets its slot 0x1c called with the object.
void __fastcall Class702da0_Dtor(u8 *object);

// 0x57b070 (thiscall on the object): with flag 0x20000000 of the flags (+4) and a positive count (+0x12c) the saved pairs {value, pointer} at +0x124 are written back (the value to the
// word the pointer names, the last pair first) and the array freed (Mem_Free); the 0x10-byte elements at +0x128 (their count the dword before them) are destroyed (the dtor at
// 0x575bf0) and their block freed from 4 bytes before; the flag is cleared. In every case the count is cleared. Returns 1.
u32 __fastcall Object_ReleaseSavedPairs(u8 *object);

// 0x57b2e0 (thiscall on the object; name unconfirmed): Object_ReleaseSavedPairs' second set: with flag 0x40000000 of the flags (+4) and
// a positive count (+0x138, re-read each time) the saved pairs {value, pointer} at +0x130 are written back from the first, each only when
// its value is not 0; the flag is cleared, the array freed (Mem_Free) and cleared, the 0x10-byte elements at +0x134 (their count the dword
// before them) destroyed (the dtor at 0x575bf0) and their block freed from 4 bytes before, and the count cleared. Without the flag or a
// positive count the count stays. Returns 1.
u32 __fastcall Object_ReleaseSavedPairs130(u8 *object);

// 0x57b490 (thiscall on the object; name unconfirmed): with a block at +0x13c already, 0 (sic). Else a tracked block of 0x38
// (TrackedGlobal_Alloc) goes there and its +0x18 gets a new texture (0x60 bytes, GameHeap_Alloc, Texture_Ctor; none got: 0) whose pixels
// are allocated (Texture_Alloc, the size [0x8bd4b0], flags 0): 1. On a failure the texture (when set) is released (through 0x590a70) and
// freed, the block freed (TrackedGlobal_Free) and cleared, the error (2, 0xb0754000) pushed (EngineErrors_Push) and 0.
u32 __fastcall Object_MakeBlock13c(u8 *object);

// 0x57b5b0 (thiscall on the object; name unconfirmed): with a tracked block at +0x13c: its texture at +0x18 (when set) is released
// (Texture_Release, gfx/texture), then, still set, released again through the jump 0x590a70 (sic) and freed (GameHeap_Free); the block
// is freed (TrackedGlobal_Free, core/gameheap) and cleared. Returns 1.
u32 __fastcall Object_FreeBlock13c(u8 *object);

// 0x564140 (thiscall; returns 1): Obj_ReleaseSevenComMembers on the object when the dword at 0x8bd524 is 1.
u32 __fastcall Obj_ReleaseSevenIfMode1(u8 *object);
// 0x57b6e0 (thiscall on the object; the mesh; pops 4; name unconfirmed): the mesh's vertices (+0x28, count +0x20) packed (FixedVerts_Pack,
// stride 0x20), the software renderer's buffer ([0x879dac]) cleared and the mesh drawn ([0x879acc]'s virtual +0x14). With the pixel format
// [0x8bd554] 2 the buffer's dwords (+8) are averaged into the block's texture (+0x13c, its +0x18, Texture_Lock): the width [0x8bd4ac]
// over the count [0x8bd4b0] (a 0 count faults) is the side of each of count x count square blocks, read row by row; each block's mean of
// its pixels' byte 2 (over side * side, signed; a side of 0 faults) becomes a grey 4444 word 0xf000 | hi << 8 | hi << 4 | hi (hi its
// top four bits; from 0xf0 | 0xff00 shifted and ORed as the original does). The width and count are read again after each block. Returns 1.
u32 __fastcall Object_RenderToBlock13c(u8 *object, void *, u8 *mesh);
}  // extern "C"
