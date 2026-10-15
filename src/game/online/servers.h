// The online servers' settings: reimplementation of 0x45aee0-0x45b1e3.
//
// The object at 0x7a14f0 (built by 0x5d2a80) holds the server names as MSVC6 std::strings
// (core/stdstring.h): mail +0, chat +0x10, login +0x20, city +0x30, news +0x40, vote +0x50;
// their ports as u16s from +0x60 in the same order; the complaints address +0x6c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x45aee0: copies them from Online.sam (gBalanceOnline 0x785828, once it has loaded), each
// string through a temporary built from the C string and assigned whole.
void __cdecl Online_ApplyBalance();
}
