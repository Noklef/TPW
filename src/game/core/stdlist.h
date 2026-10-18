// The out-of-line members of the game's MSVC6 list<string> (nodes of 0x18 bytes: next, previous, the string at +8; the list object holds the sentinel node at +4 and the count
// at +8) and of a list of dwords (nodes of 0xc bytes: next, previous, value): reimplementation of 0x5cde80, 0x5cdf00, 0x5d0330 and 0x618160.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"

extern "C" {

// 0x5cde80 (thiscall): every node is unlinked, its string tidied (StdString_Tidy with built) and the node freed, the count lowered; then the sentinel is freed and the sentinel
// pointer and the count cleared.
void __fastcall StdListString_Clear(u8 *list);

// 0x5cdf00 (thiscall; x, pops 4): a node holding a copy of x (the allocator byte, then assign(x, 0, npos)) is linked in front of the sentinel, the count raised.
void __fastcall StdListString_PushBack(u8 *list, void *, const StdString *x);

// 0x4a8ad0 (thiscall on the owner of a list of std::string at +0x10 (sentinel +0x14, count +0x18); chars, pops 4; name unconfirmed): a std::string
// made of the characters (_Grow(strlen, true); its allocator byte the low byte of the chars pointer, sic) is copied (StdString_CopyCtor) into a node linked in
// front of the sentinel, the count raised; then released.
void __fastcall TokenList_PushChars(u8 *owner, void *, const char *chars);
// 0x5ca220 (thiscall; chars, pops 4; name unconfirmed): the same for an owner's list of std::string at +0x30 (sentinel +0x34, count +0x38).
void __fastcall StringList34_PushChars(u8 *owner, void *, const char *chars);

// 0x5d0330 (thiscall; out, before, x, pops 12): the same in front of the node before; *out = the new node.
void __fastcall StdListString_Insert(u8 *list, void *, u8 **out, u8 *before, const StdString *x);

// 0x618160 (thiscall; value pointer, pops 4): a node holding *value is linked in front of the sentinel of a list of dwords, the count raised.
void __fastcall StdListDword_PushBack(u8 *list, void *, const u32 *value);

// 0x5cc700 / 0x5cc830 (thiscall): the nodes of the circular list whose sentinel is at +0x68 / +0x74 (count at +0x6c / +0x78) whose dword at +8
// equals the dword at +8 of the record the dword at +0x7c points to are unlinked and freed, the count one less each.
void __fastcall List68_EraseMatching(u8 *object);

void __fastcall List74_EraseMatching(u8 *object);

// 0x5cdf70 (thiscall; out, node, pops 8): the node is unlinked from its circular list, the counted buffer (see core/buffers) at its +0xc released
// and its fields +0xc, +0x10 and +0x14 cleared, the node freed, the count at +8 one less, and the node that followed it stored through out.
// Returns out.
u32 *__fastcall List_EraseNodeCounted(u8 *object, void *, u32 *out, u8 *node);

// 0x4f3400 (thiscall; out, first, last, pops 12): like List_EraseRange (core/stdlist) for a list whose nodes hold a counted buffer (see core/buffers) at +0xc
// (released like CountedEntries_Release does, the block freed 2 bytes below the pointer) and the fields +0xc, +0x10 and +0x14 are cleared before
// the node is freed. Returns out.
u32 *__fastcall List_EraseRangeCounted(u8 *object, void *, u32 *out, u8 *first, u8 *last);

// 0x4f54e0 (thiscall; out, node, pops 8): like List_EraseNodeCounted (core/stdlist) but the counted buffer's block is freed 2 bytes below the pointer.
u32 *__fastcall List_EraseNodeCounted2(u8 *object, void *, u32 *out, u8 *node);

// 0x5cdfe0 (thiscall; out, first, last, pops 12): like List_EraseRangeCounted (core/stdlist) but the counted buffer is freed 1 byte below the pointer
// (the count is the block's first byte). Returns out.
u32 *__fastcall List_EraseRangeCounted1(u8 *object, void *, u32 *out, u8 *first, u8 *last);

// 0x5cd660 (thiscall; index, pops 4): the index-th node of the circular list at +4 (count at +8) is unlinked, its counted buffer (the block freed 1 byte
// below the pointer) released and its fields +0xc, +0x10 and +0x14 cleared, and the node freed. Returns 1, or 0 when the list is shorter (AL).
u8 __fastcall List_EraseNthCounted(u8 *object, void *, u32 index);

// 0x4d3850 (thiscall; item, pops 4): List_PushHead (core/stdlist) of the same object.
u32 __fastcall List_PushHeadForward(u8 *list, void *, u32 item);

// 0x5b3110 (stdcall; a, b, pops 8): a new 0x14-byte node with a at +4 and b at +0x10 (the rest as the allocator left it).
u8 *__stdcall Node14_New(u32 a, u32 b);

// 0x618130 (thiscall; from, pops 4): +0 = the byte at from; a new 0xc-byte sentinel node pointing at itself twice is allocated, +4 = it, +8 = 0. Returns the object.
u8 *__fastcall Sentinel_Ctor(u8 *object, void *, const u8 *from);

// 0x5f46a0 (thiscall on a list: +4 head, +8 tail, +0xc count; nodes {object, next, prev}): the head node is unlinked and freed. Returns the object it held (0 when the list was
// empty).
u8 *__fastcall List_PopHeadObject(u8 *list);

// 0x5d0510 (thiscall on a list: +8 count; node a, list x, node c, pops 12): when c is neither a nor the node before a, the node c moves from its place (in list x) to in front
// of a (circular lists: +0 next, +4 previous); the count of this list rises, that of x falls.
void __fastcall List_SpliceOne(u8 *list, void *, u8 *a, u8 *x, u8 *c);

// 0x5ce080 (thiscall): removes every element equal to the one before it from the list of {+0xc characters (null: ""), +0x10 length, +0x14 capacity} strings: the node leaves
// the list, its counted string (the count byte before the characters: released, the block freed at 0 or 0xff) is let go, its fields cleared, the node freed and the size
// decremented. Equal means the same length and bytes.
void __fastcall StdListString_Unique(u8 *list);

// 0x5d0590 (thiscall; other list, pops 4): merges the other sorted list into this one (an element of the other goes before the first of this that it is below, unsigned);
// what is left of the other goes to the end; the size is the sum and the other's is 0. Nothing for the list itself.
void __fastcall StdListDword_Merge(u8 *list, void *, u8 *other);

// 0x5b2fd0 (stdcall; a, b): a new block of 0x28 bytes (otherwise left as it is) with a at +4 and b at +0x24. Returns it.
u8 *__stdcall Node28_New(u32 a, u32 b);

// 0x5b1680 (stdcall; a, b): a new block of 0x18 bytes (otherwise left as it is) with a at +0 and b at +4, each the block itself when 0.
// Returns it.
u8 *__stdcall Node18_New(u32 a, u32 b);

// 0x5cd400 (thiscall): the byte at +0 = the top byte of the object pointer (sic: the original reads the byte above the saved ECX on the
// stack), a new circular head node (0x18 bytes, +0 and +4 pointing to itself) at +4, the count at +8 = 0 and its first dword at +0xc.
// Returns the object.
u8 *__fastcall ListHead_Ctor(u8 *object);

// 0x5ce540 (thiscall; out, first, last, pops 12): erases the nodes of the circular list from first up to last (each unlinked and freed, the
// count at +8 one less each), stores last-reached first (always last) through out. Returns out.
u32 *__fastcall List_EraseRange(u8 *object, void *, u32 *out, u8 *first, u8 *last);

// 0x5cde50 (thiscall; source, pops 4): like ListHead_Ctor but the byte at +0 is copied from the byte the source points to and the first
// dword at +0xc is not set. Returns the object.
u8 *__fastcall ListHead_CtorFrom(u8 *object, void *, const u8 *source);

// 0x5d0690 (stdcall; a, b): a new block of 12 bytes (otherwise left as it is) with a at +0 and b at +4, each the block itself when 0. Returns it.
u8 *__stdcall Node0c_New(u32 a, u32 b);

// 0x5e2a30 (thiscall; item, pops 4): a new node of 12 bytes (item, 0, 0) is made the head of the list ({?, head +4, tail +8, count +0xc}; the
// node's +4 is the old head, the old head's +8 the node, the tail the node too for an empty list) and the count rises. Returns the node
// for an empty list, otherwise the new count.
u32 __fastcall List_PushHead(u8 *list, void *, u32 item);

// 0x5caf20 (thiscall; name unconfirmed; run only by exception unwinds, 0x6e09dc / 0x6e0a3e): a record of three std::strings (+0, +0x10,
// +0x2c) and a list of 0x18-byte nodes with counted buffers (+0x20: head +0x24, count +0x28) destroyed: the string at +0x2c tidied (inlined
// StdString_Tidy(true)), the list erased node by node (List_EraseNodeCounted), its sentinel freed and its head and count cleared, then the
// strings at +0x10 and +0 tidied.
void __fastcall StringsListRecord_Dtor(u8 *record);

// 0x5af0e0 (thiscall; name unconfirmed; run only by exception unwinds, 0x6df82b / 0x6df883): two lists of 0x18-byte nodes {next, prev, a
// counted buffer at +8} ({?, head +4, count +8}) destroyed: the one at +0xc erased node by node (List_EraseNodeCounted2), the one at +0 by
// hand (unlinked, the buffer released with CountedBuf_Deleter (core/buffers), freed, its count lowered); each sentinel freed and its head and
// count cleared.
void __fastcall CountedListPair_Dtor(u8 *pair);

// 0x5d03b0 (thiscall; other, pops 4): the dwords at +4 and +8 swapped with the other object's (a list's swap: its head node and size).
void __fastcall Obj_SwapPair(u8 *object, void *, u8 *other);
}  // extern "C"
