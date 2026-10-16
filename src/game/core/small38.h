// The insert of the map with the sentinel at 0x7b4a54: reimplementation of 0x47cd00. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47cd00 (thiscall on the tree {allocator, head, multi flag byte at +8}; result {iterator, inserted byte}, key (a pointer to the three-dword value whose first dword is the
// key), pops 8): finds where the key goes (down from the head's parent to the sentinel, to the left when the key is below the node's at +0xc). With the flag set the value is
// always inserted (StdTree_InsertA); otherwise it is only when no node has the key (the position or its predecessor, StdTree_DecA, is compared), and the result is that node
// with inserted 0. The iterator of the result is the new node or the old one.
void __fastcall StdMapA_Insert(u8 *tree, void *, u8 **result, const u32 *value);

}  // extern "C"
