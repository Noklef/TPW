// The chat client object (the class chatlists.h saves and loads; names invented, unconfirmed): +0 the library's session (imports), +4 the player, +8 the user's name (a wide string, its text at +0xc), +0x1c the host, +0x2b a flag, +0x30 and +0x48 two lists,
// +0x260 a text buffer, +0xe60 a block of containers, +0xec8 a deque, +0xeb1 and +0xef8 flags, +0xf00 a wide string.
// Reimplementations of 0x4e5950, 0x4e5be0 and 0x4e5ef0, the originals' SEH frames dropped.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct ChatClientEnv {
    u8(__fastcall *saveBuddies)(u8 *self, void *, const char *name);    // ChatLists_SaveBuddies
    u8(__fastcall *saveIgnored)(u8 *self, void *, const char *name);    // ChatLists_SaveIgnored
    void(__fastcall *messageCtor)(u8 *self);                            // 0x5d29b0
    void(__fastcall *messageSend)(u8 *self);                            // 0x5d2a60
    void(__fastcall *elementDtor)(u8 *self, void *, u32 flags);         // 0x4f7430
    void(__fastcall *dequeNext)(u8 *deque);                             // 0x6c2910
    u32 *(__fastcall *treeFind)(u8 *self, void *, u32 *out, u32 key);   // 0x5ce530
    void(__fastcall *treeErase)(u8 *self, void *, u32 *out, u32 node);  // 0x4f3400
    void(__fastcall *dtor40)(u8 *self);                                 // 0x4e58e0
    void(__fastcall *dtor30)(u8 *self);                                 // 0x4e5870
    void(__fastcall *dtor20)(u8 *self);                                 // 0x4e5800
    void(__fastcall *listDtor)(u8 *self);                               // 0x5c8ba0
    u32(__fastcall *messageCheck)(u8 *self);                            // 0x5d2a20
    s32(__cdecl *format)(u16 *to, const u16 *format, const char *name); // CRT_swprintf
    void(__fastcall *entrance)(u8 *map, void *, u32 *out, u32 b);       // Map_EntrancePos
    u8 *(__fastcall *playerCtor)(u8 *player, void *, u32 cell, u32 sprite, const u8 *name, u32 one);  // ChatPlayer_Ctor
    u16 *(__fastcall *handle)(const u8 *thing, void *, u16 *out);       // Thing_GetHandle
    u8 *(__fastcall *byHandle)(u8 *world, void *, u32 handle);          // Thing_ByHandle
    void(__fastcall *placeAt)(u8 *thing, void *, u32 x, u32 y);         // Thing_ResetPlaceAt
    u8 *(__fastcall *modeCtor)(u8 *self);                               // MMOnline_Ctor
    u8 *(__fastcall *modeBase)(u8 *self);                               // MouseModeOnlineBase_Ctor
    void(__fastcall *setMode)(u8 *holder, void *, u8 *mode);            // MouseMode_Set
};
void ChatClient_SetEnv(const ChatClientEnv *env);

extern "C" {

// 0x4e5950: with a session: the import 0x6fd47c on it, the import 0x6fd4d8 on it and it freed, +0 null, both list files saved under the name 0x786de4. With +0x2b equal to 1: a local message (vtable 0x70061c) made and sent. The strings and containers destroyed:
// +0xf00; the deque emptied; the tree at +0xe60 + 0x54 emptied from its first node and its block freed; three members; the counted buffers +0xe70 and +0xe60; the lists +0x48, +0x30; +0x1c; +0xc.
void __fastcall ChatClient_Dtor(u8 *self);

// 0x4e5be0: a local message's check answers 0: +0x2b set. +0x260 formatted from the session's name 0x75c0cc and copied to +8. No session: one made (0xc bytes, the import 0x6fd474). The import 0x6fd498 connects to the host +0x1c (null: 0x6fe5b0) and port [0x7a1552] with the
// callback 0x4e61f0 and Global7cc53c_Pick7cc4f0; refused: OutputDebugString 0x75c0fc. Connected: the import 0x6fd494 logs in with two local strings, the session's other name and +8's text (null: 0x6fd6c4), and its +0x38, +0x34. Whatever came before, +0xef8 cleared. The message is never destroyed (sic).
void __fastcall ChatClient_Connect(u8 *self);

// 0x4e5ef0: the player made at the map's entrance (+4; 0x3a8 bytes, its name +8). Its thing a kind 0x12: placed at the cell's middle, the imports 0x6fd4cc and 0x6fd4bc on the session, +0xeb1 cleared. Mouse mode MMOnline set (the holder [0x7b05e8] made when missing), the imports 0x6fd4d4 (0xa00) and 0x6fd4c0.
void __fastcall ChatClient_EnterWorld(u8 *self);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[36], gChatClient_Connect_75c0fc, 0x0075c0fc, 0x24);
