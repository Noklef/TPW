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
    u8 *(__fastcall *findKind)(u8 *owner);         // Owner2c_FindKind100 (0x5bd690)
    u32(__fastcall *uploadConnect)(u8 *session);   // Online_UploadConnect (0x5b8290)
    u32(__cdecl *post)(u32 code, u32 param);       // CommandQueue_Post (0x47f410)
    u32(__fastcall *findLast)(u8 *object, void *, const char *name);  // Names_FindLastValue (0x5bd230)
    void(__cdecl *crtFree)(void *block);           // CRT_free (0x67ad90)
};
void Online_SetStartEnv(const OnlineStartEnv *env);

// Online_CityPoll's calls: the ports by default; the bench's recorders.
struct OnlineCityPollEnv {
    void(__fastcall *start10)(u8 *list);           // ListRequest_Start10 (0x5bd070)
    void(__fastcall *start26)(u8 *list);           // ListRequest_Start26 (0x5bd4b0)
    void(__fastcall *start34)(u8 *list);           // ListRequest_Start34 (0x5bd8c0)
    void(__fastcall *start66)(u8 *list, void *, u32 value, u32 callback, u32 user);  // ListRequest_Start66 (0x5bcd10)
    void(__fastcall *done10)(u8 *list, void *, u32 failed);  // ListRequest_Done10 (0x5ba030)
    void(__fastcall *done26)(u8 *list, void *, u32 failed);  // ListRequest_Done26 (0x5ba2f0)
    void(__fastcall *done34)(u8 *list, void *, u32 failed);  // ListRequest_Done34 (0x5ba560)
    void(__fastcall *done66)(u8 *list, void *, u32 failed);  // ListRequest_Done66 (0x5b9d60)
    void(__fastcall *done30)(u8 *list, void *, u32 failed);  // ListRequest_Done30 (0x5b9b00)
    u32(__cdecl *now)();                           // Time_GetMs (0x5f5fa0)
    void(__cdecl *showText)(u32 text);             // Dialog7cca94_ShowText (0x4c25b0)
    s32(__cdecl *message4)();                      // Dialog7cca94_Message4IfUp (0x4c25e0)
    void(__cdecl *setProgress)(s32 percent);       // Dialog7cca94_SetProgress (0x4c2600)
    void(__cdecl *waitingRow)(s32 percent);        // Dialog7cc944_SetWaitingRow (0x4c0660)
};
void Online_SetCityPollEnv(const OnlineCityPollEnv *env);

// Online_Dtor's calls: the ports by default; the bench's recorders.
struct OnlineDtorEnv {
    void(__fastcall *releaseLogin)(u8 *session);   // Online_ReleaseLogin (0x5b64d0)
    void(__fastcall *resetNews)(u8 *session);      // Online_ResetNews (0x5b6980)
    void(__fastcall *resetCity)(u8 *session);      // Online_ResetCity (0x5b7430)
    void(__fastcall *resetUpload)(u8 *session);    // Online_ResetUpload (0x5b8400)
    void(__fastcall *resetDownload)(u8 *session);  // Online_ResetDownload (0x5b9040)
    s32(__cdecl *message4)();                      // Dialog7cca94_Message4IfUp (0x4c25e0)
    void(__cdecl *hangUp)();                       // Dialup_HangUpIfOpen (0x4bae40)
    void(__fastcall *stringDtor)(u8 *string);      // String_Dtor (0x5f61e0)
    void(__fastcall *wstringDtor)(u8 *string);     // WString_Dtor (0x5f6a60)
};
void Online_SetDtorEnv(const OnlineDtorEnv *env);

// The transfers' ends' calls: the ports by default; the bench's recorders.
struct OnlineEndEnv {
    u32(__cdecl *post)(u32 code, u32 param);       // CommandQueue_Post (0x47f410)
    u8 *(__fastcall *openCity)(u8 *session);       // Online_OpenCity (0x5b72f0)
    void(__fastcall *releaseNews)(u8 *session);    // Online_ReleaseNews (0x5b6b20)
    void(__fastcall *resetNews)(u8 *session);      // Online_ResetNews (0x5b6980)
    void(__fastcall *resetCity)(u8 *session);      // Online_ResetCity (0x5b7430)
    void(__fastcall *resetUpload)(u8 *session);    // Online_ResetUpload (0x5b8400)
    void(__fastcall *resetDownload)(u8 *session);  // Online_ResetDownload (0x5b9040)
    s32(__cdecl *message4)();                      // Dialog7cca94_Message4IfUp (0x4c25e0)
    void(__cdecl *hangUp)();                       // Dialup_HangUpIfOpen (0x4bae40)
    void(__cdecl *showText)(u32 text);             // Dialog7cca94_ShowText (0x4c25b0)
    void(__cdecl *setProgress)(s32 percent);       // Dialog7cca94_SetProgress (0x4c2600)
    u8 *(__cdecl *instance)();                     // Online_Instance (0x48b390)
    void(__fastcall *uploadStart)(u8 *session);    // Online_UploadStart (0x5b7cd0)
    void(__cdecl *openDialog)(u32 value);          // Dialog7cc530_Open (0x4ba980)
};
void Online_SetEndEnv(const OnlineEndEnv *env);

// Online_Poll's calls: the ports by default; the bench's recorders.
struct OnlinePollEnv {
    void(__fastcall *loginFinish)(u8 *session, void *, u32 failed);  // Online_LoginFinish (0x5b6510)
    void(__fastcall *newsPoll)(u8 *session);       // Online_NewsPoll (0x5b6a30)
    void(__fastcall *cityPoll)(u8 *session);       // Online_CityPoll (0x5b75d0)
    void(__fastcall *uploadPoll)(u8 *session);     // Online_UploadPoll (0x5b8490)
    void(__fastcall *downloadEnd)(u8 *session, void *, u32 failed);  // Online_DownloadEnd (0x5b90e0)
    s32(__cdecl *message4)();                      // Dialog7cca94_Message4IfUp (0x4c25e0)
    void(__cdecl *showText)(u32 text);             // Dialog7cca94_ShowText (0x4c25b0)
    void(__cdecl *setProgress)(s32 percent);       // Dialog7cca94_SetProgress (0x4c2600)
};
void Online_SetPollEnv(const OnlinePollEnv *env);

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

// 0x5b8290 (thiscall; the SEH frame left out; name invented, unconfirmed): the connection +0x104 released when set (0x6fd5ec), a new one made (0x6fd5dc; a failed block: 0); said; the record of +0xfc
// (Owner2c_FindKind100) gives the host (+0x2c, wide, in ANSI) and port (word +0x3c, in AX: the record's high half stays above it, sic): connect 0x6fd5e0 (host, Global7cc53c_Pick7cc4f0, port, the
// callback 0x5b8260). Answering 1: said, 0x6fd5e4 (Language_Id, +0x34, +0x38, +0x28), returns 1; else said, returns 0.
u32 __fastcall Online_UploadConnect(u8 *session);

// 0x5b7cd0 (thiscall; name invented, unconfirmed): without flag 0x10 of +0x58, or no Upload (+0x104) and Online_UploadConnect failing: said and given up (code 0x4e, 0x4f posted; with a login (+0x320) the
// session reset as Online_ResetAll; the callback +0x374 (1, +0x378); the Upload released). Else said, +0x324 = 1, the request sent (0x6fd5d0: +0x338, a stack record, 0): +0 = +0x344, +4 the byte
// Names_FindLastValue(+0xf8, +0x114), +5 = +0x370, +6 and +0x2e the wide texts +0x350 and +0x364, +0x22e 0x200 bytes of +0x120; said.
void __fastcall Online_UploadStart(u8 *session);

// 0x5b7fa0 (thiscall, pops 16; name invented, unconfirmed): the Upload's events. 0x50: the record's uses (+8) down, at none its block (+0) freed and it; said; 0x6fd5e8 (+0x338, 0). 0: said, +0x32c and
// +0x328 = 1. Errors -40 (0xde; +0x380 = [0x78ba94]), -20 (0x55), -6 (0x4e), -5 (0x4d), -4 (0x4c), -2 (0x4b), -1 (0x37): the code in +0x37c, +0x32c = 0, +0x328 = 1. Any other: nothing.
void __fastcall Online_UploadEvent(u8 *session, void *, u32 code, u32 a, u32 b, u8 *record);

// 0x5b8260 (stdcall; name invented, unconfirmed): Online_UploadEvent on Online_Instance: the callback Online_UploadConnect gives.
void __stdcall Online_UploadCallback(u32 code, u32 a, u32 b, u8 *record);

// 0x5b75d0 (thiscall; name invented, unconfirmed): with a City (+0xd4): first (+0xdc set) the four lists +0xf8, +0xfc, +0x100, +0xf4 requested and +0xdc cleared. Each of the nine lists flagged (+0xc and +0x10
// set) is answered (+0xc cleared, its ListRequest_Done*(0)). Any +0xc still set: the time in +0xd8; the transfer 0x6fd4f8(0), or (1) when 0; its list's text (0xf9 on) shown, none: Dialog7cca94_Message4IfUp;
// the progress 0x6fd4e0 as a percent (when the total is above 0) to Dialog7cca94_SetProgress, and to Dialog7cc944_SetWaitingRow for +0xec's list. None: Dialog7cca94_Message4IfUp, and 360 s past +0xd8 the City
// closed (0x6fd4f4), destroyed (0x6fd540), freed, said, +0xd4 cleared.
void __fastcall Online_CityPoll(u8 *session);

// 0x5b5920 (thiscall): the destructor. The login released, its state and strings cleared; the News, City, Upload and Download reset, Message4 and hang up. The News (still set) dropped, the lists at
// +0xe0..+0x100 deleted (not cleared), the Upload and the Download's object and block dropped, the object at +0x3a4 deleted (not cleared). The strings destroyed, the node list at +0x3cc freed.
void __fastcall Online_Dtor(u8 *session);

// 0x5b90e0 (thiscall; failed; pops 4): +0x398 (set) posted and cleared. Not failed and nothing posted: the City opened and its 0x6fd52c(0, the block's size, +0x34), +0x3f0 = 1. Otherwise with +0x388:
// the login dropped and everything reset. The callback +0x3c0 (failed, +0x3c4), the Download's object and block dropped, +0x384 = 0.
void __fastcall Online_DownloadEnd(u8 *session, void *, u32 failed);

// 0x5b8490 (thiscall): with an Upload and +0x324. +0x328 clear: the progress shown (text 0x101). Set: ok = +0x32c clear, +0x324 = 0, +0x37c / +0x380 posted; ok or posted, with +0x320: the login dropped
// and everything reset. The callback +0x374 (ok, +0x378) as read before the reset; the Upload released.
void __fastcall Online_UploadPoll(u8 *session);

// 0x5b79f0 (cdecl; failed): not failed: Online_UploadStart. Failed: as Online_UploadPoll's end, the callback with 1.
void __cdecl Online_UploadLoginDone(u32 failed);

// 0x5b7ba0 (thiscall; pops 28): the Upload reset; the string at +0x330 assigned the first argument, +0x344 the second, the strings at +0x348 and +0x35c the next two, +0x370, +0x374 and +0x378 the rest.
// Logged in: Online_UploadStart. Otherwise +0x320 = 1, the login's callback +4 = Online_UploadLoginDone, +8 = 0, and the login dialog opened.
void __fastcall Online_UploadBegin(u8 *session, void *, const void *a, u32 b, const void *c, const void *d, u32 e, u32 f, u32 g);

// 0x5b5bf0 (thiscall): the frame's poll. With an Auth and +0x64: the login finished, +0x64 = 0. The news, city and upload polled. A download running (+0x39c, +0x3a0 and +0x38c set): when ended (+0x390)
// Message4, +0x38c = 0, and Online_DownloadEnd(+0x394 clear); otherwise its text (0x102) and, with a total above 0, its progress.
void __fastcall Online_Poll(u8 *session);

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
