// The postcard sent: its picture saved and copied, the mail set up and connected. Reimplementation of 0x4a8600 (name invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

// The calls (the unported by address, the ports by default; the bench's recorders).
struct PostcardSendEnv {
    void(__fastcall *savePicture)(u8 *panel, void *, const char *path);                          // 0x5d4a30
    u8 *(__fastcall *mailCtor)(u8 *memory);                                                       // 0x5c93c0
    void *(__fastcall *assign)(u8 *string, void *, const char *chars, u32 n);                     // StdString_AssignChars
    char *(__cdecl *strtok)(char *text, const char *delimiters);                                  // 0x67c4e0
    void(__fastcall *pushToken)(u8 *owner, void *, const char *chars);                            // TokenList_PushChars
    void(__fastcall *attach)(u8 *mail, void *, const char *path);                                 // 0x5c99f0
    u8 *(__fastcall *managerCtor)(u8 *memory);                                                    // 0x5c7420
    void(__fastcall *useManager)(u8 *mail, void *, u8 *part);                                     // 0x5ca010
    u32(__fastcall *post)(u8 *centre, void *, u8 *msg);                                           // MessageCentre_Post
    void *(__fastcall *assignWide)(u8 *string, void *, const u16 *chars, u32 n);                  // 0x4051d0, std::wstring::assign
    u32(__fastcall *connect)(u8 *mail, void *, const char *host, u32 port, u8 *pick, u32 zero);   // 0x5c9f10
    u32(__cdecl *command)(u32 code, u32 param);                                                   // CommandQueue_Post
};
void PostcardSend_SetEnv(const PostcardSendEnv *env);

extern "C" {

// 0x4a8600 (cdecl): the path [0x7cbcb0] = "%s%s" of [0x7cb958] and "postcard"; the panel [0x7cc18c] saves there (0x5d4a30); the local path "%s%08d.jpg" of [0x7cb958] and the counter [0x7cbf38] (then one more); CopyFileA (path, local, 0).
// The mail: 0x94 bytes (none: a fault on its first use, as the original) made by 0x5c93c0. The body [0x7cb408] ("%S", 100 bytes on the stack) to its string +0x1c. The addresses [0x7cb4d0] ("%S") split by strtok on ";, \n", each to its list (TokenList_PushChars). The local path to the string +0x2c, then attached (0x5c99f0). Its +0x90 = 0x4a7bf0 (Postcard_OnSent).
// The session (Online_Instance) not logged in: 0x5ca010 with +0x64 of the manager [0xf81ebc] (made the first time: 0x110 bytes, 0x5c7420), message 0x13 of 8 posted. Logged in: the user's name (each second byte of Online_UserName up to a 0 byte) to the string +0, the session's +0x34 and +0x38 to +0x3c and +0x40, Online_String14 and Online_String28 (wide here) to the wide strings +0x44 and +0x54 (0x4051d0), +0x64 = 9,
// then connect (0x5c9f10) to the host [0x7a14f4] (0: the empty string at 0x6fe5b0, an empty one: "ftp.ea-europe.com" at 0x754454 and port 0x19; else the port, a signed word, [0x7a1550]) with Global7cc53c_Pick7cc4f0 and 0; an answer other than 0: the command 0x39 (CommandQueue_Post, 0).
void __cdecl Postcard_Send();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[8], gPostcardsend_74d320, 0x0074d320, 0x8);
GAME_DATA(char[4], gPostcardsend_74f8c4, 0x0074f8c4, 0x4);
GAME_DATA(char[20], gPostcardsend_754454, 0x00754454, 0x14);
GAME_DATA(char[8], gPostcard_Send_754468, 0x00754468, 0x8);
GAME_DATA(char[12], gPostcard_Send_754470, 0x00754470, 0xc);
GAME_DATA(u8[512], gPostcard_Send_7cbcb0, 0x007cbcb0, 0x200);
