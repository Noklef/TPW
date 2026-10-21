// The game's doubly linked pointer list: reimplementation of its append, 0x5e42d0.
//
// A list is {vtable, head, tail, count}; each node {item, next, prev} is a Mem_Alloc'd 12-byte
// block. The derived lists (the WAD directory's folder and file lists among them) differ only
// in their destructors; most of their code is inline in its users.
#pragma once

#include "../../../re/types.h"

struct PtrListNode {
    void *item;
    PtrListNode *next;
    PtrListNode *prev;
};
struct PtrList {
    const void *vtable;
    PtrListNode *head;
    PtrListNode *tail;
    u32 count;
};
static_assert(sizeof(PtrList) == 0x10, "PtrList is 0x10 bytes");

extern "C" {
// __thiscall (item): a node for item at the tail. A failed allocation is not checked (the
// original writes through the null).
void __fastcall PtrList_Append(PtrList *l, void *edx, void *item);
}

struct PtrListEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
};
// The list's allocation goes through env (nullptr: the game's).
void PtrList_SetEnv(const PtrListEnv *env);

extern "C" {

// 0x5ce910 (thiscall; out, unused, pops 8): a list cursor (a node pointer at +0) moves to the node's next (its first dword); the
// node it left is stored at *out. Returns out.
u32 *__fastcall ListCursor_Next(u32 *cursor, void *, u32 *out, u32 unused);

// 0x5e2a80 (thiscall on the list {+4 first, +8 last, +0xc count} of {value, next at +4, previous at +8} nodes; value, pointer to the iterator, pops 8): a new node (zeroed,
// 12 bytes) holding the value goes before the node the iterator is (at the end for none; at the front for the first node) and the count grows.
void __fastcall PtrList_Insert(u8 *list, void *, u32 value, u8 **position);

// 0x56b5c0 (thiscall; item, pops 4): the item is looked for in the array of pointers at +8 (count at +0x1c) from the back; when found it is
// freed, the count is one less, the slot gets the last element and the dword at +4 of what +4 points to = the new count. Returns 1 when
// found, otherwise 0.
u32 __fastcall PtrArray_RemoveAndFree(u8 *object, void *, u32 item);

// 0x56b4c0 (thiscall; image, pops 4; name unconfirmed): a new sprite frame (0x44 bytes, the game's heap, SpriteFrame_Init) with +0x24 -1, flags
// (+0x10) 0x58000, +0x2c 10.0f and the image (+0x28) appended to the same array (the decoded textures' 0x879ce0, 0x5f3f4c); the count is stored at
// +0x1c and at +4 of what +4 points to. Returns the frame; 0 when the allocation failed (the error (2, 0xb0754000) pushed).
u8 *__fastcall PtrArray_AddSpriteFrame(u8 *object, void *, u32 image);

// 0x5d8a80: removes the first node with a value from the linked list at +0xc (unlinks it, counts it off, frees it).
void __fastcall ListRemovec_5d8a80(u8 *o, void *, u32 v0);

// 0x5da180: removes the first node with a value from the linked list at +0x58 (unlinks it, counts it off, frees it).
void __fastcall ListRemove58_5da180(u8 *o, void *, u32 v0);

// 0x5da450: removes the first node with a value from the linked list at +0x4 (unlinks it, counts it off, frees it).
void __fastcall ListRemove4_5da450(u8 *o, void *, u32 v0);

// 0x5dd4b0: removes the first node with a value from the linked list at +0x6c (unlinks it, counts it off, frees it).
void __fastcall ListRemove6c_5dd4b0(u8 *o, void *, u32 v0);

// 0x5e15c0: removes the first node with a value from the linked list at +0x8 (unlinks it, counts it off, frees it).
void __fastcall ListRemove8_5e15c0(u8 *o, void *, u32 v0);

// 0x5e4250: removes the first node with a value from the linked list at +0x18 (unlinks it, counts it off, frees it).
void __fastcall ListRemove18_5e4250(u8 *o, void *, u32 v0);

// 0x5f42f0: the same for the list at +0x20080 (the park map's sprites: its vtable 0x703078's second slot).
void __fastcall ListRemove20080_5f42f0(u8 *o, void *, u32 v0);

// 0x5e1540 (thiscall on the holder; item, pops 4): the pointer list at +8 (head at +4, tail at +8, count at +0xc) gets the item in order of the signed dword at +0x24 of the items:
// first (List_PushHead) when the list is empty, otherwise before the first item whose key is not below the new one's (PtrList_Insert), else at the end (PtrList_Append). The holder keeps
// the item at +0x10.
void __fastcall SortedPtrList_Add(u8 *holder, void *, u8 *item);

// 0x56b450 (thiscall on the same array object; image, pops 4; name unconfirmed): the image is looked for in the array of sprite images at +0xc
// (count at +0x18) from the back; when found it is released (SpriteImage_Release, even a null one), destroyed (SpriteImage_Dtor) and freed
// when not null, the count is one less and the slot gets the last element. Returns 1 when found, otherwise 0.
u32 __fastcall PtrArray_RemoveImage(u8 *object, void *, u8 *image);

// 0x56b210 (thiscall on the same array object; name unconfirmed): with bit 0 of +0 set, it is cleared and, when there is a renderer
// (0x87a6d0), the part (the object at +4 at its virtual base offset, or 0) leaves its tag 0x60 list (Renderer_RemoveFromSlot); with images
// (+0xc) each of the +0x18 is released and, when not null, destroyed and freed, the array freed (TrackedGlobal_Free) and +0xc / +0x18
// cleared; with an object at +4 each of the +0x1c frames at +8 is freed, the object deleted through its virtual base (slot 0, flags 1) and
// +4 / +0x1c cleared; +8 and +0x10 are cleared in any case. Returns 1.
u32 __fastcall PtrArray_ReleaseAll(u8 *object, void *);
// 0x56b0c0 (thiscall on the same array object; frames, images, pops 8; name unconfirmed): with a capacity already (+0x10) the error (2,
// 0xb0754015) is pushed onto the engine's list and 0 returned. Otherwise a sprite set (Mem_Alloc 0x24, built inline as RendererPart_Ctor's,
// its virtual base added to the live objects) goes to +4 (none: the error (2, 0xb0754000), 0); the set's +4 cleared, +8 = its tracked
// array of `frames` dwords (TrackedArray_Alloc), +0xc = a tracked block of `images` (0: `frames`) dwords (TrackedGlobal_Alloc); either
// failing releases everything (PtrArray_ReleaseAll) and gives 0. Then +0x10 = frames, +0x14 = images and 1.
u32 __fastcall PtrArray_Init(u8 *object, void *, u32 frames, u32 images);

// 0x56b570 (thiscall; key, a, b, c, pops 16): the array of pointers at +8 (count +0x1c) searched
// backwards for the key; when found the key's dwords at +0x14, +0x18, +0x1c = a, b, c and 1 is
// returned, else 0.
u32 __fastcall PtrArray_SetVectorOf(const u8 *list, void *, u8 *key, u32 a, u32 b, u32 c);

// 0x6c49b0 (thiscall on a pointer list: head +4, tail +8, count +0xc, nodes of next +4 / previous +8; item, pops 4; name unconfirmed): a
// copy of the item's 16 bytes (Mem_Alloc 0x10; none: 0) goes in before the first node whose record's float at +0xc the item's is above
// (fcompp: not past NaNs): at the head through List_PushHead, else in a new node (Mem_Alloc 0xc, Init_6c4fd0) linked in; none: a new
// node at the tail. The count is raised; returns the new node (the head's: the found node's previous). A failed node allocation is
// written through anyway (sic).
u8 *__fastcall PtrList_InsertCopyByKey(u8 *list, void *, const u8 *item);
}  // extern "C"
