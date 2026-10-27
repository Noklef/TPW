// The online chat: the singleton at 0x7cddec (0xf0c bytes, made by 0x46cf10, constructed by 0x4e5260; "ONLINE CHAT: ..."). Its critical
// section is at +0x48. Reimplementation of 0x4e5260, 0x4e6130 and 0x4e6150 (thiscall). The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e5260 (thiscall): the constructor. +0 and +4 = 0; the wide string +8 and the string +0x18 empty; the bytes +0x28-+0x2c = 0; critical
// sections at +0x30 and +0x48 (CritSec_Ctor); the string maps record at +0xe60 (StringMapsRecord_Ctor); +0xec0 (byte), +0xec4, +0xecc-+0xed8
// = 0; Init_5ec210 on +0xedc; +0xeec-+0xef4 and the byte +0xef8 = 0; the wide string +0xefc empty. The four strings' allocator bytes
// (+8, +0x18, +0xec8, +0xefc) are one uninitialised stack byte of the original's (sic; 0 here). Then +8 = rand() as swprintf's L"%08x"
// (0x75c0f0), and +0x18 = the global string at 0x7a1500 (assign(x, 0, npos) inlined: shared when all of it and its count is below
// 0xfe, else copied; the self-assignment arm, never taken, erases twice through 0x4f1400). Returns the chat.
u8 *__fastcall OnlineChat_Ctor(u8 *chat);

// 0x4e6130: under the lock (+0x48, CritSec_Enter / CritSec_Leave) the bytes +0x29 and +0x2a set to 1.
void __fastcall OnlineChat_Set29And2a(u8 *chat);

// 0x4e6150: the byte +0x2a, read under the lock (in AL only).
u8 __fastcall OnlineChat_Get2a(u8 *chat);
}
