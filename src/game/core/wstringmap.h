// A map keyed by wide strings (a red-black tree of nodes {left, parent, right, the key's string at +0x10}): reimplementation of the bounds 0x4f6400 / 0x4f6940 and the
// lookup 0x4f2b40 and the erase by key 0x4f2ad0. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f6400 / 0x4f6940 (thiscall on the tree {allocator, head}; the key {allocator, characters (null: the empty string 0x6fd6c4), length}, pops 4): the lower / upper bound
// of the key among the nodes {left, parent, right, the key's string at +0x10 (characters, length)} under the head's parent, down to the sentinel at 0x7ced24: the last node
// passed whose string is not below / is above the key (a lexicographic comparison of the 16-bit characters, then the length); the head when there is none.
u8 *__fastcall WStringMap_LowerBound(u8 *tree, void *, const u8 *key);

u8 *__fastcall WStringMap_UpperBound(u8 *tree, void *, const u8 *key);

// 0x4f2b40 (thiscall on the map; out, key, pops 8): the node WStringMap_LowerBound finds for the key when it is not the head (the dword at +4) and the key does not sort before the
// node's string, else the head. *out is set and out returned.
u32 *__fastcall WStringMap_Find(u8 *map, void *, u32 *out, const u8 *key);

// 0x4f2ad0 (thiscall on the map; key, pops 4; name unconfirmed): erase(key): the nodes from WStringMap_LowerBound to WStringMap_UpperBound
// counted (StdTree_IncD) and erased (StdTree_EraseRangeD: the map's tree is stdtree's D). Returns the count.
u32 __fastcall WStringMap_EraseKey(u8 *map, void *, const u8 *key);

}  // extern "C"
