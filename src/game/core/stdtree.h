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

extern "C" {

void __fastcall StdTree_IncA(u8 **it);
void __fastcall StdTree_IncB(u8 **it);
void __fastcall StdTree_IncC(u8 **it);
void __fastcall StdTree_IncD(u8 **it);
void __fastcall StdTree_IncE(u8 **it);
void __fastcall StdTree_IncF(u8 **it);
void __fastcall StdTree_IncG(u8 **it);

u8 **__fastcall StdTree_EraseA(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseB(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseC(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseD(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseE(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseF(u8 *tree, void *, u8 **out, u8 *node);
u8 **__fastcall StdTree_EraseG(u8 *tree, void *, u8 **out, u8 *node);

}  // extern "C"
