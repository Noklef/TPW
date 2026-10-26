// The sound code's shuffle bag (the class and all names are invented; its users are near 0x6be3c4, 0x6be406 and 0x6be439): +0 a list (+4
// its count, +8 its entries, 0x2a bytes each), +4 a random seed, +8 the next entry's index, the bytes +0xc / +0xd the two strides
// (1 and 2) and +0xe which of them comes next. Reimplementation of 0x6be100-0x6be24e.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6be140 (thiscall): the index (+8) 0, then the seed (+4) stepped (0x19660d x + 0x3c6ef35f); rotated right 13 modulo the count (DIV:
// a count of 0 faults) it picks k. With a = entry k - 1 and b = entry k (as addresses, unsigned): while a is not below the entries and b
// is below the last entry, a and b swap their 0x2a bytes, a steps back one entry, the flag +0xe flips and b steps on by the stride it
// now picks (+0xc + the flag) entries. The list and its entries are re-read for each test.
void __fastcall ShuffleBag_Shuffle(u8 *bag);
// 0x6be130 (thiscall (list), pops 4): +0 = list, then ShuffleBag_Shuffle.
void __fastcall ShuffleBag_SetList(u8 *bag, void *, u8 *list);
// 0x6be100 (thiscall (list), pops 4): +8 and the flag +0xe 0, the strides 1 and 2, the seed Time_GetMs (), then ShuffleBag_SetList.
// Returns the bag.
u8 *__fastcall ShuffleBag_Ctor(u8 *bag, void *, u8 *list);
// 0x6be210 (thiscall (refill), pops 4): with the index (+8) not below the count, 0 without refill, else ShuffleBag_Shuffle. Returns
// the entry at the index, which steps on.
u8 *__fastcall ShuffleBag_Next(u8 *bag, void *, u32 refill);

}  // extern "C"
