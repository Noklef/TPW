// The online items an owner keeps in its list at +0x2c (see online/itemlist for the searches over it): reimplementation of their constructors
// (0x5bdac0, 0x5bdcd0, 0x5bde30, 0x5be1c0, 0x5be3e0) and 0x70293c's assignment (0x5bdfd0). Their destructors are in core/vdtor and core/vdel.
//
// Every item is {vtable, owner (+4), id (+8), ...}; a constructor from a record copies the record's fields, gives its strings the record's texts
// (the vtable's +0x14, AssignStr) and then appends a node for the item at the tail of the owner's list (PtrList_Append's code, inline). Records
// hold their texts as pointers to wide (UTF-16) strings and their wider fields unaligned.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5bdac0 (thiscall; record, owner; pops 8): the 0x702938 item (0x68 bytes): wide strings at +0xc and +0x20 (texts record +4 and +8) and +0x44
// (+0x1c); +0x34, +0x38 and +0x3c the record's floats at +0xc, +0x14 and +0x10 times 1.01 (0x702930, a double; x87, 24-bit precision);
// +0x40 = record +0x18, +0x58..+0x64 = record +0x20..+0x2c. Appended to the owner's list. Returns the item.
u8 *__fastcall Class702938_Ctor(u8 *item, void *, const u8 *record, u8 *owner);

// 0x5bdcd0 (thiscall): the 0x70293c item (0xc0 bytes) empty: wide strings at +0xc, +0x20, +0x34 and +0x94 given the empty wide text 0x78a374,
// a string at +0x7c the empty text 0x786de4; owner 0, id -1, +0x48..+0x54 -1, the 32 bytes at +0x58 0xff, +0x78 and +0x90 -1, the words at
// +0xa8, +0xaa and +0xac 0xffff, +0xb0..+0xbc -1. Not listed anywhere. Returns the item.
u8 *__fastcall Class70293c_Ctor(u8 *item);

// 0x5bde30 (thiscall; record, owner; pops 8): the 0x70293c item from a record: the wide strings' texts record +4, +8, +0xc and +0x4c, the
// string's (+0x7c) record +0x44 (a narrow text); +0x48..+0x54 record +0x10..+0x1c, the 32 bytes at +0x58
// record +0x20, +0x78 record +0x40, +0x90 record +0x48, the words record +0x50..+0x54, +0xb0..+0xbc record +0x56..+0x62 (unaligned).
// Appended to the owner's list. Returns the item.
u8 *__fastcall Class70293c_CtorFrom(u8 *item, void *, const u8 *record, u8 *owner);

// 0x5bdfd0 (thiscall; other; pops 4): the 0x70293c item's assignment: every field but the owner (+4) copied from other, the strings through
// their vtables' +0x18 (Assign). Returns the item.
u8 *__fastcall Class70293c_Assign(u8 *item, void *, const u8 *other);

// 0x5be1c0 (thiscall; record, owner; pops 8): the 0x702940 item (0x48 bytes): narrow strings at +0xc, +0x20 and +0x34 given the wide texts at
// record +4, +8 and +0xc through a temporary wide string (its AssignStr, then String_AssignFromWide). Appended to the owner's list. Returns
// the item.
u8 *__fastcall Class702940_Ctor(u8 *item, void *, const u8 *record, u8 *owner);

// 0x5be3e0 (thiscall; record, owner; pops 8): the 0x702944 item (0x80 bytes): +0xc = record +4; wide strings at +0x10 and +0x24 (texts
// record +8 and +0xc), +0x40 (+0x16), +0x54 (+0x1a) and +0x6c (+0x22); +0x38 = record +0x10, the word +0x3c = record +0x14, +0x68 = record
// +0x1e. Appended to the owner's list. Returns the item.
u8 *__fastcall Class702944_Ctor(u8 *item, void *, const u8 *record, u8 *owner);
}
