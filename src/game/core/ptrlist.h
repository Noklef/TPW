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

// 0x5e1540 (thiscall on the holder; item, pops 4): the pointer list at +8 (head at +4, tail at +8, count at +0xc) gets the item in order of the signed dword at +0x24 of the items:
// first (List_PushHead) when the list is empty, otherwise before the first item whose key is not below the new one's (PtrList_Insert), else at the end (PtrList_Append). The holder keeps
// the item at +0x10.
void __fastcall SortedPtrList_Add(u8 *holder, void *, u8 *item);
}  // extern "C"
