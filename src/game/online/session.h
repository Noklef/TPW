// The online session: the singleton at 0xf7d88c (made by 0x48b390, 0x404 bytes, constructed by
// 0x5b50b0) that runs the login, news, city and upload connections ("ONLINE_LOGIN: ...").
// Reimplementation of its getters 0x5b6450-0x5b64c0 (thiscall), its connections' release and reset (0x5b64d0, 0x5b6b20, 0x5b6980,
// 0x5b83e0, 0x5b8400, 0x5b9040), the login's answer (0x5b62e0) and the Auth's deleting destructor (0x5b7b80).
//
// On "Authorization OK" (0x5b62e0) the login copies the user's record in: +0x34/+0x38, a name
// object at +0x3c (its wide text at +0x44), +0x50-+0x5c, and sets +0x60 (logged in).
#pragma once

#include "../../../re/types.h"

// Online_Instance's call: Online_Construct; the bench's recorder.
struct OnlineInstanceEnv {
    u8 *(__fastcall *ctor)(u8 *memory);
};
void Online_SetInstanceEnv(const OnlineInstanceEnv *env);

// Online_Start's calls (the bench's recorders; each has a bench of its own).
struct OnlineStartEnv {
    void(__fastcall *releaseNews)(u8 *session);    // Online_ReleaseNews (0x5b6b20)
    void(__fastcall *resetCity)(u8 *session);      // Online_ResetCity (0x5b7430)
    void(__fastcall *resetUpload)(u8 *session);    // Online_ResetUpload (0x5b8400)
    void(__fastcall *resetDownload)(u8 *session);  // Online_ResetDownload (0x5b9040)
    s32(__cdecl *message4)();                      // Dialog7cca94_Message4IfUp (0x4c25e0)
    u8 *(__cdecl *instance)();                     // Online_Instance (0x48b390)
    void(__fastcall *listStart)(u8 *list, void *, u32 value, u32 callback, u32 user);  // ListRequest_Start66 (0x5bcd10)
    void(__cdecl *openDialog)(u32 value);          // Dialog7cc530_Open (0x4ba980)
};
void Online_SetStartEnv(const OnlineStartEnv *env);

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

// 0x5b7430 (thiscall; name unconfirmed; six callers): the city's connection reset. +0xd8 = the time (Time_GetMs), +0xdc = 1. With a City
// (+0xd4): City::Release (0x6fd4f4); when still set ~City (0x6fd540) and freed; +0xd4 cleared; "ONLINE CITY: Connection released" said.
// Then the lists the dwords +0xe0 to +0x100 point to are reset (+4 = -1, +8 to +0x20 = 0): +0xe0's native list (+0x30, when set)
// released by ccCityList::Release (0x6fd538), +0xe4 / +0xe8 / +0xec's by ccParkList::Release (0x6fd4e8), each +0x30 then cleared;
// +0xf0 and +0xf4 Shared_Release66, +0xf8 Shared_Release10; +0xfc and +0x100 Obj_ResetAndRelease26 / 34 (0, 0).
void __fastcall Online_ResetCity(u8 *session);

// 0x5b72f0 (thiscall; name unconfirmed; six callers): the city's connection opened unless there is one (+0xd4). A City is made (Mem_Alloc
// 0xc, City::City 0x6fd4e4; its result, or 0 when the block is not had, is +0xd4) and "About to connect" said; City::Connect (0x6fd534) to
// the server name [0x7a1524] (when 0 the empty string at 0x6fe5b0), Global7cc53c_Pick7cc4f0, the port word [0x7a1556] (pushed as a dword
// whose high half is the server name's, sic) and the callback 0x5b72c0. Not 1: "Could not connect" said and the City released as
// Online_ResetCity does. 1: "Connected" said, +0xd8 = Time_GetMs, City::SetSessionInfo (0x6fd53c) with the language's id (0x78b530) and
// +0x34, +0x38, +0x28; +0xdc = 1. Returns +0xd4. (The City::City call's unwind, freeing the block, is not kept.)
u8 *__fastcall Online_OpenCity(u8 *session);

// 0x5b6070 (thiscall; name invented): the news, city, upload and download reset (each as above), then Dialog7cca94_Message4IfUp.
void __fastcall Online_ResetConnections(u8 *session);

// 0x5b5e40 (thiscall; name invented): the login released, +0x64, +0x60, +0x68, +4, +8, +0x34, +0x38 and +0x50 = 0, the strings at +0xc, +0x20
// and +0x3c assigned ""; then Online_ResetConnections and Dialup_HangUpIfOpen.
void __fastcall Online_ResetAll(u8 *session);

// 0x5b87e0 (thiscall (a, b, c), pops 12; name invented, unconfirmed): Online_ReleaseNews; +0x74, +0x90 and +0xac = -1; +0xc8, +0xcc, +0x78, +0x94, +0xb0, +0xd0 = 0; the strings +0x7c (0x786de4), +0x98 and +0xb4 (0x78a374)
// assigned by slot 5; the city, upload and download reset; the dialog's message. +0x3a8 = a, +0x3c0 = b, +0x3c4 = c. Logged in (+0x60): the instance's list +0xf0 starts on its +0x3a8 with the callback 0x5b88e0. Otherwise
// +4 = 0x5b87a0, +8 = 0 and Dialog7cc530_Open (0x5b5cb0).
void __fastcall Online_Start(u8 *session, void *, u32 a, u32 b, u32 c);

// 0x5b50b0 (thiscall; name invented): the session's constructor. Its strings made and emptied, its members as the resets leave them, the nine
// lists made (+0xe0 to +0x100: none when a block is not had), Online_ResetCity on them, the download object +0x3a4 (Class70293c_Ctor).
// The news' and the download's releases in the listing see their members just cleared, so they do nothing but the reset. Returns the session.
// (The unwinds of a throwing constructor are not kept.)
u8 *__fastcall Online_Construct(u8 *session);

// 0x48b390 (cdecl; name invented): the session [0xf7d88c], made the first time (0x404 bytes, constructed by OnlineInstanceEnv's ctor; memory refused: none, asked again next time).
u8 *__cdecl Online_Instance();

// 0x5b7960 (thiscall; park; pops 4; name unconfirmed): with the city's connection open (Online_OpenCity), City::UnpublishPark (0x6fd4fc)
// with 0, the park and +0x34; returns its answer, else 0.
u32 __fastcall Online_UnpublishPark(u8 *session, void *, u32 park);

// 0x5b6da0 (thiscall, pops 0x10; name unconfirmed): an answer from the online city. Codes 2 to 6 bring a native list for a request: with the
// list that asked for it (its +4 the request; code 2: +0xe0; 3: +0xe4 to +0xf4; 4: +0xfc; 5: +0xf8; 6: +0x100) its old +0x30, when set, is
// released (the kind's Release), +0x30 = the data and +0x14 = +0x10 = 1. For no such list, the data, when set, loses a count (+0xc) and at 0
// is destroyed (the kind's destructor) and freed. Each case says so with OutputDebugStringA. Codes -10, -6, -5, -4, -2, -1 and 7 are
// failures (error 0x56, 0x4e, 0x4d, 0x4c, 0x4b, 0x37, 0; 7 says "CRC match", 8 "No CRC match" and ends): the first list of +0xe0 to +0x100
// with that request gets +0x18 = the error, +0x14 = 0, +0x10 = 1. Other codes do nothing.
void __fastcall Online_ListAnswered(u8 *session, void *, s32 code, s32 request, u32, u8 *data);
}
