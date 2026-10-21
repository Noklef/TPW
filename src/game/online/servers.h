// The online servers' settings: reimplementation of 0x45aee0-0x45b1e3.
//
// The object at 0x7a14f0 (built by 0x5d2a80) holds the server names as MSVC6 std::strings
// (core/stdstring.h): mail +0, chat +0x10, login +0x20, city +0x30, news +0x40, vote +0x50;
// their ports as u16s from +0x60 in the same order; the complaints address +0x6c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d2a80 (thiscall; name unconfirmed): the settings' defaults, as the static initialiser 0x45ad60 builds the object at 0x7a14f0:
// mail "gatekeeper.ea.com" (0x77485c), chat "britain.bullfrog.ea.com" (0x774844), login, city and news "chorlton" (0x774838), vote
// empty, the ports 25, 6002, 7598, 7591 and 7597 (+0x60..+0x68) and the complaints address 0x77481c. Each string is emptied
// (_Tidy(false)) and, when _Grow(length, true) gives it room, the characters copied in; the first three end themselves inline, the others
// through _Eos. Their allocator bytes are one uninitialised stack byte in the original (0 here: the bench leaves them out). Returns the
// object.
u8 *__fastcall OnlineServers_Ctor(u8 *servers);

// 0x45ad90 (thiscall; name unconfirmed): the destructor (the static's, given to atexit): the complaints address released (_Tidy(true)),
// then vote, news, city, login, chat and mail the same way inline.
void __fastcall OnlineServers_Dtor(u8 *servers);

// 0x45aee0: copies them from Online.sam (gBalanceOnline 0x785828, once it has loaded), each
// string through a temporary built from the C string and assigned whole.
void __cdecl Online_ApplyBalance();
}
