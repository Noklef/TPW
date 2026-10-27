// Loops over arrays and lists of objects that call a virtual method of each: reimplementation of 0x5ed460, 0x5ed500, 0x5ed530, 0x5e40e0, 0x5e4110, 0x5d5cc0 and 0x5d5c50.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5ed460 (stdcall; first, last, pops 8): each 0x14-byte element from first up to last has its slot 0xc called with 0 (the destroy of a range).
void __stdcall Range14_CallSlotC(u8 *first, u8 *last);

// 0x5ed500 (cdecl; first, last, value): each element from first up to last has its slot 0x18 called with value (the fill of a range).
void __cdecl Range14_CallSlot18(u8 *first, u8 *last, u8 *value);

// 0x5ed530 (cdecl; first, last, destEnd): going backwards from destEnd, each 0x14-byte destination element before it has its slot 0x18 (read before stepping
// back, from the element itself) called with the matching source element before last, until first is reached (the backward copy of a range). Returns the
// first destination element it filled (destEnd when nothing).
u8 *__cdecl Range14_CopyBackward(u8 *first, u8 *last, u8 *destEnd);

// 0x5e40e0 / 0x5e4110 (thiscall): over the list whose sentinel is at +0x18 (the first node at its +4; nodes {object, next}): each object's slot 0x24 is called and when
// it returns non-zero its slot 8 / 0xc is called too.
void __fastcall List18_ExpireSlot8(u8 *object);
void __fastcall List18_ExpireSlotC(u8 *object);

// 0x5d5cc0 (thiscall): over the same list each object's slot 0xc is called. Returns the dword at +0x14.
u32 __fastcall List18_CallSlotC(u8 *object);

// 0x5d5c50 (thiscall; the object [0xf82884] points to, called from 0x54e73a before 0x5d5cc0; name unconfirmed): a frame's tick. +0x28 = the
// last time (+0x2c), +0x2c = the real time now (Clocks_RealNow of the game clock 0x785970), +0x30 = their difference (unsigned), at most 500;
// +8 = +0x30 * the float [0x7029cc] (0.01; fild of the qword, fmul, stored as a float). Then over the list at +0x18 each object's slot 8 is
// called (the next node read first). Returns the dword at +0x14.
u32 __fastcall List18_TickSlot8(u8 *object);

}  // extern "C"
