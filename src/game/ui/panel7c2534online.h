// The game's menu, the rows object at [0x7c2534]: the handler of its online rows, and the answers of the menu's question boxes. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct String;
struct WString;

// The calls (the ports by default, the unported by address; the bench's recorders: its own bench has them).
struct Panel7c2534OnlineEnv {
    void(__fastcall *controls)(u8 *object);                                     // 0x409300
    u32(__cdecl *f662420)();                                                    // 0x662420
    void(__fastcall *hide)(u8 *object);                                         // WindowAt10_Hide
    s32(__fastcall *tell)(u8 *object);                                          // WindowAt10_Message4IfAny
    u32(__cdecl *setGlobal)(u32 value);                                         // Global879088_Set_5508b0
    WString *(__fastcall *sessionName)(u8 *session, void *, WString *out);      // 0x5b8160
    s32(__cdecl *assign)(String *to, const WString *from);                      // String_AssignFromWide
    void(__fastcall *save)(u8 *object, void *, u32 name, void *buffer, u32 flag);   // SaveGame_Save 0x414920
    void(__fastcall *words)(u8 *game, void *, u16 *table);                      // 0x51ac20
    void(__fastcall *setNameWords)(u8 *object, void *, u32 flag, u8 *text, const u16 *table);   // Obj_SetNameAndWords
    void(__fastcall *forgetSlots)(u8 *slots);                                   // 0x5c8650
    void(__cdecl *openSaves)();                                                 // Dialog7cb3f8_Open
    u32(__cdecl *setMode3)();                                                   // Global786b80_SetAndMode3
    void(__fastcall *start)(u8 *session, void *, u32 callback, u32 value);      // 0x5b5cc0
    void(__fastcall *leave)(u8 *session);                                       // 0x5b5e40
    u32(__fastcall *hasActivity)(const u8 *session);                            // Obj_HasActivity
    void(__cdecl *options)();                                                   // Dialog7cb2fc_Open
    u16 *(__cdecl *text)(u32 index);                                            // Language_WideText
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    void(__fastcall *logOff)(u8 *session);                                      // 0x5b6070
    s32(__fastcall *findKey)(const u8 *map, void *, u32 key, u32 modifiers);    // KeyMap_FindKey
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);              // Class702da0_Push
    s32(__cdecl *handler)(u8 *window, u32 message, u32 a, u32 b);               // WindowAt10_Handler
};
void Panel7c2534Online_SetEnv(const Panel7c2534OnlineEnv *env);   // nullptr: the game's

extern "C" {

// Closing means as in panel7c2534handler.h (the object told and freed through Mem_Free).
// 0x48bd40 (cdecl; window, message, a, b). Message 0x1000b: a = 0x1b, or a key the key map [0x78721c] does not know (the byte +2 of b its modifiers; AX tested): closes, 0 returned. Message 0x100001 by a: 9 closes, the session's 0x5b5cc0(0x5da3c0, 0), 0 returned.
// 10 closes, the session's 0x5b5e40, then the first of [[0xf85614] + 0xc], [[0xf85614] + 8] whose slot 9 answers not 0 gets slot 8 (neither: 0 returned at once), [[0xf85614] + 4] slot 7, the push (0x258, 1) on [[0xf85614] + 0x14], 0 returned. 11, with no activity in the
// session: closes, Dialog7cb2fc_Open, 0 returned. 12 and 15: the question box 0xe, callback 0x48bc50, and 9, callback 0x48bc30. 13 closes, the session's 0x5b6070, [[0xf85614] + 0x10] slot 11, 0 returned. 14 closes, 0 returned. Any other, and after a box: WindowAt10_Handler, its answer returned.
s32 __cdecl Panel7c2534_OnlineHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x48b400 (cdecl, no arguments): the session's name (0x5b8160) into the String 0xf82838, its WString destroyed; SaveGame_Save on 0x78a450 ([0xf82840], a local buffer, 1); Global879088_Set_5508b0(2); 0x51ac20 on [0x7cf83c] fills a table of 0x100 words; the session's Obj_SetNameAndWords(1, the dword
// +8 of 0x786b68, the table); closes. Answer 1. Each session through Online_Instance.
u32 __cdecl Panel7c2534_AnswerSave();

// 0x48b5b0, 0x48b620 (cdecl, no arguments): Global879088_Set_5508b0(1), closes. Answer 1. (The second also calls 0x5da3c0, which only returns.)
u32 __cdecl Panel7c2534_AnswerQuit();
u32 __cdecl Panel7c2534_AnswerQuit2();

// 0x48bc30 (cdecl, no arguments): Global786b80_SetAndMode3. Answer 1.
u32 __cdecl Panel7c2534_AnswerMode3();

// 0x48bc50 (cdecl, no arguments): UserSlots_Instance's 0x5c8650, Dialog7cb3f8_Open, closes. Answer 1.
u32 __cdecl Panel7c2534_AnswerSaves();

// 0x48b310 (cdecl, no arguments; name invented, unconfirmed): the box 9's callback (rows 8, 0x14, 0x19): the session's 0x5b6070 (Online_Instance),
// Global879088_Set_5508b0(3), closes. Answer 1.
u32 __cdecl Panel7c2534_AnswerLogOff();
}
