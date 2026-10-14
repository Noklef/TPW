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
