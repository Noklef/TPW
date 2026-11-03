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

// Sets of the second wide string copy (its traits compare without case: stdwstring's WStdString_*), each its own instantiation with its
// own sentinel: the online chat avatar's ignore list (+0xea0; sentinel 0x7ced34) and buddy list (+0xe90; sentinel 0x7ced2c). Names
// invented, unconfirmed.
// 0x4f65b0 / 0x4f6270 (thiscall on the set; key, pops 4): the lower bound: the last node passed whose string (+0x10 characters, passed as
// they are, +0x14 length) is not below the key's (null: 0x700624) by WStdString_Compare's order; the head when there is none.
u8 *__fastcall ChatIgnoreSet_LowerBound(u8 *set, void *, const u8 *key);
u8 *__fastcall ChatBuddySet_LowerBound(u8 *set, void *, const u8 *key);

// 0x4f6870 (thiscall on the buddy set; key, pops 4; name invented): the upper bound: the last node passed whose string (+0x10 characters, null: 0x700624; +0x14 length) is above
// the key's (+4 characters, passed as they are, +8 length) by that order; the head when there is none. The STL lock left out.
u8 *__fastcall ChatBuddySet_UpperBound(u8 *set, void *, const u8 *key);

// 0x4f2700 (thiscall on the buddy set; key, pops 4): the entries of key erased; returns how many. Name invented, unconfirmed.
u32 __fastcall ChatBuddySet_EraseKey(u8 *set, void *, const u8 *key);

// 0x4f03c0 / 0x4f2770 (thiscall on the set; out, key, pops 8): find: the lower bound when it is not the head and the key's characters
// (passed as they are) do not sort before its string (null: 0x700624), else the head. *out is set and out returned.
u32 *__fastcall ChatIgnoreSet_Find(u8 *set, void *, u32 *out, const u8 *key);
u32 *__fastcall ChatBuddySet_Find(u8 *set, void *, u32 *out, const u8 *key);

// 0x4f4050 (thiscall on the buddy set; out, key, pops 8): *out = ChatBuddySet_LowerBound(key); returns out. Name invented, unconfirmed.
u32 *__fastcall ChatBuddySet_LowerBoundOut(u8 *set, void *, u32 *out, const u8 *key);

}  // extern "C"
