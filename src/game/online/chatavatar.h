// The player's avatar in an online chat room (the WEACHATR library's Chat): an object { +0 the Chat, +4 the park thing standing for the player,
// +0xeb1 a byte cleared when the player acts }. Reimplementation of the moves that tell the room (Chat::MovePlayer, import 0x6fd4cc, with a
// mode and the avatar as its context) and place the thing (Thing_ResetPlaceAt, only for a thing of kind 0x12): 0x4ea240, 0x4ea370, 0x4ea3f0,
// 0x4ea410, 0x4ebfb0, 0x4eab40 and 0x4eac30; and the players' lookup by name (0x4f7d70, 0x4ebf00). Names unconfirmed (the modes' meanings are not known).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4ea370 (thiscall; x, y, pops 8): Chat::MovePlayer(x, y as 16-bit values, mode 0, the avatar); then the thing in the thing table's slot
// (0x7cfb90) for the avatar's thing's handle, when of kind 0x12 (the byte +2), is placed at (x, y) (Thing_ResetPlaceAt).
void __fastcall ChatAvatar_MoveTo(u8 *avatar, void *, u32 x, u32 y);

// 0x4ea240 (thiscall; x, y, pops 8): the avatar's thing's scripts released (Person_ReleaseScripts); the slot thing of its handle, when of
// kind 0x12, placed at (x, y); then Chat::MovePlayer(x, y as 16-bit values, mode 1, the avatar).
void __fastcall ChatAvatar_Arrive(u8 *avatar, void *, u32 x, u32 y);

// 0x4ea3f0 (thiscall): Chat::MovePlayer(the avatar's thing's position: the words at +4 and +6, mode 2, the avatar). Returns its result.
u32 __fastcall ChatAvatar_SayPosition(u8 *avatar);

// 0x4ea410 (thiscall; x, y, pops 8): ChatAvatar_MoveTo's moves with mode 3.
void __fastcall ChatAvatar_MoveTo3(u8 *avatar, void *, u32 x, u32 y);

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

// 0x4f7d70 (thiscall on a thing of kind 0x12, a chat player; name, pops 4): whether the std::wstring's characters (null: the empty
// string 0x6fd6c4) equal the player's name, the wide characters at +0x19c (wcscmp 0x67c290 == 0, inlined here). Returns 1 or 0 (AL).
u8 __fastcall ChatPlayer_NameIs(const u8 *thing, void *, const u8 *name);

// 0x4ebf00 (stdcall, ECX not read; name, pops 4): the park's things walked newest first (World_FirstThing / World_NextThing on [0x7cf83c])
// for the first chat player (kind 0x12) whose name it is (ChatPlayer_NameIs). Returns it, or 0.
u8 *__stdcall ChatPlayers_FindByName(const u8 *name);
}
