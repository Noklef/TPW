// The online chat: the singleton at 0x7cddec (0xf0c bytes, made by 0x46cf10, constructed by 0x4e5260; "ONLINE CHAT: ..."). Its critical
// section is at +0x48. Reimplementation of 0x4e6130 and 0x4e6150 (thiscall). The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e6130: under the lock (+0x48, CritSec_Enter / CritSec_Leave) the bytes +0x29 and +0x2a set to 1.
void __fastcall OnlineChat_Set29And2a(u8 *chat);

// 0x4e6150: the byte +0x2a, read under the lock (in AL only).
u8 __fastcall OnlineChat_Get2a(u8 *chat);
}
