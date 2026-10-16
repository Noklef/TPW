// Two walks over the map keyed by game strings (the instantiation with the sentinel at 0xf7c968 and a pointer as the mapped value at +0x20): reimplementation of 0x5b2f40
// and 0x5b0c40. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b2f40 (thiscall on the tree {allocator, head}; the key (a string), pops 4): the lower bound: the last node passed whose string (at +0xc) is not below the key
// (String_Less), the head for none.
u8 *__fastcall StdTreeF_LowerBound(u8 *tree, void *, const u8 *key);

// 0x5b20c0 (thiscall; out, key, pops 8): *out = the lower bound; returns out.
u8 **__fastcall StdTreeF_FindLower(u8 *tree, void *, u8 **out, const u8 *key);

// 0x5b0c40 (thiscall on an object whose tree head is at +0x3c): true when every node's mapped object (the pointer at +0x20) has a non-zero byte at +0xb9, else false at the
// first that has not (in order from the leftmost).
u8 __fastcall StdTreeF_AllFlagged(u8 *object);

}  // extern "C"
