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

// 0x5d0330 (thiscall; out, before, x, pops 12): the same in front of the node before; *out = the new node.
void __fastcall StdListString_Insert(u8 *list, void *, u8 **out, u8 *before, const StdString *x);

// 0x618160 (thiscall; value pointer, pops 4): a node holding *value is linked in front of the sentinel of a list of dwords, the count raised.
void __fastcall StdListDword_PushBack(u8 *list, void *, const u32 *value);

}  // extern "C"
