// The 0x24-byte element of counted strings whose clears and deleters are in buffers.h (CountedPair_*): a narrow std::string at +0, a wide one at
// +0x10 and a dword at +0x20 (what it holds is not known yet; the code at 0x4ea5xx-0x4ebaxx builds them and hands them to container inserts).
// Reimplementation of its constructor 0x4ea740.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"
#include "stdwstring.h"

struct CountedPair {
    StdString narrow;  // +0
    StdWString wide;   // +0x10
    u32 value;         // +0x20
};
static_assert(sizeof(CountedPair) == 0x24, "two strings and a dword");

extern "C" {

// 0x4ea740 (thiscall; a std::wstring by value, pops 16; name unconfirmed): narrow empty and wide assigned the string (StdWString_AssignSub from 0,
// npos), each taking the string's allocator byte; value 0. The string is then destroyed: its block's count byte lowered, or the block freed
// 2 bytes before the characters when the count is 0 or 0xff. Returns the pair.
CountedPair *__fastcall CountedPair_CtorWide(CountedPair *p, void *edx, StdWString s);
}
