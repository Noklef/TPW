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

// 0x503ac0 (group, pops 4; name unconfirmed; called by 0x4a9be4, which takes it from 100): with a current project (+8 not negative), its
// research left as a share of the dearest project's cost (+0x1464's mCost; its index not checked): (mCost - mResearchDone) * 100.0 /
// that cost, at the x87's precision, truncated (__ftol's low dword); 0 without a current project, unless its mCost is above 0.1 and the
// dearest cost above 1.0 (a NaN: 0).
u32 __fastcall ResearchLab_LeftPercent(const u8 *lab, void *, u32 group);

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

// 0x4ca630 / 0x4ca650 / 0x4ca670 / 0x4ca690 (cdecl; names unconfirmed): ResearchLab_CountNotDone of category 0 / 1 / 2 / 3 on the lab
// (GameTable7cfb90_Get1da722_519550 of the game object [0x80239c]).
u32 __cdecl Research_CountNotDoneCat0();
u32 __cdecl Research_CountNotDoneCat1();
u32 __cdecl Research_CountNotDoneCat2();
u32 __cdecl Research_CountNotDoneCat3();

// 0x5034f0 / 0x5036a0 (thiscall (group), pops 4; names unconfirmed): the group's current project moved to the next / previous one it can
// take up: of the group's category (mCat == group), its mResearchGroup at most the group's highestGroupAvailable (bytes, unsigned), not
// done; for the group 4 also the object control record (ObjectControls_Find, not quiet; none faults) of the id (the word +0x284 of the
// project's definition (ObjectDefs_Find), or mObjId when that is 0) with its byte +0x10 set, and that id a key of the global save info
// (GlobalSave_HasKey) when the id's definition has a positive +0xc4 (each looked up whatever the one before gave). The first such project
// after the current one (before it, going down from the last), else the first (last) before (after) it; the current one itself is not
// taken. None: a current project that is done is dropped (-1). (EAX is left as it happens; the callers 0x4aa0e9 and on ignore it.)
void __fastcall ResearchLab_NextProject(u8 *lab, void *, u32 group);
void __fastcall ResearchLab_PrevProject(u8 *lab, void *, u32 group);
// 0x503b80 (thiscall (cat), pops 4; name invented): raises the category's group's highestGroupAvailable (+0x1418 + cat * 16) while the
// share of its projects that are done, among the mNumProjects projects of the category whose mResearchGroup is at most it (done * 100 /
// count, unsigned; 0 for none), reaches the threshold of the level (the dwords at 0x785540, unsigned) and it is below 7. When it rose,
// with some project of the category not done (ResearchLab_CountNotDone) and the category 0-3, the advisor message 0x85 + cat (type 0, a
// 0, b -1, flag 0, no score; on the stack, bytes +2, +3, +0x17 unwritten) is posted to the advisor of the world [0x80239c].
void __fastcall ResearchLab_UpdateLevels(u8 *lab, void *, u32 cat);

// 0x503330 (thiscall; name invented; called by the lab's 0x502d60 after ResearchLab_FillProjects): for each group 0-4, its effort set to
// the dword at 0x785568 + group * 4, 100 at most (unsigned); the 5 groups' outputs recomputed (as ResearchLab_FillProjects's tail); the
// group's current project set to the first of the mNumProjects projects with mCat the group, mResearchGroup at most its
// highestGroupAvailable (bytes, unsigned) and not done (kept when there is none); then ResearchLab_UpdateLevels of the group.
void __fastcall ResearchLab_ApplyEfforts(u8 *lab);
// 0x502d60 (thiscall; name invented): the lab set up: no projects (+0x145c 0), the dearest project's index +0x1464 -1, the total output
// +0x1460 the setting at 0x785580; the byte +0x1468 whether there are any staff of kind 8 (Things_CountStaffOfKind; the game table entry of
// [0x80239c] looked up first, its answer unused); then ResearchLab_FillProjects, ResearchLab_ApplyEfforts and the lab added to the message
// centre's slot 0x11.
void __fastcall ResearchLab_Init(u8 *lab);

// 0x504090 / 0x504140 (thiscall; names invented, unconfirmed): the lab's flag +0x1468 set (from clear) or cleared (from set), and then a
// message 13 of 5 or 6 (ThingMsg13_Ctor) posted (Object_PostMessage: agent B's unported 0x40fb10 through the objects' env); nothing when
// it already was. 0x5041f0: the first when the game object's [0x80239c] word +0x1da742 is set, else the second.
void __fastcall ResearchLab_FlagOn(u8 *lab);
void __fastcall ResearchLab_FlagOff(u8 *lab);
void __fastcall ResearchLab_SyncFlag(u8 *lab);

// What the lab's message handler calls in the game's C library (the benches point it at a logging stub).
struct ResearchEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());   // crt_atexit (0x67a7d0)
};
// The calls go to env (nullptr: the game's).
void Research_SetEnv(const ResearchEnv *env);

// 0x503d40 (thiscall (message), pops 4): a message of type 0x11 (its vtable's slot 0; any other: nothing), the lab's slot in the message
// centre: the project after the definition of the message's id (+8; ObjectDefs_Find) set up past the projects (ResearchProject_SetNextUpgrade)
// and counted when it was; ResearchLab_UpdateLevels of the message's category (+0xc) and a message 12 of it (ThingMsg12_Ctor) posted
// (Object_PostMessage: agent B's unported 0x40fb10 through the objects' env). Then, when no project is left undone (each one's mResearchDone
// +0x10 at or above its mCost +0xc, the last first; a NaN is undone): the global save info's (GlobalSave_Instance, inlined) current theme
// marked mAllResearchCompleted; with the mode setting (0xfb3b7c, read the first time under the guard 0xfb34ac & 1 by Setting_ReadMode, its
// empty destructor 0x5d1750 given to atexit) 2, or 0 when every theme's is set (StdTreeF_AllFlagged on the info), the advisor message 0xa7
// (type 0, a 0, b -1, flag 0, no score; on the stack, its bytes +2, +3 and +0x17 never written) posted to the advisor of the world
// [0x80239c] (GameTable7cfb90_Get1da71c_5194d0) and the "End" object made (EndThing_Create).
void __fastcall ResearchLab_HandleMessage(u8 *lab, void *, u8 *message);
}
