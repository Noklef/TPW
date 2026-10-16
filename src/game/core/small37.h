// The bounds of a map keyed by wide strings: reimplementation of 0x4f6400 and 0x4f6940. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f6400 / 0x4f6940 (thiscall on the tree {allocator, head}; the key {allocator, characters (null: the empty string 0x6fd6c4), length}, pops 4): the lower / upper bound
// of the key among the nodes {left, parent, right, the key's string at +0x10 (characters, length)} under the head's parent, down to the sentinel at 0x7ced24: the last node
// passed whose string is not below / is above the key (a lexicographic comparison of the 16-bit characters, then the length); the head when there is none.
u8 *__fastcall WStringMap_LowerBound(u8 *tree, void *, const u8 *key);
u8 *__fastcall WStringMap_UpperBound(u8 *tree, void *, const u8 *key);

}  // extern "C"
