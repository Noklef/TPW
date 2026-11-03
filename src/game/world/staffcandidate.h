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

// What it calls (the fncmp driver's recorders; the game's by default).
struct StaffCandidateEnv {
    void(__fastcall *tellDialog)(void *dialog, void *, u32 slot);   // 0x481550, agent C's, on 0x7b51f0
};
void StaffCandidate_SetEnv(const StaffCandidateEnv *env);   // nullptr: the game's
