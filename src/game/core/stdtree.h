// The out-of-line members of the MSVC6 red-black tree behind the game's std::map / std::set (<xtree>): iterator increment and erase, emitted once for each of seven
// instantiations that differ in the nil node (a global pointer) and where a node keeps its colour (0 red, 1 black; the node is {left, parent, right, value ..., colour}).
// The head node is the tree object's +4: its left is the leftmost node, its parent the root, its right the rightmost; the object's +0xc is the size. The STL lock the
// originals take is left out. Reimplementation of 0x47d570 / 0x410670 / 0x4f44d0 / 0x4f4f20 / 0x4f5430 / 0x5b2480 / 0x5b2e90 (increment: *it = the next node in order)
// and 0x47c010 / 0x40ff80 / 0x4f3b10 / 0x4f4580 / 0x4f2d50 / 0x5b1b80 / 0x5b2530 (erase: thiscall on the tree, out, the node; the node is unlinked, freed, the size lowered
// and *out = the node after it; returns out). The nil node and colour: A 0x7b4a54 / +0x18, B 0x785de4 / +0x10, C 0x7ced2c / +0x20, D 0x7ced24 / +0x1c, E 0x7ced34 / +0x1c,
// F 0xf7c968 / +0x24, G 0xf7c970 / +0x10. The values of C, D and E start with a std::string (at the node +0xc), whose block erase releases before freeing
// the node (the characters' count byte is lowered, or the block freed 2 bytes before them when it is 0 or 0xff); the value of F starts with one of the game's String (String_Dtor).
#pragma once

#include "../../../re/types.h"

struct StdWString;

extern "C" {

void __fastcall StdTree_IncA(u8 **it);
void __fastcall StdTree_IncB(u8 **it);
void __fastcall StdTree_IncC(u8 **it);
void __fastcall StdTree_IncD(u8 **it);
void __fastcall StdTree_IncE(u8 **it);
void __fastcall StdTree_IncF(u8 **it);
void __fastcall StdTree_IncG(u8 **it);

// The decrement (--iterator: *it = the previous node in order; the end, the head node, goes to the rightmost) of A 0x47d4b0, B 0x410b20, C 0x4f6340, D 0x4f64f0, E 0x4f6680,
// F 0x5b2ff0 and G 0x5b3130.
void __fastcall StdTree_DecA(u8 **it);
void __fastcall StdTree_DecB(u8 **it);
void __fastcall StdTree_DecC(u8 **it);
void __fastcall StdTree_DecD(u8 **it);
void __fastcall StdTree_DecE(u8 **it);
void __fastcall StdTree_DecF(u8 **it);
void __fastcall StdTree_DecG(u8 **it);

// The constructors (thiscall on the tree object) of A 0x47ce70, C 0x4f4120, D 0x4f4b70, E 0x4f5080 and G 0x5b2af0: the nil node (made on the first use: left and right 0,
// parent 0, black) and the head node (red, parent the nil node, left and right itself) are allocated (node sizes 0x1c, 0x24, 0x20, 0x20, 0x14), the head is the object's +4 and
// its size +0xc is 0; the dword counting the maps (0x7b4a50, 0x7ced30, 0x7ced28, 0x7ced38, 0xf7c96c) is raised.
void __fastcall StdTree_CtorA(u8 *tree);
void __fastcall StdTree_CtorC(u8 *tree);
void __fastcall StdTree_CtorD(u8 *tree);
void __fastcall StdTree_CtorE(u8 *tree);
void __fastcall StdTree_CtorG(u8 *tree);

// The insert of A 0x47d140 (values of three dwords whose first, unsigned, is the key), B 0x4107b0 and G 0x5b2ba0 (values of one word, the key; B compares through StdTreeB_KeyLess): thiscall on the tree; out, x, y, the value
// (pops 16). A node holding a copy of the value (at +0xc; red, children nil, parent y) is made and linked: as y's left child when y is the head node, x is not nil or the key
// is below y's, otherwise as its right child; the head's leftmost / rightmost / root follow; then the tree is rebalanced. *out = the new node; returns out.
u8 **__fastcall StdTree_InsertA(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u32 *value);
u8 **__fastcall StdTree_InsertB(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u16 *value);
u8 **__fastcall StdTree_InsertG(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u16 *value);
// 0x4f4c20: the same for tree D (the wide string set: a std::wstring at +0xc, copied by StdWString_Construct; below y's when WString_Compare
// is negative).
u8 **__fastcall StdTree_InsertD(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const StdWString *value);

u8 **__fastcall StdTree_EraseA(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseB(u8 *tree, void *, u8 **out, u8 *node);
// 0x410720 (thiscall on tree B, the tree unused; a node, pops 4): _Erase: the nodes under it freed, the right subtrees by recursion, down the left by a loop.
void __fastcall StdTree_EraseSubtreeB(u8 *tree, void *, u8 *node);
// 0x4f4070 / 0x4f4ac0 / 0x4f4fd0: the same _Erase for trees C, D and E, each node's std::string (+0xc) released as erase does before the node is freed.
void __fastcall StdTree_EraseSubtreeC(u8 *tree, void *, u8 *node);
void __fastcall StdTree_EraseSubtreeD(u8 *tree, void *, u8 *node);
void __fastcall StdTree_EraseSubtreeE(u8 *tree, void *, u8 *node);
// 0x4104b0 (thiscall on tree B; out, first, last, pops 12): erase(first, last). When the tree has nodes, first is the leftmost and last the head, all of it is freed at once
// (_Erase from the root; the head's parent the nil node, its left and right itself, the size 0); otherwise the range is erased node by node (StdTree_IncB, StdTree_EraseB).
// *out = last; returns out.
u8 **__fastcall StdTree_EraseRangeB(u8 *tree, void *, u8 **out, u8 *first, u8 *last);
// 0x4f2590 / 0x4f2960 / 0x4f3290: the same erase(first, last) for trees C, D and E (the whole tree cleared by their _Erase, whose root loop releases each string with
// CountedBuf_Reset; node by node with their increment and erase).
u8 **__fastcall StdTree_EraseRangeC(u8 *tree, void *, u8 **out, u8 *first, u8 *last);
u8 **__fastcall StdTree_EraseRangeD(u8 *tree, void *, u8 **out, u8 *first, u8 *last);
u8 **__fastcall StdTree_EraseRangeE(u8 *tree, void *, u8 **out, u8 *first, u8 *last);
// Tree B's destructor (no address of its own; inlined in 0x40f970): every node erased (StdTree_EraseRangeB), the head node freed, the head and the size zeroed, the map
// counter 0x785de0 lowered and at 0 the nil node 0x785de4 freed and cleared.
void StdTree_DtorB(u8 *tree);
// Tree B's constructor (no address of its own; inlined in 0x405460): as StdTree_CtorA (node size 0x14, the nil node 0x785de4, the map counter 0x785de0).
void StdTree_CtorB(u8 *tree);
// 0x4e5870 / 0x4e5800 / 0x4e58e0 (thiscall): the same destructor for trees C, D and E (StdTree_EraseRangeC / D / E; the map counters 0x7ced30 / 0x7ced28 /
// 0x7ced38, the nil nodes 0x7ced2c / 0x7ced24 / 0x7ced34).
void __fastcall StdTree_DtorC(u8 *tree);
void __fastcall StdTree_DtorD(u8 *tree);
void __fastcall StdTree_DtorE(u8 *tree);
u8 **__fastcall StdTree_EraseC(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseD(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseE(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseF(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseG(u8 *tree, void *, u8 **out, u8 *node);

// 0x410ac0 / 0x47d460 / 0x5b30b0 (thiscall on a tree object whose +4 is the head node; key pointer, pops 4): the lower bound of the key (a word / dword / word, unsigned; at
// +0xc of the nodes, left child at +0, right child at +8, the sentinel the dword at 0x785de4 / 0x7b4a54 / 0xf7c970 holds): the first node not below the key, the head node
// when there is none.
u8 *__fastcall Tree16a_LowerBound(u8 *tree, void *, const u16 *key);

u8 *__fastcall Tree32_LowerBound(u8 *tree, void *, const u32 *key);

u8 *__fastcall Tree16b_LowerBound(u8 *tree, void *, const u16 *key);

// 0x4f0370 / 0x4f0460 (thiscall on an iterator; out, int, pops 8): *out = the old node, the iterator moved on (StdTree_IncC / StdTree_IncE). Returns out.
u8 **__fastcall StdTreeC_PostInc(u8 **it, void *, u8 **out, s32 unused);

u8 **__fastcall StdTreeE_PostInc(u8 **it, void *, u8 **out, s32 unused);

u8 **__fastcall Tree16a_Find(u8 *tree, void *, u8 **out, const u16 *key);

u8 **__fastcall Tree32_Find(u8 *tree, void *, u8 **out, const u32 *key);

u8 **__fastcall Tree16b_Find(u8 *tree, void *, u8 **out, const u16 *key);

u32 __fastcall Obj_HasKey5af780(u8 *object, void *, u32 key);

// 0x47cd00 (thiscall on the tree {allocator, head, multi flag byte at +8}; result {iterator, inserted byte}, key (a pointer to the three-dword value whose first dword is the
// key), pops 8): finds where the key goes (down from the head's parent to the sentinel, to the left when the key is below the node's at +0xc). With the flag set the value is
// always inserted (StdTree_InsertA); otherwise it is only when no node has the key (the position or its predecessor, StdTree_DecA, is compared), and the result is that node
// with inserted 0. The iterator of the result is the new node or the old one.
void __fastcall StdMapA_Insert(u8 *tree, void *, u8 **result, const u32 *value);

// 0x5b1930: the same over the tree with the sentinel at 0xf7c970, whose keys are 16-bit (StdTree_DecG / StdTree_InsertG).
void __fastcall StdMapG_Insert(u8 *tree, void *, u8 **result, const u16 *value);

// 0x40fe60: the same over tree B (the message centre's; sentinel 0x785de4, 16-bit keys; StdTree_DecB / StdTree_InsertB), the result made by Pair_Assign; returns it.
u8 **__fastcall StdMapB_Insert(u8 *tree, void *, u8 **result, const u16 *value);

// 0x4f2810: the same over tree D (the wide string set; StdTree_DecD / StdTree_InsertD): the key compared with each node's by 0x4f7870 (the
// whole key against the node's string, no data being the empty string at 0x6fd6c4) and with the one found by WString_Compare.
u8 **__fastcall StdMapD_Insert(u8 *tree, void *, u8 **result, const StdWString *value);

// 0x5b2f40 (thiscall on the tree {allocator, head}; the key (a string), pops 4): the lower bound: the last node passed whose string (at +0xc) is not below the key
// (String_Less), the head for none.
u8 *__fastcall StdTreeF_LowerBound(u8 *tree, void *, const u8 *key);

// 0x5b0c40 (thiscall on an object whose tree head is at +0x3c): true when every node's mapped object (the pointer at +0x20) has a non-zero byte at +0xb9, else false at the
// first that has not (in order from the leftmost).
u8 __fastcall StdTreeF_AllFlagged(u8 *object);

// 0x5b20c0 (thiscall; out, key, pops 8): *out = the lower bound; returns out.
u8 **__fastcall StdTreeF_FindLower(u8 *tree, void *, u8 **out, const u8 *key);

// 0x5b1650 (thiscall on the tree; out, value, pops 8): StdMapG_Insert's result {iterator, inserted byte} copied to out. Returns out.
u32 *__fastcall StdMapG_InsertCopy(u8 *tree, void *, u32 *out, const u16 *value);

// 0x4f0390: the same for tree D (StdMapD_Insert).
u32 *__fastcall StdMapD_InsertCopy(u8 *tree, void *, u32 *out, const StdWString *value);

// 0x5af740 (thiscall on an object whose tree is at +0x28; key (a 16-bit value passed on the stack), amount, pops 8): inserts the key; true (and +0x1c grows by the amount)
// when it was new, else false.
u8 __fastcall Object_AddKeyed(u8 *object, void *, u32 key, s32 amount);

// 0x40f950 (thiscall; &other, pops 4): tree B's key compare: 1 when the word key is below other's (unsigned), else 0.
u32 __fastcall StdTreeB_KeyLess(const u16 *key, void *, const u16 *other);

// 0x47b060 (thiscall; &comp, &alloc, pops 8): A's map constructor: +0 = the alloc byte, +1 = the comp byte, +8 = 0 (no
// multi-insert), then the tree as StdTree_CtorA makes it. Returns the map.
u8 *__fastcall StdMapA_Ctor(u8 *map, void *, const u8 *comp, const u8 *alloc);

// 0x47b130 (thiscall; &key, pops 4): A's operator[]: StdMapA_Insert of {key, 0, 0}, then the found or new node's value
// (+0x10).
u32 *__fastcall StdMapA_Index(u8 *map, void *, const u32 *key);

// Tree H, the sound code's map of dword keys to Strings (std::map<u32, String>; nil node 0xfb2008, its map counter 0xfb200c, nodes of 0x28 bytes, the colour at
// +0x24; the value {key, String} at +0xc, the String at +0x10, released with String_Dtor): its increment 0x6c2de0, decrement 0x6c2d20, insert 0x6c29b0 (the node's
// value made by StdPairH_Construct), erase 0x6c1b70, _Erase 0x6c20b0, erase(first, last) 0x6c1270 and destructor 0x6c11c0, as the other trees'.
void __fastcall StdTree_IncH(u8 **it);
void __fastcall StdTree_DecH(u8 **it);
u8 **__fastcall StdTree_InsertH(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u8 *value);
u8 **__fastcall StdTree_EraseH(u8 *tree, void *, u8 **out, u8 *node);
void __fastcall StdTree_EraseSubtreeH(u8 *tree, void *, u8 *node);
u8 **__fastcall StdTree_EraseRangeH(u8 *tree, void *, u8 **out, u8 *first, u8 *last);
void __fastcall StdTree_DtorH(u8 *tree);
// 0x6c1a90 (thiscall on the map; result, value, pops 8): H's insert(value) as StdMapA_Insert: result = {the found or new node, inserted byte}.
void __fastcall StdMapH_Insert(u8 *tree, void *, u8 **result, const u8 *value);
// 0x6c1130 (thiscall; &key, pops 4): H's operator[]: {key, an empty String} inserted (the pair's String a copy of an empty one; both released after), then
// the found or new node's String (+0x10).
u8 *__fastcall StdMapH_Index(u8 *map, void *, const u32 *key);
// 0x6c30a0 (cdecl; where, value): unless where is null, StdPairH_Copy there. 0x6c3110 (thiscall; value, pops 4): the pair {key, String} copied (String_CtorCopy);
// returns it.
void __cdecl StdPairH_Construct(u8 *where, const u8 *value);
u8 *__fastcall StdPairH_Copy(u8 *pair, void *, const u8 *value);
// 0x6c0b10 (thiscall on the map; from, key, pops 8; name unconfirmed): map[key] = the WString from (String_AssignWide, the String's vtable +0x18). Returns
// the String.
u8 *__fastcall StdMapH_AssignWide(u8 *map, void *, const u8 *from, u32 key);
// 0x6c0b30 (thiscall on the map; text, &index, pops 8; name unconfirmed): the first index i, from 0 while below the size (read again each time), whose
// map[i] (StdMapH_Index: a missing key is inserted, growing the size) String_Equals text: *index = i and 1; else 0 (index untouched).
u32 __fastcall StdMapH_FindValue(u8 *map, void *, const u8 *text, u32 *index);
// 0x6c2140 (thiscall): H's constructor, as the other trees'.
void __fastcall StdTree_CtorH(u8 *tree);
// 0x6c0a60 (thiscall; name invented): a map H destroyed after a clear: erase(begin, end), then the destructor's own (StdTree_DtorH's work
// inlined: a second erase over the now empty map, the head freed, head and size 0, the nil node freed by the last map out).
void __fastcall StdMapH_ClearDtor(u8 *map);
// 0x6c0ba0 (thiscall (key), pops 4; name invented): for a key from 1 to below the size (unsigned), map[key] (StdMapH_Index: a missing
// key inserted) set to the empty text 0x786de4 through its String's vtable +0x14 (String_AssignStr).
void __fastcall StdMapH_ClearAt(u8 *map, void *, u32 key);

// Tree I, another of the sound code's trees (H's neighbour; nil node 0xfb2010, its counter 0xfb2014, the colour at +0x14, plain values: nodes of 0x18
// bytes presumably, freed as they are): its increment 0x6c3910, erase 0x6c3340, _Erase 0x6c3870, erase(first, last) 0x6c31e0 and destructor 0x6c3130,
// as the other trees'.
void __fastcall StdTree_IncI(u8 **it);
u8 **__fastcall StdTree_EraseI(u8 *tree, void *, u8 **out, u8 *node);
void __fastcall StdTree_EraseSubtreeI(u8 *tree, void *, u8 *node);
u8 **__fastcall StdTree_EraseRangeI(u8 *tree, void *, u8 **out, u8 *first, u8 *last);
void __fastcall StdTree_DtorI(u8 *tree);
// Tree J, the sound code's map of dword keys to trees I (std::map<u32, tree I>; nil node 0xfb2000, its counter 0xfb2004, nodes of 0x24 bytes, the colour
// at +0x20; the value {key, tree I} at +0xc, the tree object at +0x10, released as StdTree_DtorI, inlined): its increment 0x6c2e90, constructor 0x6c2860,
// erase 0x6c21f0, _Erase 0x6c2760 and erase(first, last) 0x6c13e0 (its whole-tree branch calling StdTree_DtorI), as the other trees'.
void __fastcall StdTree_IncJ(u8 **it);
void __fastcall StdTree_CtorJ(u8 *tree);
u8 **__fastcall StdTree_EraseJ(u8 *tree, void *, u8 **out, u8 *node);
void __fastcall StdTree_EraseSubtreeJ(u8 *tree, void *, u8 *node);
u8 **__fastcall StdTree_EraseRangeJ(u8 *tree, void *, u8 **out, u8 *first, u8 *last);

// 0x410be0 / 0x410c00 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x7899b4 = 1 and 0x78a0bc = 1 / 2^23 (as
// camera's).
void __cdecl StdTree_InitUnusedOne();
void __cdecl StdTree_InitUnusedEpsilon();
}  // extern "C"
