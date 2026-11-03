// Three online mouse mode handlers on the cell under the cursor (the word 0x7b05cc): reimplementation of 0x46c610, 0x46cee0 and 0x46cf80, thiscall with two arguments not used, popping 8; in the data at 0x6fe9e8, 0x6fe984 and 0x6fe994. Names
// invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default; the bench runs them as recorders).
struct ChatCursorEnv {
    u32(__cdecl *modifiers)();                                 // Wrap_KeyMap_AsyncModifiers_4815c0
    void(__cdecl *click)(s32 x, s32 y, u32 flags, u32 stored);  // GameMode_MapClick (0x524960)
    u8 *(__cdecl *chat)();                                     // OnlineChat_Instance (0x46cf10)
    u8(__fastcall *goTo)(u8 *chat, void *, u32 cell);          // ChatAvatar_GoToCell (0x4eac30)
    u8(__fastcall *moveTo)(u8 *chat, void *, u32 cell);        // ChatAvatar_MoveToCell (0x4eab40)
};
void ChatCursor_SetEnv(const ChatCursorEnv *env);

extern "C" {

// 0x46c610: GameMode_MapClick (column (id - 1) & 0x7f, row (id - 1) >> 7, the async modifiers, 1).
void __fastcall ChatCursor_Click(void *, void *, u32, u32);

// 0x46cee0 / 0x46cf80: ChatAvatar_GoToCell / ChatAvatar_MoveToCell of the online chat (OnlineChat_Instance) at the id; its answer. sic: the id's dword keeps the high half of the incoming ECX in the original (a word stored over its pushed ECX),
// and so here. Those callees read the word only.
u32 __fastcall ChatCursor_GoTo(void *ecx, void *, u32, u32);
u32 __fastcall ChatCursor_MoveTo(void *ecx, void *, u32, u32);
}
