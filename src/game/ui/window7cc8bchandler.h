// The handler of the chat window [0x7cc8bc]. The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the handler calls (the benches point these at recorders): the ports.
struct Window7cc8bcHandlerEnv {
    u8 *(__cdecl *chat)();                                                         // OnlineChat_Instance 0x46cf10
    void(__cdecl *pauseHide)();                                                    // GamePause_Hide 0x4a9380
    void(__fastcall *vote)(u8 *avatar);                                            // ChatAvatar_Vote 0x4eb300
    void(__fastcall *earmuffsOn)(u8 *avatar);                                      // ChatAvatar_EarmuffsOn 0x4ebee0
    void(__fastcall *earmuffsOff)(u8 *avatar);                                     // ChatAvatar_EarmuffsOff 0x4ebef0
    void(__fastcall *fillRooms)(u8 *avatar);                                       // ChatAvatar_FillRoomList 0x4ee8d0
    void(__fastcall *fillBuddies)(u8 *avatar);                                     // ChatAvatar_FillBuddyList 0x4ee9e0
    void(__fastcall *clear)(u8 *list);                                             // Window7059f0_Clear 0x6649d5
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault 0x65f66e
};
// The handler's calls go to env (nullptr: the defaults).
void Window7cc8bcHandler_SetEnv(const Window7cc8bcHandlerEnv *env);

// 0x4be820 (cdecl (window, message, a, b); name invented): 0x101: [0x7cc8d0] 0 for a 0xc3126, else 1; the name list (the child 0xc3123's child 0xc3124 of [0x7cc8bc], neither checked) cleared
// and filled with the rooms for 0xc3126, else the buddies. 0x100: a 0xc3128 the earmuffs on for b 1, else off; 0xc3129 the vote, then the window's child 0xc3129 (not checked) slot 5 (0);
// 0xc312a GamePause_Hide. Every message then goes to Window_CallDefault, its answer returned.
s32 __cdecl Window7cc8bc_Handler(u8 *window, u32 message, u32 a, u32 b);
}
