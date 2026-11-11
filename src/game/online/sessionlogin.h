// The online session's login and its news connection (see session.h for the session's other members). Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls out to the dialogs, the balance file and the command queue; the bench's recorders.
struct OnlineLoginEnv {
    void(__cdecl *openLoginDialog)(u32 callback);                                // Dialog7cc530_Open
    u32(__cdecl *finishDialog)(u32 value);                                       // Dialog7cc5f4_Finish
    s32(__cdecl *openListsDialog)();                                             // Dialog7cc654_Open
    void(__cdecl *showText)(u32 index);                                          // Dialog7cca94_ShowText
    s32(__cdecl *message4)();                                                    // Dialog7cca94_Message4IfUp
    void(__cdecl *setProgress)(u32 percent);                                     // Dialog7cca94_SetProgress
    u32(__fastcall *loadBalance)(u8 *self, void *, const char *path, u32 keep);  // BalanceFile_Load
    void(__cdecl *applyBalance)();                                               // Online_ApplyBalance
    void(__cdecl *openNewsDialog)();                                             // Dialog7cc61c_Open
    u32(__cdecl *post)(u32 code, u32 param);                                     // CommandQueue_Post
    void(__cdecl *free)(void *block);                                            // CRT_free
};
void OnlineLogin_SetEnv(const OnlineLoginEnv *env);   // nullptr: the game's

extern "C" {

// 0x5b5cb0 (cdecl, the login dialog's callback): Online_LoginFinish(1) on Online_Instance.
void __cdecl Online_LoginDialogClosed();

// 0x5b5cc0 (thiscall (callback, user), pops 8): kept at +4 and +8; the login dialog opened with callback 0x5b5cb0.
void __fastcall Online_OpenLoginDialog(u8 *session, void *, u32 callback, u32 user);

// 0x5b5ce0 (stdcall, 4 arguments; Auth::Connect's callback): Online_LoginResult on Online_Instance.
void __stdcall Online_LoginAnswer(s32 status, u32 a, u32 b, u8 *record);

// 0x5b5d10 (thiscall (name, password), pops 8): the texts assigned to +0xc and +0x20 (vtable +0x14), the login released (Online_ReleaseLogin
// inlined), a new Auth (0xc bytes; none when not had) at +0, Auth::Connect to the host [0x7a1514] (0: the empty string at 0x6fe5b0) with
// Global7cc53c_Pick7cc4f0, the port word [0x7a1554] over the host's high half (sic) and callback 0x5b5ce0. Refused: +0x68 = 0x4f and
// Online_LoginFinish(0). Else Auth::Login with the language's id, +0x14, +0x28 and 0.
void __fastcall Online_LogIn(u8 *session, void *, const u16 *name, const u16 *password);

// 0x5b6510 (thiscall (flag), pops 4): an error +0x68 posted (CommandQueue_Post, param 0) and cleared. Flag set: the callback +4, when set, called
// cdecl (flag, +8), +4 and +8 cleared; the login released; +0x60, +0x64, +0x68, +4, +8 cleared, +0xc, +0x20, +0x3c assigned the empty text, +0x34,
// +0x38, +0x50 cleared. Flag 0 and logged in (+0x60): the login released, the dialog [0x7cc5f4] finished with 0, +4 and +8 moved to +0xc8 and +0xcc,
// and unless the lists' dialog opens (Dialog7cc654_Open answers not 0) Online_OpenNews.
void __fastcall Online_LoginFinish(u8 *session, void *, u32 flag);

// 0x5b6630 (stdcall, 4 arguments; News::Connect's callback): Online_NewsResult on Online_Instance.
void __stdcall Online_NewsAnswer(s32 status, u32 a, u32 b, u8 *record);

// 0x5b6660 (thiscall): the News released and a new one (0xc bytes; none when not had) at +0x6c, the text 0xf4 shown, News::Connect to the host [0x7a1534]
// (0: 0x6fe5b0) as Online_LogIn's. Refused: +0xd0 = 0x4f and Online_NewsFinish. Else the three requests' ids at +0x74, +0x90, +0xac.
void __fastcall Online_OpenNews(u8 *session);

// 0x5b67d0 (thiscall (status, a, b, record), pops 0x10): status 50: a request's text ([record], a byte order mark skipped) to +0x98 (+0x94 = 1) when a is
// +0x90, or to +0xb4 (+0xb0 = 1) when a is +0xac; the record loses a count (+8), at 0 its text (CRT_free) and it are freed. Status 51 with +0x74 not 0
// (sic: not compared with a): [record] to +0x7c, +0x78 = 1. Statuses -6, -5, -4, -2: +0xd0 = 0x4e, 0x4d, 0x4c, 0x4b.
void __fastcall Online_NewsResult(u8 *session, void *, s32 status, u32 a, u32 b, u8 *record);

// 0x5b6a30 (thiscall): with a News at +0x6c: an error +0xd0 or all three answers (+0x78, +0x94, +0xb0): message 4 and Online_NewsFinish. Else the
// request News::GetActive (0x6fd5a4) gives for 0 or else 1, its text shown (0xf5, 0xf6, 0xf7, other ids message 4), and from News::GetProgress (0x6fd584)
// (done, total) with total above 0 the percentage done * 100 / total set.
void __fastcall Online_NewsPoll(u8 *session);

// 0x5b6b60 (thiscall, SEH dropped): the callback +0xc8, when set, called cdecl (0, +0xcc) and +0xc8, +0xcc cleared. An error +0xd0: posted, then
// Online_ResetNews. Else the root (0xf816e8) and the two texts 0xf7cd80, 0xf7c9f0 are the balance file's path; the text +0x7c written to it
// (FileStream_OpenPath, mode 0xc0000010; no file: not) and closed; written: BalanceFile_Load on 0x785828 with keep 1, and when it gives 1
// Online_ApplyBalance. The News released; the first time (+0x70) the news dialog opened and +0x70 = 1.
void __fastcall Online_NewsFinish(u8 *session);

}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[40], gOnline_LogIn_773414, 0x00773414, 0x28);
GAME_DATA(char[32], gOnline_LogIn_77343c, 0x0077343c, 0x20);
GAME_DATA(char[40], gOnline_OpenNews_7734a0, 0x007734a0, 0x28);
GAME_DATA(char[32], gOnline_OpenNews_7734c8, 0x007734c8, 0x20);
GAME_DATA(char[32], gOnline_OpenNews_7734e8, 0x007734e8, 0x20);
GAME_DATA(char[36], gOnline_NewsResult_773508, 0x00773508, 0x24);
GAME_DATA(char[28], gOnline_NewsResult_77352c, 0x0077352c, 0x1c);
GAME_DATA(char[32], gOnline_NewsResult_773548, 0x00773548, 0x20);
GAME_DATA(char[28], gOnline_NewsResult_773568, 0x00773568, 0x1c);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const u8[0x10], gSessionlogin_702808, 0x00702808, 0x10);   // a GUID passed to the news request
