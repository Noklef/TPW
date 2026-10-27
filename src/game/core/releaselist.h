// A locked list of COM objects waiting to be released (the class name is invented): one instance at 0xfb2730, made on first use (its flag
// 0xfb2728 bit 0) and destroyed at exit; the sound objects' slots (soundslot.h) hand it the objects they drop. Nodes of 0xc bytes
// (value, next, previous) pushed at the head under a critical section. Reimplementation of 0x6d7370-0x6d75b7; the names are invented.
#pragma once

#include "../../../re/types.h"

struct ReleaseNode {
    u32 value;          // +0
    ReleaseNode *next;  // +4
    ReleaseNode *prev;  // +8
};

struct ReleaseList {    // 0xfb2730
    u32 vtable;         // 0x711c20 (the base list's: 0x711c1c)
    ReleaseNode *head;  // +4
    ReleaseNode *tail;  // +8
    u32 count;          // +0xc
    u8 lock[0x18];      // +0x10, a CRITICAL_SECTION
};
static_assert(sizeof(ReleaseNode) == 0xc, "ReleaseNode is 0xc bytes");
static_assert(sizeof(ReleaseList) == 0x28, "ReleaseList is 0x28 bytes");

struct ReleaseListEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());  // crt_atexit (0x67a7d0)
};
// The instance's atexit goes to env (nullptr: the game's).
void ReleaseList_SetEnv(const ReleaseListEnv *env);

extern "C" {

// 0x6d7370 (thiscall): the nodes popped from the head and freed while the count lasts, the lock deleted, then the base list's
// destructor (inlined: as 0x6d7420).
void __fastcall ReleaseList_Dtor(ReleaseList *list);
// 0x6d7420 (thiscall): the base list's destructor: its vtable 0x711c1c, the chain from the head freed, all cleared.
void __fastcall ReleaseList_BaseDtor(ReleaseList *list);
// 0x6d7460 (cdecl): the instance at 0xfb2730, constructed first (and its destructor 0x6d74a0 registered with atexit) unless 0xfb2728 bit 0
// says it was.
ReleaseList *__cdecl ReleaseList_Get();
// 0x6d74a0 (cdecl): the destructor on the instance (what ReleaseList_Get registers).
void __cdecl ReleaseList_AtExit();
// 0x6d74b0 (thiscall): an empty list, vtable 0x711c20, its lock initialised. Returns the list.
ReleaseList *__fastcall ReleaseList_Ctor(ReleaseList *list);
// 0x6d7500 (thiscall flags; pops 4): the base list's destructor, then the list freed with flags bit 0. Returns the list.
ReleaseList *__fastcall ReleaseList_BaseDeletingDtor(ReleaseList *list, void *, u32 flags);
// 0x6d7520 (thiscall value; pops 4): under the lock, a new node of value pushed at the head (a failed allocation writes through null,
// as the original). Returns what LeaveCriticalSection does.
u32 __fastcall ReleaseList_Push(ReleaseList *list, void *, u32 value);

// 0x6d7690 (thiscall flags; pops 4; slot 0 of the base's vtable 0x711c1c; name invented): as ReleaseList_BaseDeletingDtor, the base's
// destructor inlined. Returns the list.
ReleaseList *__fastcall ReleaseListBase_DeletingDtor(ReleaseList *list, void *, u32 flags);

}  // extern "C"
