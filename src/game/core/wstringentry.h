// An entry of two wide strings and a dword (0x24 bytes), built by value from a std::wstring and handed to a container insert by the code at
// 0x4ea5xx-0x4ebaxx (what it holds is not known yet). Reimplementation of its constructor 0x4ea740.
#pragma once

#include "../../../re/types.h"
#include "stdwstring.h"

struct WStringEntry {
    StdWString first;   // +0: left empty
    StdWString second;  // +0x10: the constructor's string
    u32 value;          // +0x20: 0
};
static_assert(sizeof(WStringEntry) == 0x24, "two strings and a dword");

extern "C" {

// 0x4ea740 (thiscall; a std::wstring by value, pops 16; name unconfirmed): first empty and second assigned the string (StdWString_AssignSub from 0,
// npos), each taking the string's allocator byte; value 0. The string is then destroyed: its block's count byte lowered, or the block freed
// 2 bytes before the characters when the count is 0 or 0xff. Returns the entry.
WStringEntry *__fastcall WStringEntry_Ctor(WStringEntry *e, void *edx, StdWString s);
}
