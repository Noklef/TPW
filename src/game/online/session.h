// The online session: the singleton at 0xf7d88c (made by 0x48b390, 0x404 bytes, constructed by
// 0x5b50b0) that runs the login, news, city and upload connections ("ONLINE_LOGIN: ...").
// Reimplementation of its getters 0x5b6450-0x5b64c0 (thiscall).
//
// On "Authorization OK" (0x5b62e0) the login copies the user's record in: +0x34/+0x38, a name
// object at +0x3c (its wide text at +0x44), +0x50-+0x5c, and sets +0x60 (logged in).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b6450: +0x60, set while logged in.
u32 __fastcall Online_LoggedIn(const u8 *session);

// 0x5b6460: 1 unless logged in (+0x60) with the user's flag 0x20 (+0x58) clear.
u32 __fastcall Online_Restricted(const u8 *session);

// 0x5b6480 / 0x5b6490: the C strings at +0x14 / +0x28.
const char *__fastcall Online_String14(const u8 *session);
const char *__fastcall Online_String28(const u8 *session);

// 0x5b64a0: the user's name (+0x44, wide).
const u16 *__fastcall Online_UserName(const u8 *session);

// 0x5b64b0 / 0x5b64c0: the user's flags (+0x58) / +0x5c.
u32 __fastcall Online_UserFlags(const u8 *session);
u32 __fastcall Online_User5c(const u8 *session);
}
