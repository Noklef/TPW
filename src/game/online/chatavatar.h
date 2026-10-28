// The player's avatar in an online chat room (the WEACHATR library's Chat): an object { +0 the Chat, +4 the park thing standing for the player,
// +0xeb1 a byte cleared when the player acts }. Reimplementation of the moves that tell the room (Chat::MovePlayer, import 0x6fd4cc, with a
// mode and the avatar as its context) and place the thing (Thing_ResetPlaceAt, only for a thing of kind 0x12): 0x4ea240, 0x4ea2d0, 0x4ea370, 0x4ea3f0,
// 0x4ea410, 0x4ebfb0, 0x4eab40 and 0x4eac30; and the players' lookup by name (0x4f7d70, 0x4ebf00). Names unconfirmed (the modes' meanings are not known).
#pragma once

#include "../../../re/types.h"

struct StdWString;

extern "C" {

// 0x4ea370 (thiscall; x, y, pops 8): Chat::MovePlayer(x, y as 16-bit values, mode 0, the avatar); then the thing in the thing table's slot
// (0x7cfb90) for the avatar's thing's handle, when of kind 0x12 (the byte +2), is placed at (x, y) (Thing_ResetPlaceAt).
void __fastcall ChatAvatar_MoveTo(u8 *avatar, void *, u32 x, u32 y);

// 0x4ea240 (thiscall; x, y, pops 8): the avatar's thing's scripts released (Person_ReleaseScripts); the slot thing of its handle, when of
// kind 0x12, placed at (x, y); then Chat::MovePlayer(x, y as 16-bit values, mode 1, the avatar).
void __fastcall ChatAvatar_Arrive(u8 *avatar, void *, u32 x, u32 y);

// 0x4ea2d0 (thiscall; x, y, pops 8; name unconfirmed): the avatar's thing's script placed at (0, 0, 0) with r15 0 at entry point 3
// (Person_PlaceScript), its register 8 set to 1.5 (SpriteScript_SetRegF); then ChatAvatar_MoveTo's moves with mode 1.
void __fastcall ChatAvatar_Enter(u8 *avatar, void *, u32 x, u32 y);

// 0x4ea3f0 (thiscall): Chat::MovePlayer(the avatar's thing's position: the words at +4 and +6, mode 2, the avatar). Returns its result.
u32 __fastcall ChatAvatar_SayPosition(u8 *avatar);

// 0x4ea410 (thiscall; x, y, pops 8): ChatAvatar_MoveTo's moves with mode 3.
void __fastcall ChatAvatar_MoveTo3(u8 *avatar, void *, u32 x, u32 y);

// 0x4eace0 (thiscall; x, y, pops 8; name unconfirmed): Chat::MovePlayer(x, y as 16-bit values, mode 1, the avatar) alone. Returns its result.
u32 __fastcall ChatAvatar_MovePlayer1(u8 *avatar, void *, u32 x, u32 y);

// 0x4eeb50 (thiscall; name unconfirmed): the avatar's wide text at +0xf00 (null: the empty string at 0x6fd6c4) copied (wcslen,
// StdWString_Grow(n, 1) when it gives room: the characters, a terminator and the length) into a std::wstring on the stack, which is told
// to Chat::Goto(const wstring &, void *context) (import 0x6fd490, __thiscall on the Chat at +0) with the avatar; then the string is
// released (_Tidy inlined: its block loses a count, or is freed when owned alone or frozen).
void __fastcall ChatAvatar_GotoRoom(u8 *avatar);

// 0x4e5210 (cdecl, no arguments; a button's callback, pushed by 0x4edb0f; name invented, unconfirmed): ChatAvatar_GotoRoom of the online
// chat (OnlineChat_Instance, made when missing). Returns 1.
u32 __cdecl ChatAvatar_GotoRoomCallback();

// 0x4ee6d0 (thiscall; name (a std::wstring), a dword not read, pops 8; name invented; called from 0x4e6d9c with the avatar's own +8): the
// name's characters (null: the empty string at 0x6fd6c4) added to the window [0x7cc8bc]'s name list (Window7cc8bc_ListAddStd, data 0), and
// the name inserted into the avatar's set of names at +0xe80 (tree D: StdMapD_Insert, the result not read).
void __fastcall ChatAvatar_AddName(u8 *avatar, void *, const StdWString *name, u32 unused);

// 0x4ebfb0 (thiscall, the object not used but for ECX's high word in x (sic); handle, cell, pops 8): the thing in the thing table's slot for
// the handle's low word, when of kind 0x12, placed at the centre of the map cell (1-based: column (cell - 1) & 0x7f, row (cell - 1) >> 7; x
// and y the column / row in the high byte and 0x80 in the low one, their high words ECX's and the cell's). Returns 1 when placed, else 0 (AL).
u8 __fastcall ChatAvatar_PlaceAtCell(u8 *self, void *, u32 handle, u32 cell);

// 0x4eab40 (thiscall; cell, pops 4): the slot thing of the avatar thing's handle, when of kind 0x12, placed at the centre of the 1-based map cell
// (as ChatAvatar_PlaceAtCell, the row from the cell's low word); then Chat::MovePlayer(that centre, mode 1, the avatar), the byte +0xeb1
// cleared and Chat::NotAFK(the avatar) (import 0x6fd4bc). Returns 1 when placed, else 0 (AL).
u8 __fastcall ChatAvatar_MoveToCell(u8 *avatar, void *, u32 cell);

// 0x4eac30 (thiscall; cell, pops 4): the byte +0xeb1 cleared and Chat::NotAFK(the avatar); then the slot thing of the avatar thing's handle,
// when of kind 0x12 and sent to the cell (Sprite_SetDestinationCellMarked), has Chat::MovePlayer told the cell's corner (column and row of the
// cell's low word less 1, shifted left 8; mode 0). Returns 1 when sent, else 0 (AL).
u8 __fastcall ChatAvatar_GoToCell(u8 *avatar, void *, u32 cell);

// 0x4f7c90 (thiscall on a chat player; called by 0x50b49c after Person_UpdateSpeedParticle; name invented, unconfirmed): with the walk mode
// +0x39c 1 or 2, one step (Sprite_WalkNav, through the env): arrived (0), the mode 0, the script reference's +0x10 3 (Set4_4217f0 on +0xc)
// and, with the byte +0x3a0 set, the online chat (OnlineChat_Instance) told the player's position (ChatAvatar_MovePlayer1 of the words +4
// and +6; the original's high halves are other registers', not read); stuck (2), the mode 3 and +0x10 5. Other modes: nothing.
void __fastcall ChatPlayer_StepWalk(u8 *player);

// What ChatPlayer_StepWalk calls through an environment (the fncmp driver's recorder; the game's by default, the port).
struct ChatAvatarEnv {
    u32(__fastcall *walkNav)(u8 *sprite);   // Sprite_WalkNav 0x4fa2a0
};
void ChatAvatar_SetEnv(const ChatAvatarEnv *env);   // nullptr: the game's

// 0x4f7d70 (thiscall on a thing of kind 0x12, a chat player; name, pops 4): whether the std::wstring's characters (null: the empty
// string 0x6fd6c4) equal the player's name, the wide characters at +0x19c (wcscmp 0x67c290 == 0, inlined here). Returns 1 or 0 (AL).
u8 __fastcall ChatPlayer_NameIs(const u8 *thing, void *, const u8 *name);

// 0x4ebf00 (stdcall, ECX not read; name, pops 4): the park's things walked newest first (World_FirstThing / World_NextThing on [0x7cf83c])
// for the first chat player (kind 0x12) whose name it is (ChatPlayer_NameIs). Returns it, or 0.
u8 *__stdcall ChatPlayers_FindByName(const u8 *name);
}
