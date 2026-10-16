// Two members of the standard lists: the duplicate removal of a list of counted strings and the merge of two sorted lists of dwords: reimplementation of 0x5ce080 and
// 0x5d0590. Lists are {allocator, head, size}; a node is {next, previous, value...} and the head is a node whose next is the first element.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5ce080 (thiscall): removes every element equal to the one before it from the list of {+0xc characters (null: ""), +0x10 length, +0x14 capacity} strings: the node leaves
// the list, its counted string (the count byte before the characters: released, the block freed at 0 or 0xff) is let go, its fields cleared, the node freed and the size
// decremented. Equal means the same length and bytes.
void __fastcall StdListString_Unique(u8 *list);

// 0x5d0590 (thiscall; other list, pops 4): merges the other sorted list into this one (an element of the other goes before the first of this that it is below, unsigned);
// what is left of the other goes to the end; the size is the sum and the other's is 0. Nothing for the list itself.
void __fastcall StdListDword_Merge(u8 *list, void *, u8 *other);

}  // extern "C"
