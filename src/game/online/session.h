// The online session: the singleton at 0xf7d88c (made by 0x48b390, 0x404 bytes, constructed by
// 0x5b50b0) that runs the login, news, city and upload connections ("ONLINE_LOGIN: ...").
// Reimplementation of its getters 0x5b6450-0x5b64c0 (thiscall), its connections' release and reset (0x5b64d0, 0x5b6b20, 0x5b6980,
// 0x5b83e0, 0x5b8400, 0x5b9040), the login's answer (0x5b62e0) and the Auth's deleting destructor (0x5b7b80).
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

// The connections are objects of the WEA*R DLLs, whose methods are imports (__thiscall). Each release says so on the debugger's output
// (OutputDebugStringA, "ONLINE ...: Releasing connection..."). Names unconfirmed.

// 0x5b64d0: with an Auth (+0): Auth::Release (0x6fd45c); when still set ~Auth (0x6fd460) and freed (Mem_Free); +0 cleared; "ONLINE LOGIN" said.
void __fastcall Online_ReleaseLogin(u8 *session);

// 0x5b6b20: the same for the News at +0x6c (Release 0x6fd58c, ~News 0x6fd588; "ONLINE NEWS").
void __fastcall Online_ReleaseNews(u8 *session);

// 0x5b6980: the News released as Online_ReleaseNews does; then +0x74, +0x90 and +0xac = -1, +0xc8, +0xcc, +0x78, +0x94, +0xb0 and +0xd0
// = 0, and the strings at +0x7c (""), +0x98 and +0xb4 ("", 0x786de4 / 0x78a374) assigned through their vtable's +0x14.
void __fastcall Online_ResetNews(u8 *session);

// 0x5b83e0: with an Upload (+0x104): Upload::Release (0x6fd5ec) and +0x104 cleared (not freed).
void __fastcall Online_ReleaseUpload(u8 *session);

// 0x5b8400: the Upload released so; +0x320..+0x32c, +0x344 and +0x370..+0x380 = 0, the strings at +0x348 and +0x35c assigned "" (0x78a374).
void __fastcall Online_ResetUpload(u8 *session);

// 0x5b9040: "ONLINE FTPDOWNLOAD: Reseting" said; the object at +0x3a0 (when set: +0x3a0 cleared first) deleted (vtable +0 with 1), the
// block at +0x39c (when set) freed and cleared, +0x384..+0x398, +0x3a8, +0x3c0, +0x3c4 and +0x3f0 = 0, the string at +0x3dc assigned "".
void __fastcall Online_ResetDownload(u8 *session);

// 0x5b7b80 (thiscall; flags; pops 4): the Auth's deleting destructor: ~Auth (0x6fd460), freed with flag 1. Returns the Auth.
u8 *__fastcall Auth_DeletingDtor(u8 *auth, void *, u32 flags);

// 0x5b62e0 (thiscall; status, two unused, record; pops 16): the login's answer. 60 (OK): "Authorization OK" said; +0x34 / +0x38 = the
// record's +0 / +4, the name object at +0x3c assigned the record's +0x30 (vtable +0x14), +0x50..+0x5c = the record's +0x58..+0x64, the
// record's count (+0x68) decremented and the record freed at 0; +0x68 = 0 and +0x60 (logged in) and +0x64 = 1. Else +0x68 = the error:
// 0x4e, 0x4d, 0x4c, 0x4b for -6, -5, -4, -2, 0x37 for anything else; +0x60 = 0, +0x64 = 1.
void __fastcall Online_LoginResult(u8 *session, void *, s32 status, u32, u32, u8 *record);
}
