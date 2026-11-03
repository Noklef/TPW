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

// What it calls (the fncmp driver's recorders; the game's by default).
struct StaffCandidateEnv {
    void(__fastcall *tellDialog)(void *dialog, void *, u32 slot);   // 0x481550, agent C's, on 0x7b51f0
};
void StaffCandidate_SetEnv(const StaffCandidateEnv *env);   // nullptr: the game's
