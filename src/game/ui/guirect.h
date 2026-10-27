// The GUI library's rectangle (vtable 0x705920, over the base 0x705938): four words, left +4, top +6, right +8 and bottom +0xa (right and
// bottom exclusive). Dialogs (0x7058f8) hold two at +0x14 and +0x20. Reimplementation of 0x659d00-0x65aa20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x659d00 (thiscall; name unconfirmed): the base constructor (Fn_65a9d0), the vtable 0x705920 and the four words 0; returns the rectangle.
u8 *__fastcall GuiRect_Ctor(u8 *rect);

// 0x659d43 (thiscall (left, top, right, bottom: words), pops 16; name unconfirmed): as GuiRect_Ctor with the four words given.
u8 *__fastcall GuiRect_CtorWith(u8 *rect, void *, u32 left, u32 top, u32 right, u32 bottom);

// 0x659d90 (thiscall; name unconfirmed): the vtable 0x705920, then the base destructor (Fn_66bff7), whose EAX is left.
u32 __fastcall GuiRect_Dtor(u8 *rect);

// 0x665eb0 (thiscall (flags), pops 4; name unconfirmed): the vector deleting destructor: with flag 2 the array (the count dword before
// it) destroyed last first (CRT_VectorDtorIterator, 12 bytes each, GuiRect_Dtor) and its block (from the count) freed (Mem_Free); else
// the one rectangle destroyed and, with flag 1, freed. Returns the rectangle.
u8 *__fastcall GuiRect_DeletingDtor(u8 *rect, void *, u32 flags);

// 0x659e83 (thiscall (x, y: words), pops 8; slot 0; name unconfirmed): 1 in AX when left <= x < right and top <= y < bottom (signed), else
// 0. Only AX is set.
u32 __fastcall GuiRect_Contains(u8 *rect, void *, u32 x, u32 y);

// 0x659ee2 (thiscall (x, y: word pointers, each may be 0), pops 8; slot 1; name unconfirmed): the centre, left + (right - left) / 2
// (an arithmetic shift) and likewise top / bottom, stored through each pointer given.
void __fastcall GuiRect_Centre(u8 *rect, void *, u16 *x, u16 *y);

// 0x65a3e9 (thiscall (dx, dy: words), pops 8; slot 2; name unconfirmed): moves the rectangle: dx added to left and right, dy to top and
// bottom.
void __fastcall GuiRect_Offset(u8 *rect, void *, u32 dx, u32 dy);

// 0x65a43e (thiscall (dl, dt, dr, db: words), pops 16; slot 3; name unconfirmed): adds each to left, top, right and bottom.
void __fastcall GuiRect_Adjust(u8 *rect, void *, u32 dl, u32 dt, u32 dr, u32 db);

// 0x65aa20 (thiscall (source), pops 4; name unconfirmed): the assignment: the base's (Fn_65aa80, which does nothing), then the four words
// copied from the source; returns the rectangle.
u8 *__fastcall GuiRect_Assign(u8 *rect, void *, const u8 *source);

// 0x661770 (thiscall (source), pops 4; name unconfirmed; called by 0x65cdbd, 0x65e767, 0x65e8f4, 0x66becf): the copy constructor: the
// base's (Fn_6617d0), the four words copied from the source, the vtable 0x705920; returns the rectangle.
u8 *__fastcall GuiRect_CopyCtor(u8 *rect, void *, const u8 *source);

// 0x65a493 / 0x65a4f5 (thiscall; the second (source: a rectangle, may be 0), pops 4; names unconfirmed; the second called by 0x65a61f,
// 0x65a67a, 0x65a752): a 0x10-byte node of a GUI rectangle at +4 (GuiRect_Ctor) and a dword +0 = 0 (a link?): the first sets the four
// words 0 again (Fn_659dac), the second assigns the source to the rectangle when given (GuiRect_Assign). Return the node.
u8 *__fastcall GuiRectNode_Ctor(u8 *node);
u8 *__fastcall GuiRectNode_CtorFrom(u8 *node, void *, const u8 *source);

// 0x65a55b / 0x65aa90 (thiscall; the second (flags), pops 4; names unconfirmed; each calls the other): the node's destructor deletes
// the rest of its list first (the next node +0's deleting destructor with 1), then GuiRect_Dtor of its rectangle (whose EAX is left);
// the deleting one destroys the node and frees it (Mem_Free) with flags bit 0; returns the node.
u32 __fastcall GuiRectNode_Dtor(u8 *node);
u8 *__fastcall GuiRectNode_DeletingDtor(u8 *node, void *, u32 flags);

// 0x65a5d2 / 0x65a6b0 / 0x65a7e3 (thiscall; (rect), pops 4 / (left, top, right, bottom: words), pops 16 / (node), pops 4; names
// unconfirmed; the first called by 0x65a958, the last by 0x65a98d): the list's head +0 (the node holding it: its +0 is the first node,
// or 0) is given a last node: Append a new one (Mem_Alloc 0x10, GuiRectNode_CtorFrom; 0 when the allocation fails) of a copy of the
// rectangle; AppendRect one of a new rectangle of the four words (Mem_Alloc 0xc, GuiRect_CtorWith; the node made of no rectangle when
// that fails), the rectangle allocated before the node and never freed (sic); AppendNode the node given. The last node is the head's
// +0 when that is 0, else the node of the chain from it whose +0 is 0.
void __fastcall GuiRectNode_Append(u8 *list, void *, const u8 *rect);
void __fastcall GuiRectNode_AppendRect(u8 *list, void *, u32 left, u32 top, u32 right, u32 bottom);
void __fastcall GuiRectNode_AppendNode(u8 *list, void *, u8 *node);

// 0x65a824 (thiscall (shape, out), pops 8; recursive, no other caller; name unconfirmed): the first of the list's GuiRect_Overlap of
// its rectangle +4 with the shape that is not 0 (in AX; 0 at the end of the list). (The original then stores the rectangle in its own
// argument out, which is lost: out is never written.)
u32 __fastcall GuiRectNode_Overlap(u8 *node, void *, u8 *shape, u32 out);

// 0x65a887 (thiscall (shape, may be 0), pops 4; recursive, no other caller; name unconfirmed): with a shape, each node's rectangle
// cut by it (GuiRect_Split into two local rectangles, while it splits): the rectangle becomes the second part and the first part goes
// to a new list (the first a new node of it as GuiRectNode_Append makes them, then GuiRectNode_Append; an allocation that fails leaves
// the list empty for the next); then the rest of the list (from +0) done the same, its list appended (GuiRectNode_AppendNode) or taken.
// Returns the new list (0 when none).
u8 *__fastcall GuiRectNode_Subtract(u8 *node, void *, u8 *shape);

// 0x659f3d (thiscall (shape), pops 4; name unconfirmed): how the shape (its words +4 / +6 / +8 / +0xa read as a rectangle's, whatever it
// is) lies against the rectangle, in AX: 0x200 when they are the same (the shape's bottom compared with the top, sic); 0x100 when the
// shape contains (its slot 0) the points (left + 1, top + 1) and (right - 1, bottom - 1); else the rectangle's own slot 0 tells which of
// the shape's corners it contains (top left 0x51, top right 0x92, bottom right 0xa8, bottom left 0x64, or-ed), returned when any of the
// bits 0xf is set; else the shape's edges crossing it: with the shape spanning it across (left <= left, right >= right), its top (0x10)
// and bottom (0x20) strictly between the top and bottom; spanning it down, its left (0x40) and right (0x80) strictly between the sides.
u32 __fastcall GuiRect_Overlap(u8 *rect, void *, u8 *shape);

// 0x65a1c3 (thiscall (shape, first, second: rectangles, each may be 0), pops 12; name unconfirmed): the rectangle split by the shape's
// edge that crosses it (GuiRect_Overlap): its top (0x10) or bottom (0x20) into the parts above and below it, its left (0x40) or right
// (0x80) into the parts left and right of it, the first / second given set (Fn_659dac) to (0x10) above / below, (0x20) below / above,
// (0x40) left / right, (0x80) right / left; then 1 in AX. 0 in AX with no shape, when the shape contains it (0x100), or otherwise (a
// corner inside, or apart): then the first given is set to the rectangle itself. Only AX is set.
u32 __fastcall GuiRect_Split(u8 *rect, void *, u8 *shape, u8 *first, u8 *second);

// 0x659e3e (thiscall (width, height: word pointers, each may be 0), pops 8; name unconfirmed): right - left and bottom - top stored through
// each pointer given.
void __fastcall GuiRect_GetSize(u8 *rect, void *, u16 *width, u16 *height);

// 0x659de5 (thiscall (left, top, right, bottom: word pointers, each may be 0), pops 16; called by 0x47de65, 0x47de84; name
// unconfirmed): the four words stored through each pointer given.
void __fastcall GuiRect_GetEdges(u8 *rect, void *, u16 *left, u16 *top, u16 *right, u16 *bottom);

// 0x661700 (thiscall (other), pops 4; name unconfirmed): 1 when the four words equal the other's, else 0.
u32 __fastcall GuiRect_Equals(const u8 *rect, void *, const u8 *other);
}
