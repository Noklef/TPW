// The staff pool's candidates (names invented, unconfirmed): the 0x20 entries of 0x14 bytes at the start of the world (StaffPoolEntry_Ctor,
// StaffPool_ValidEntry and the rest are in src/game/world/staff) as they are generated for hire.
#pragma once

#include "../../../re/types.h"

// 0x5083f0 (thiscall (name), pops 4): 1 when a used entry's name (+0xa set; StaffPoolEntry_FormatName into 0x7cee18, 0x80) or a staff member's
// in the world's thing list (Thing_IsStaff, Staff_GetName) is the name (wcscmp), else 0. Only AL is meant.
u8 __fastcall StaffPool_NameTaken(u8 *pool, void *, const u16 *name);

// 0x507580 (thiscall): up to 15 tries: for a kind +0 of 0-4 the text id +4 = World_Rand % 0x23 (another kind keeps it, with the original's
// debug print left out), the name formatted into 0x7cef20 (0x80), until StaffPool_NameTaken of the world answers no.
void __fastcall StaffPoolEntry_PickName(u8 *entry);

// 0x507600 (thiscall (kind, slot), pops 8): a new candidate of a kind: +0 the kind; the kinds 0-4 the sprite +9 from the folder 5, 6, 4, 7, 8
// (SpriteFolder_RandomSprite) and, a World_Rand wasted, the grade World_Rand % 3 + the kind's average grade [0x785298 + 4 * kind] - 1 in a byte
// (a byte above 4, so also the -1, is 4); another kind only the kind's low byte as the grade (the original's debug print left out). The grade
// +8, the name (StaffPoolEntry_PickName), +0xa = 1, the staff dialog told (0x481550 on 0x7b51f0, slot), +0xc the tick (Out_SetFromGame1da70c),
// +0x10 the timeout t = [0x78530c] plus World_Rand % (t >> 1, an arithmetic shift, the division unsigned).
void __fastcall StaffPoolEntry_Generate(u8 *entry, void *, u32 kind, u32 slot);

// 0x508330 (thiscall (kind), pops 4): unless the flag +0x299 + kind is set, the first entry unused (+0xa 0) generated for the kind: 1; else 0. Only AL is meant.
u8 __fastcall StaffPool_AddCandidate(u8 *pool, void *, u32 kind);

// 0x5087d0 (thiscall): once (the flag +0x298, then set; 0 when it was): for the kinds 0-4 the counts [0x7852ac] (dwords, signed) times the same (inlined). Only AL
// is meant: 1.
u8 __fastcall StaffPool_FillAll(u8 *pool);

// 0x507bb0 (thiscall; name invented, unconfirmed): StaffPool_FillAll, then the tick into +0x294 (Out_SetFromGame1da70c).
void __fastcall StaffPool_Start(u8 *pool);

// 0x508170 (thiscall; name invented, unconfirmed): round the kinds 1, 0, 2, 3, 4 until no kind is short: a kind whose used entries plus its dword +0x280 are under its limit [0x7852d4]
// (both signed) gets a candidate (as StaffPool_AddCandidate). A short kind asks for another round even when it was not added: with its flag set or the pool full it never ends (the original's).
void __fastcall StaffPool_TopUp(u8 *pool);

// 0x507bf0 (thiscall (slot), pops 4; name invented, unconfirmed): the candidate at slot (under 0x20, in use: else 0) taken up: of kinds 0-4, a mouse mode made for
// placing the staff (Mem_Alloc(0x1c); MMPlaceStaff_Ctor with 5, 4, 6, 7, 8, bytes +8, +9, dword +4; none when that failed), the holder [0x7b05e8] made when missing
// (inlined; a failed one 0) and the mode set (MouseMode_Set). +0xb = 1, Staff_CountTypes; each candidate in use whose kind's flag (+0x299 + kind) is set and with no
// +0xb goes out of use (the staff dialog told: 0x481550 on 0x7b51f0, slot). 1.
u8 __fastcall StaffPool_TakeCandidate(u8 *pool, void *, u32 slot);

// 0x5084f0 (thiscall; name invented, unconfirmed): each candidate in use, older than 4 * +0x10 ticks (Game1da70c_MinusAt0 of +0xc) and with no +0xb, goes out of use (dialog
// told). Then, the clock +0x294 older than 4 * [0x785304]: up to [0x785308] new candidates (no more than the free entries; Staff_CountTypes first). Each kind 0-4 wants
// [0x7852c0 + kind * 4] less those in use, 0 with its flag +0x299 + kind; each new one rolls World_Rand % the total wanted through the wants (signed), the kind it lands on,
// or the one before (at first, sic, the address of +0x294 as a kind: a wild read in both) when the wants are negative enough to pass them all. A kind with its flag set, no
// free entry or no wants left ends it; else StaffPoolEntry_Generate in the first free entry and one less wanted. Then StaffPool_TopUp and +0x294 = the tick.
void __fastcall StaffPool_Update(u8 *pool);

// What it calls (the fncmp driver's recorders; the game's by default).
struct StaffCandidateEnv {
    void(__fastcall *tellDialog)(void *dialog, void *, u32 slot);   // 0x481550, agent C's, on 0x7b51f0
};
void StaffCandidate_SetEnv(const StaffCandidateEnv *env);   // nullptr: the game's

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gStaffPool_Update_785304, 0x00785304, 0x4);
GAME_DATA(u32, gStaffPool_Update_785308, 0x00785308, 0x4);
GAME_DATA(s32, gStaffPoolEntry_Generate_78530c, 0x0078530c, 0x4);
