// The chat's name menus: the handler of their lists. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"

extern "C" {

// 0x4bede0 (cdecl; window, message, a, b; name invented, unconfirmed): the handler of the children 0x1789 / 0x3003 of the windows [0x7cc8c4] / [0x7cc8c8]. Message 0x100: the name is the window's (Window_DwordA0(0)); the chat [0x7cddec]
// (made when none: 0xf0c bytes, OnlineChat_Ctor) told it by the row a: 0x3004 and 0x178a blackmark, 0x3005 and 0x178b goto, 0x3006 and 0x178c ignore when b is 1 else unignore, 0x3007 add buddy, 0x178d remove buddy, 0x178e locate, 0x178f page;
// any other row none. Then the menu [0x7cc8cc] hidden and unlinked and the focus on [0x7cc8bc] (0). Every message then goes to Window_CallDefault, its answer returned.
s32 __cdecl ChatNameMenu_Handler(u8 *window, u32 message, u32 a, u32 b);
}

// The handler's calls (the bench's recorders; their own benches have them).
struct ChatNameMenuEnv {
    u32(__fastcall *dwordA0)(u8 *window, void *, u32 index);                     // Window_DwordA0
    u8 *(__fastcall *chatCtor)(u8 *chat);                                        // OnlineChat_Ctor
    void(__fastcall *blackmark)(u8 *avatar, void *, const StdWString *name);     // ChatAvatar_Blackmark
    void(__fastcall *page)(u8 *avatar, void *, const StdWString *name);          // ChatAvatar_Page
    void(__fastcall *locate)(u8 *avatar, void *, const StdWString *name);        // ChatAvatar_Locate
    void(__fastcall *gotoName)(u8 *avatar, void *, const StdWString *name);      // ChatAvatar_Goto
    void(__fastcall *addBuddy)(u8 *avatar, void *, const StdWString *name);      // ChatAvatar_AddBuddy
    void(__fastcall *removeBuddy)(u8 *avatar, void *, const StdWString *name);   // ChatAvatar_RemoveBuddy
    void(__fastcall *addIgnore)(u8 *avatar, void *, const StdWString *name);     // ChatAvatar_AddIgnore
    void(__fastcall *removeIgnore)(u8 *avatar, void *, const StdWString *name);  // ChatAvatar_RemoveIgnore
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                    // Window_SetShown
    u16(__fastcall *unlink)(u8 *window);                                         // Window_Unlink
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                        // Window_SetFocus
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b); // Window_CallDefault
};
void ChatNameMenu_SetEnv(const ChatNameMenuEnv *env);
