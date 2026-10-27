// The online chat: the singleton at 0x7cddec (0xf0c bytes, made by 0x46cf10, constructed by 0x4e5260; "ONLINE CHAT: ..."). Its critical
// section is at +0x48, its queue of message vectors at +0xec8. Reimplementation of 0x4e4fc0 (stdcall), 0x4e5260, 0x4e6130 and 0x4e6150 (thiscall). The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e5260 (thiscall): the constructor. +0 and +4 = 0; the wide string +8 and the string +0x18 empty; the bytes +0x28-+0x2c = 0; critical
// sections at +0x30 and +0x48 (CritSec_Ctor); the string maps record at +0xe60 (StringMapsRecord_Ctor); +0xec0 (byte), +0xec4, +0xecc-+0xed8
// = 0; Init_5ec210 on +0xedc; +0xeec-+0xef4 and the byte +0xef8 = 0; the wide string +0xefc empty. The four allocator bytes (the strings'
// +8, +0x18, +0xefc and the queue's +0xec8) are one uninitialised stack byte of the original's (sic; 0 here). Then +8 = rand() as swprintf's L"%08x"
// (0x75c0f0), and +0x18 = the global string at 0x7a1500 (assign(x, 0, npos) inlined: shared when all of it and its count is below
// 0xfe, else copied; the self-assignment arm, never taken, erases twice through 0x4f1400). Returns the chat.
u8 *__fastcall OnlineChat_Ctor(u8 *chat);

// 0x4e4fc0 (stdcall, pops 16; the callback 0x4edf43 hands the import at 0x6fd5f8; only code read; name invented): a vector of two pairs
// (CountedPair: empty strings, values 0x2711 and code; CountedPairVector_InsertN at its end) is queued on the chat's deque of such vectors
// (+0xec8: allocator byte, begin and end iterators, map +0x24, count +0x2c; blocks of 0x1000 bytes, 16-byte entries): the chat (0x7cddec)
// made first when there is none (0xf0c bytes, OnlineChat_Ctor; none: the null chat used, sic); under the lock at +0x30, a new back block
// (Deque32_GrowBack) when the count is 0 or the end is at its block's end, the vector copied to the end (CountedPairVector_Construct), the
// count raised. The vector's pairs are then destroyed (CountedPair_Deleter) and its block freed. The vector's and the pairs' allocator bytes are
// code's low byte (the original's temporaries read code's stack slot, sic).
void __stdcall OnlineChat_QueueCallback(u32 code, u32, u32, u32);

// 0x4e6130: under the lock (+0x48, CritSec_Enter / CritSec_Leave) the bytes +0x29 and +0x2a set to 1.
void __fastcall OnlineChat_Set29And2a(u8 *chat);

// 0x4e6150: the byte +0x2a, read under the lock (in AL only).
u8 __fastcall OnlineChat_Get2a(u8 *chat);
}
