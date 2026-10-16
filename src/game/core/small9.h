// More tiny constructors, containers and comparisons: reimplementation of 0x506580, 0x50e260, 0x5101f0, 0x5e2a30, 0x55f720, 0x5b96e0,
// 0x5b9740, 0x5d1330, 0x4791c0 and 0x47aba0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x506580 (thiscall; word, pops 4): for the kind byte at +2 being 4 the word at +0x218, for 5 the word at +0x21a, is equal to the low 16 bits of
// word (1 or 0); any other kind gives 0 (the original reports it to its empty debug printf).
u32 __fastcall Person_HasWord(const u8 *object, void *, u32 word);

// 0x50e260 (stdcall; out, text): the globals 0x7cf100 and 0x7cf104 = 0 and the two dwords at out = those (the original's debug printf is
// where a measurement would be made).
void __stdcall TextExtent_Zero(u32 *out, const u8 *text);

// 0x5101f0 (thiscall): a new block of 0x50 bytes cleared (when there is one); the dwords at +0 and +4 = the block (0 for none) and the dword at
// +8 = the block + 0x280 (sic: 0x280 for none). Returns the object.
u8 *__fastcall Block50_Ctor(u8 *object);

// 0x5e2a30 (thiscall; item, pops 4): a new node of 12 bytes (item, 0, 0) is made the head of the list ({?, head +4, tail +8, count +0xc}; the
// node's +4 is the old head, the old head's +8 the node, the tail the node too for an empty list) and the count rises. Returns the node
// for an empty list, otherwise the new count.
u32 __fastcall List_PushHead(u8 *list, void *, u32 item);

// 0x55f720 (thiscall; size, pops 4): the byte at +0 = size, the byte at +1 = 0 for 0x40, 2 for 0x10, 1 for 0x20 and any other size (sic: which
// also becomes 0x20); the buffer at 0x8798c8 is freed (when there is one) and a new one of byte(+0)^2 * 4 bytes is made.
void __fastcall Lnd_SetTileSize(u8 *object, void *, u32 size);

// 0x5b96e0 / 0x5b9740 (thiscall): the fields at +4..+0x30 cleared, the vtable pointer at +0 = 0x702880 / 0x702890, a new member of 16 bytes
// (vtable 0x7028a8 / 0x7028ac, +4, +8, +0xc = 0) at +0x2c (0 for none). Returns the object.
u8 *__fastcall Class702880_Ctor(u8 *object);
u8 *__fastcall Class702890_Ctor(u8 *object);

// 0x5d1330 (thiscall; other, pops 4): the order of the byte strings (pointer +4, length +8; a null pointer is the empty text at 0x6fe5b0) of
// the object and other: the bytes of the common length are compared as unsigned (-1 or 1 at the first difference), then the lengths
// (-1 shorter, 1 longer, 0 equal).
s32 __fastcall ByteString_Compare(const u8 *object, void *, const u8 *other);

// 0x4791c0 (thiscall): the counted buffer (see small6) at +4 of each 16-byte entry from [+0x14] up to [+0x18] released and its fields cleared;
// then the block at +0x14 freed and the dwords at +0x14, +0x18 and +0x1c = 0.
void __fastcall CountedVector14_Clear(u8 *object);

// 0x47aba0 (thiscall): the same with the entries from [+4] up to [+8] and the fields +4, +8 and +0xc.
void __fastcall CountedVector4_Clear(u8 *object);

}  // extern "C"
