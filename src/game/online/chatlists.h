// The chat object's two list files (names invented, unconfirmed): wide text, a line a name, in the online folder: the buddies (a tree at +0xe94, each loaded one ChatAvatar_SetBuddy) and the ignored (+0xea0 a set, its tree at +0xea4, each loaded one ChatAvatar_ToggleIgnore
// unless found, and the import 0x6fd484). Each builds its path the same way: the online folder's WString (+0xc the user's text, null: an empty string), a narrow copy of the name and a suffix that nothing reads, then sprintf. Reimplementation of 0x4ef010, 0x4ef3b0, 0x4ef880 and 0x4efc10;
// each original's SEH frame is dropped. The stream objects keep the original's layouts: an ofstream's filebuf at +4, ios at +0x58; an ifstream's filebuf at +8, ios at +0x5c; the filebuf's file at +0x50.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct ChatListsEnv {
    u8 *(__fastcall *singletonCtor)(u8 *self);                                              // 0x5c7420
    u32(__stdcall *onlineFolder)(u8 *out);                                                  // GamePaths_OnlineFolder
    u32(__cdecl *format)(char *out, u32 format, u32 text);                                  // 0x67a650
    u8 *(__fastcall *ostreamCtor)(u8 *self, void *, u8 *buffer, u32 isstd, u32 init, u32 mostDerived);  // BasicOstreamW_Ctor
    u8 *(__fastcall *istreamCtor)(u8 *self, void *, u8 *buffer, u32 isstd, u32 mostDerived);            // BasicIstreamW_Ctor
    u8 *(__fastcall *filebufCtor)(u8 *self, void *, u32 file);                              // WFileBuf_Ctor
    u8 *(__fastcall *filebufOpen)(u8 *self, void *, const char *path, u32 mode);            // 0x4f3920
    void(__fastcall *filebufInit)(u8 *self, void *, u32 file, s32 which);                   // WFileBuf_Init
    void(__fastcall *filebufRead)(u8 *self);                                                // 0x4f5ad0
    void(__fastcall *filebufDtor)(u8 *self);                                                // WFileBuf_Dtor
    void(__fastcall *setState)(u8 *ios, void *, u32 state, u32 reraise);                    // BasicIosW_SetState
    void(__fastcall *clear)(u8 *ios, void *, u32 state, u32 reraise);                       // BasicIosW_Clear
    void(__fastcall *ofstreamDtor)(u8 *ios);                                                // WOfstream_VbaseDtor
    void(__fastcall *iosBaseDtor)(u8 *ios);                                                 // IosBase_Dtor
    void(__fastcall *iosDtor)(u8 *ios);                                                     // BasicIosW_Dtor
    void(__fastcall *wstringDtor)(u8 *string);                                              // WString_Dtor
    u32(__cdecl *fclose)(u32 file);                                                         // CRT_fclose
    u32(__cdecl *fopen)(const char *path, u32 mode);                                        // Std_Fiopen
    u32(__cdecl *formatWide)(u16 *out, u32 format, u32 text);                               // CRT_swprintf
    u32(__cdecl *writeLine)(u8 *stream, const u16 *line);                                   // 0x4f6be0
    u8 **(__fastcall *shareLocale)(u8 *ios, void *, u8 **out);                              // Obj_ShareMember20
    u8 *(__cdecl *getFacet)(u8 **locale, u32 a, u32 b);                                     // 0x4f6a30
    void(__fastcall *releaseLocale)(u8 **ref);                                              // CountedRef_Release
    u32(__fastcall *widen)(u8 *ios, void *, u32 c);                                         // 0x4f1640
    u8 *(__fastcall *getline)(u8 *stream, void *, u16 *line, u32 size, u32 delimiter);      // 0x4f1ca0
    u32(__fastcall *setBuddy)(u8 *self, void *, u8 *name, u32 flags);                       // ChatAvatar_SetBuddy
    void(__fastcall *assign)(u8 *string, void *, const u16 *chars, u32 n);                  // 0x4051d0
    void(__fastcall *findIgnored)(u8 *set, void *, u32 *out, const u8 *key);                // ChatIgnoreSet_Find
    void(__fastcall *toggleIgnore)(u8 *self, void *, u8 *name);                             // ChatAvatar_ToggleIgnore
};
void ChatLists_SetEnv(const ChatListsEnv *env);

extern "C" {

// 0x4ef010 (thiscall (name), pops 4): the buddies written, each node's +0x10 (null: 0x700624) through swprintf ("%s"-like 0x75c6c8) and the stream. The open fails: state 2. No file then: all destroyed, 0. A failed fclose: state | 2. Returns 1.
u8 __fastcall ChatLists_SaveBuddies(u8 *self, void *, const char *name);
// 0x4ef880 (thiscall (name), pops 4): the same for the ignored (+0xea4, its own increment and suffixes).
u8 __fastcall ChatLists_SaveIgnored(u8 *self, void *, const char *name);
// 0x4ef3b0 (thiscall (name), pops 4): no +0: 0. The file (none yet) opened by Std_Fiopen for reading; each line got up to the facet's '\n' (at most 0x20 characters) while the stream is good: a wide string of it, ChatAvatar_SetBuddy (it, 0). Returns 1.
u8 __fastcall ChatLists_LoadBuddies(u8 *self, void *, const char *name);
// 0x4efc10 (thiscall (name), pops 4): no +0: 0. The file opened by the filebuf; each line: a wide string of it, a search of the set, not found: ChatAvatar_ToggleIgnore; another copy of it to the import 0x6fd484 (ECX: +0). Returns 1.
u8 __fastcall ChatLists_LoadIgnored(u8 *self, void *, const char *name);
// 0x67acd0: wcslen.
u32 __cdecl CRT_wcslen(const u16 *text);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u16[4], gChatlists_75c6c8, 0x0075c6c8, 0x8);
GAME_DATA(char[12], gChatLists_75c6d0, 0x0075c6d0, 0xc);
GAME_DATA(char[12], gChatLists_75c6e8, 0x0075c6e8, 0xc);
GAME_DATA(u8 *, gChatlists_f81ebc, 0x00f81ebc, 0x4);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u16[6], gText_ChatLists_75c6dc, 0x0075c6dc, 0xc);   // L".bud"
GAME_DATA(u16[6], gText_ChatLists_75c6f4, 0x0075c6f4, 0xc);   // L".ign"
GAME_DATA(const s32[2], Vbtable_700630, 0x00700630, 0x8);   // {0, 0x58}: the wide output file stream's
GAME_DATA(const s32[2], Vbtable_700640, 0x00700640, 0x8);   // {0, 0x5c}: the wide input file stream's
