// The research lab (thing model 14, serialized at 0x502dc0): reimplementation of 0x503850,
// 0x5039f0, 0x503b30, 0x504040, 0x5031a0 (thiscall on the lab), 0x504750 (on a group) and 0x504580 (on a project). The lab has 0x100
// projects of 20 bytes at +0xc (u16 mObjId +0, mCat +4, u8 mResearchGroup +8, u8 mUpgradeLevel +9,
// float mCost +0xc, float mResearchDone +0x10), 5 groups of 16 at +0x140c (effort +0, output +4,
// currentProject +8 (-1: none), u8 highestGroupAvailable +0xc) and mNumProjects at +0x145c.
// A project is done when mResearchDone >= mCost (a NaN either side: not done).
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x503850 (group): the group's effort.
u32 __fastcall ResearchLab_GroupEffort(const u8 *lab, void *, u32 group);

// 0x5039f0 (&out, group): the group's current project's mObjId to *out, 0 when it has none.
// Returns out.
u16 *__fastcall ResearchLab_CurrentObjId(const u8 *lab, void *, u16 *out, u32 group);

// 0x503b30 (cat): how many of the mNumProjects projects of the category (5: any) are not done.
u32 __fastcall ResearchLab_CountNotDone(const u8 *lab, void *, u32 cat);

// 0x504040 (group): whether the group has a current project and it is not done (a byte).
u8 __fastcall ResearchLab_CurrentNotDone(const u8 *lab, void *, u32 group);

// 0x503ff0 (thiscall; name unconfirmed): for each of the mNumProjects projects, the last first, the byte +0x10 of its object control
// record (ObjectControls_Find of its id in the game's table, not quiet) set to 1; a record not found faults (sic).
void __fastcall ResearchLab_FlagProjectControls(const u8 *lab);

// 0x504750: no current project (-1), highestGroupAvailable 1. Returns the group.
u8 *__fastcall ResearchGroup_Ctor(u8 *group);

// 0x504210 (thiscall; stream, save, version, unused): a project's mObjId, mCat, mResearchGroup, mUpgradeLevel, mCost and mResearchDone saved
// or loaded as a serializer does (Save_Field; the load path partly through the readers).
u32 __fastcall ResearchProject_Serialize(u8 *project, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x504760 (thiscall; stream, save, version, unused): a group's effort, output, currentProject and highestGroupAvailable saved or loaded as
// a serializer does (Save_Field).
u32 __fastcall ResearchGroup_Serialize(u8 *group, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x502dc0 (thiscall; the same arguments): the lab (thing model 14): the thing (Thing_Serialize), its 0x100 projects
// (ResearchProject_Serialize) and 5 groups (ResearchGroup_Serialize), 1 at the first that fails; then mNumProjects (+0x145c), mWorkLoad,
// mMostExpensiveProject (dwords) and mLabActive (the byte at +0x1468) as a serializer does (Save_Field).
u32 __fastcall ResearchLab_Serialize(u8 *lab, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x504580 (thiscall on a project; def, pops 4; name unconfirmed): for an object definition whose category (+0x17c) is not 4, its next
// upgrade: from the level after the one in its object control record (+0x14; ObjectControls_Find, not quiet, in the game's table) up to
// 2, the first level whose cost (the dword at +0x180 + level * 0x40) is above 0 makes the project (mObjId the definition's id, mCat 4,
// mResearchGroup 0, mUpgradeLevel the level, mCost the cost as a float, mResearchDone 0) and 1 is returned; each level passed over is
// written to the record's +0x14. 0 (AL) when none is left.
u8 __fastcall ResearchProject_SetNextUpgrade(u8 *project, void *, const u8 *def);

// 0x5031a0 (thiscall; name unconfirmed): the lab's projects from the object definitions (0x7890a0, First / Next): each definition with
// no dword at +0x508 and +0x4ac below 4 adds the project mNumProjects: with a cost (+0x180) the definition's own (its id, mCat +0x17c,
// mResearchGroup the byte +0x178, mUpgradeLevel 0, mCost the cost as a float, mResearchDone 0); without one its control record's byte
// +0x10 is set and its next upgrade (ResearchProject_SetNextUpgrade), when it has one. mMostExpensiveProject (+0x1464) becomes the new
// project when it is negative or its cost is not >= the new one's (NaNs too); mNumProjects counts it (no limit, sic). Then each group's
// output (+4) = its share of the 5 groups' summed effort (effort * 100 / sum, unsigned, 32-bit) times mWorkLoad (+0x1460) / 100 (unsigned,
// the product 32-bit), 0 for a sum of 0.
void __fastcall ResearchLab_FillProjects(u8 *lab);

// 0x4ca6b0 (cdecl; name unconfirmed): 1 when nothing is left to research: no project of the lab (GameTable7cfb90_Get1da722_519550 of the game
// object) not done (ResearchLab_CountNotDone of category 5 not above 0, signed), and each object definition (0x7890a0, First / Next) with
// +0x4ac not 4 has its control record's dword +0x18 (ObjectControls_FindUncached, not quiet, in the game's table) or its own +0xc4 set.
// Else 0 (in AL only).
u8 __cdecl Research_AllDone();
}
