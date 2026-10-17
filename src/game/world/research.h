// The research lab (thing model 14, serialized at 0x502dc0): reimplementation of 0x503850,
// 0x5039f0, 0x503b30, 0x504040 (thiscall on the lab) and 0x504750 (on a group). The lab has 0x100
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
}
