// The options' changes applied and the first settings loaded: reimplementation of 0x4237f0, 0x424bb0 and 0x449df0. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorders): everything but the strings.
void OptionsChanges_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x4237f0 (thiscall, plain RET): Options_NoteChanges, then by the level +0x94: 1: with no session ([0xf82884] 0) the save 0x78dcf0 saved (UserSaves_SaveB), bit 0 of +0x98: Sound_Post700b5c, the camera's level
// (+0x30, Sound_SetCameraLevel), the session looked at afresh and the save loaded back (UserSaves_LoadB); 2: text 0x193 (Charset_FormatWide of [0x78ba94], 0xff units) in a Dialog_Box of two buttons
// (KeyAction_Clear78d96c 0x424c10, Options_ConfirmSave 0x424bb0), which keeps the window open. Then Options_ApplySound; the save folder (GamePaths_SaveFolder, with UserSlots_Instance in ECX) and the wide strings 0x78e6f8,
// 0x78e230, made narrow, opened (FileStream_Open, mode 0xc0000010): the tag 1 written, in full: Options_SerializeDisplay (saving, the tag); closed. Then with +0x3a Camera_SnapYaw; Sound_ApplySettings; the window closed.
void __fastcall Options_ApplyChanges(u8 *options);

// 0x424bb0 (cdecl; the dialog's button): Global879088_Set_5508b0(2), the dword 0x786b80 = 1. A session: its +0x14 = 3. None: the save 0x78da20 saved (UserSaves_SaveA), 0x786b80 = 1 again. The window closed. Returns 1.
u32 __cdecl Options_ConfirmSave();

// 0x449df0 (cdecl): [0x7a1a14] = 0, [0x7a1a8c] = 0xc0e15; Options_LoadMiscFile of 0x78d8d8 with the path 0x74ce84: a 1 sets [0x78d968]. With [0x78d968]: [0x7a1a8c] = [0x78d950], [0x79d86c] = [0x78d954],
// GameOwner_SelectMode([0x78d95c]), [0x79d9c8] = [0x78d964].
void __cdecl Options_LoadStartup();
}
