// The online chat's events and its buddy flags (names invented, unconfirmed).
#pragma once

#include "../core/stdwstring.h"
#include "../../../re/types.h"

extern "C" {

// 0x4e6e80 (thiscall on the avatar): the event a tokens vector carries, the first token's word +0x20 its code (token i at +0x24 * i, its
// word at +0x20). 0xb, 0xc connected and lost (Window7cc8bc_SetC3128); 0xf, 0x10 notices that need a word in token 1; 0x11 - 0x13, 0x16 -
// 0x1c, 0x3f - 0x42, 0x2a, 0x2e, 0x2f, 0x44, 0x45, 0x3d, 0x1e, 0x1f, 0x3c, 0x47, 0x51 notices of one to three token texts; 0x22 a chat
// player's move (token 4: walk to, place at, release, place with script 3); 0x23 - 0x26 a person's status and arrival or departure, kept
// in the room list +0xe80 and the buddy set +0xe90; 0x27 a person seen; 0x38, 0x39 a buddy added or removed; 0x3a, 0x3b an ignored name
// added or removed (+0xea0); 0x4f the buddy and ignore sets told to the Chat; 0x52 the avatar sent to a room or to a person's cell.
void __fastcall ChatAvatar_HandleEvent(u8 *avatar, void *, const u8 *tokens);

// 0x4ee710 (thiscall on the avatar): the buddy set +0xe90's entry of name given the flags (bit 1 at its +0x1c, bit 2 at its +0x1d), made when
// missing; its ChatAvatar_NameFlags told to both window lists. Returns whether it was made.
u8 __fastcall ChatAvatar_SetBuddy(u8 *avatar, void *, const StdWString *name, u32 flags);

// 0x4eec40 (thiscall on the avatar, pops 4; name invented, unconfirmed): the result of a connection told as a notice in colour 7: 0 entry 0x80, -1, -4
// and -5 0x81, -2 0x82, -0x1e 0x83 (the string by 0x4051d0); -0x1f 0xb4 and any other 0x81 (the string by _Grow); then the byte +0xec0 = 1 and a line on the
// debugger's output. The strings' allocator bytes are the result's low byte, which nothing reads.
void __fastcall ChatAvatar_ShowResult(u8 *avatar, void *, s32 result);

// 0x4e6790 (thiscall on the avatar, pops 4; name invented, unconfirmed): the event of a tokens vector while not yet in a room (+0x28 clear), the first
// token's word +0x20 its code. 1, 7, 0x2c nothing. 0x34: the room (the session's Obj_Get3a4 +8 as %d) and the session's user (+0x5c) told to the Chat (slot
// 0x6fd480), and to slot 0x6fd4c4 that object's +0x14 text, the text at 0x78a374, the +0x14 text of the owner's item of its key +0x48 (Owner2c_FindByKey),
// the room's string, 0x100. 0x31: +0x28 = 1 and Windows7cc8bc_SelectSecond. 0x36: +0x28 = 1, ChatAvatar_AddName of the name +8, Game_StartCrowd on the
// game, 0x4ef3b0 and 0x4efc10 on the path 0x786de4. 0x28, 0x29, 0x2b, 0x2d, 0x30, 0x32, 0x33: the command 0xa2, 0xa3, 0xa5, 0xa8, 0xaa, 0xae, 0xaf
// posted with [0x78bac8]; those and any other code (not 0x34 and the rest named): the chat's OnlineChat_Set29And2a. Each says so on the debugger's output.
void __fastcall ChatAvatar_LobbyEvent(u8 *avatar, void *, const u8 *tokens);

// 0x4e6560 (thiscall on the chat; name invented, unconfirmed): under the lock +0x30 the queue of tokens vectors (the deque +0xec8: iterators +0xecc, +0xedc,
// count +0xef4) handed out and emptied as CountedPairVectorDeque_Clear does: a vector with tokens whose first word is 0x2711 goes to ChatAvatar_ShowResult
// (on the chat's instance; the second token's word), else to ChatAvatar_HandleEvent (+0x28 set) or ChatAvatar_LobbyEvent. The map is left in +0xeec (sic).
void __fastcall ChatAvatar_PumpEvents(u8 *chat);

// 0x4e5230 (cdecl, a callback handed to the session's join; name invented, unconfirmed): a failure (nonzero): ChatAvatar_GotoDone on the online chat and
// "goto ftp download failed" on the debugger's output, EAX the import's. None: Global879088_Set_5508b0(2).
u32 __cdecl ChatAvatar_GotoFtpDone(s32 failed);

// 0x4e6170 (thiscall on the chat, each frame; name invented, unconfirmed): ChatAvatar_PumpEvents; then with a vote +0xec4 and the byte +0xec0 set the vote is
// done: a line on the debugger's output, Vote::Release (0x6fd604), ~Vote (0x6fd608) and its block freed unless +0xec4 was cleared meanwhile, both fields
// cleared, a second line, Window7cc8bc_SetC3129(0).
void __fastcall ChatAvatar_Update(u8 *chat);

}

// What the events call through an environment (the fncmp driver's recorders; the game's by default: the ports, or the game's addresses for the unported).
struct ChatEventEnv {
    void(__stdcall *listSetData)(const u16 *text, u32 data);                              // Window7cc8bc_ListSetDataStd 0x481850
    void(__stdcall *listSetData1)(const u16 *text, u32 data);                             // Window7cc8bc_ListSetData1Std 0x4818a0
    void(__stdcall *listRemove)(const u16 *text);                                         // Window7cc8bc_ListRemoveStd 0x481840
    void(__stdcall *listRemove1)(const u16 *text);                                        // Window7cc8bc_ListRemove1Std 0x481890
    void(__fastcall *playerCtor)(void *player, void *, u32 cell, s32 x, const StdWString *name, u32 zero);   // 0x4f7b20
    void(__fastcall *thingDelete)(u8 *thing);                                             // Thing_Delete 0x50b780
    u8 *(__fastcall *tokenNarrow)(const u8 *token, void *, void *out);                    // StdString_CopyInto 0x4c4a80
    void(__fastcall *join)(u8 *session, void *, s32 room, u32 callback, u32 zero);        // 0x5b87e0 (B)
    u32(__fastcall *destination)(u8 *sprite, void *, s32 x, s32 y);                       // Sprite_SetDestinationMarked 0x4f7de0
    void(__fastcall *placeAt)(u8 *thing, void *, u32 x, u32 y);                           // Thing_ResetPlaceAt 0x4f7e20
    void(__fastcall *releaseScripts)(u8 *person);                                         // Person_ReleaseScripts 0x4f9ed0
    u32(__fastcall *placeScript)(u8 *person, void *, u32 x, float y, u32 z, u32 r15, u32 anim);   // Person_PlaceScript 0x4f9e60
    void(__cdecl *setRegF)(s32 slot, u32 index, u32 bits);                                // SpriteScript_SetRegF 0x4755d0
    u8(__fastcall *placeAtCell)(u8 *self, void *, u32 handle, u32 cell);                  // ChatAvatar_PlaceAtCell 0x4ebfb0
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);               // Charset_FormatWide 0x6acd60 (the driver's recorder logs all three text arguments)
    void(__cdecl *setC3128)(u8 value);                                                    // Window7cc8bc_SetC3128 0x4bd400
    u32(__cdecl *post)(u32 code, u32 param);                                              // CommandQueue_Post 0x47f410
    u32(__cdecl *selectSecond)();                                                         // Windows7cc8bc_SelectSecond 0x4bce80
    void(__fastcall *addName)(u8 *avatar, void *, const StdWString *name, u32 unused);    // ChatAvatar_AddName 0x4ee6d0
    void(__fastcall *startCrowd)(u8 *game);                                               // Game_StartCrowd 0x516700
    void(__fastcall *readBuddies)(u8 *avatar, void *, const char *path);                  // 0x4ef3b0
    void(__fastcall *readIgnored)(u8 *avatar, void *, const char *path);                  // 0x4efc10
    void(__fastcall *handleEvent)(u8 *avatar, void *, const u8 *tokens);                  // ChatAvatar_HandleEvent 0x4e6e80
    void(__cdecl *setC3129)(u8 value);                                                    // Window7cc8bc_SetC3129 0x4bd450
};
void ChatEvents_SetEnv(const ChatEventEnv *env);   // nullptr: the game's

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[4], gChatevents_759920, 0x00759920, 0x4);
GAME_DATA(char[28], gChatAvatar_GotoFtpDone_75c0d4, 0x0075c0d4, 0x1c);
GAME_DATA(char[48], gChatAvatar_Update_75c120, 0x0075c120, 0x30);
GAME_DATA(char[72], gChatAvatar_Update_75c150, 0x0075c150, 0x48);
GAME_DATA(char[72], gChatAvatar_LobbyEvent_75c198, 0x0075c198, 0x48);
GAME_DATA(char[48], gChatAvatar_LobbyEvent_75c1e0, 0x0075c1e0, 0x30);
GAME_DATA(char[56], gChatAvatar_LobbyEvent_75c210, 0x0075c210, 0x38);
GAME_DATA(char[52], gChatAvatar_LobbyEvent_75c248, 0x0075c248, 0x34);
GAME_DATA(char[72], gChatAvatar_LobbyEvent_75c27c, 0x0075c27c, 0x48);
GAME_DATA(char[48], gChatAvatar_LobbyEvent_75c2c4, 0x0075c2c4, 0x30);
GAME_DATA(char[92], gChatAvatar_LobbyEvent_75c2f4, 0x0075c2f4, 0x5c);
GAME_DATA(char[52], gChatAvatar_LobbyEvent_75c350, 0x0075c350, 0x34);
GAME_DATA(char[52], gChatAvatar_LobbyEvent_75c384, 0x0075c384, 0x34);
GAME_DATA(char[44], gChatAvatar_LobbyEvent_75c3b8, 0x0075c3b8, 0x2c);
GAME_DATA(char[52], gChatAvatar_LobbyEvent_75c3e4, 0x0075c3e4, 0x34);
GAME_DATA(char[64], gChatAvatar_HandleEvent_75c448, 0x0075c448, 0x40);
GAME_DATA(char[64], gChatAvatar_HandleEvent_75c488, 0x0075c488, 0x40);
GAME_DATA(char[64], gChatAvatar_HandleEvent_75c4c8, 0x0075c4c8, 0x40);
GAME_DATA(char[76], gChatevents_75c508, 0x0075c508, 0x4c);
GAME_DATA(char[44], gChatAvatar_ShowResult_75c69c, 0x0075c69c, 0x2c);
