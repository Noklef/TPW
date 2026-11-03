// The world's start (reimplementation of 0x5156a0; the name is invented, unconfirmed): the balance files, the random seed, the ten single things, the park's gates and lights.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct WorldStartEnv {
    void(__fastcall *objectControlsInit)(u8 *table);                                    // ObjectControls_Init
    void(__fastcall *actionRestart)(u8 *record);                                        // ActionRec_Restart
    u8 *(__fastcall *settingReadMode)(u8 *setting);                                     // Setting_ReadMode
    s32(__cdecl *atexit)(void *exit);                                                   // CRT_atexit
    u32(__fastcall *balanceLoad)(u8 *file, void *, const char *path, u32 keep);         // BalanceFile_Load
    u32(__cdecl *time)(void *out);                                                      // 0x67d3f0 (the CRT's time)
    void(__fastcall *initBlock)(u8 *world);                                             // World_InitBlock
    void(__fastcall *prepare)(u8 *world);                                               // 0x4d7af0 (agent C's)
    u8 *(__fastcall *construct[10])(u8 *block);                                         // the ten things' constructors, in kThings' order
    u8 *(__fastcall *constructObject)(u8 *block, void *, u32 cell, u32 id, s32 angle, u32 ride);   // Object_Construct
    void(__fastcall *researchInit)(u8 *lab);                                            // ResearchLab_Init
    void(__cdecl *mathTables)();                                                        // 0x544360 (agent B's)
    void(__cdecl *setPathBehaviours)();                                                 // Steering_SetPathBehaviours
    void(__stdcall *vectorCtor)(u8 *array, u32 size, s32 count, void *ctor, void *dtor);   // CRT_VectorCtorIterator
    u32(__fastcall *post)(u8 *centre, void *, u8 *msg);                                 // MessageCentre_Post
};

void WorldStart_SetEnv(const WorldStartEnv *env);

extern "C" {
// 0x5156a0 (thiscall on the world (level), pops 4; an SEH frame, dropped; the empty Debug_Printf left out; name invented, unconfirmed): +0x1da738 = 1 for its time. ObjectControls_Init on [0x7cf83c] +
// 0x1d943c, ActionRec_Restart on 0x785a08, the 8 dwords 0x802080 zeroed, the setting 0xfb3b7c read once (guard bit 0 of 0xfb34a9, 0x5d1750 at exit). Setting 1: the balance file 0x785040 loads
// "data\levels\online_Standard.sam" (keep 0), then level\online_Standard.sam (keep 1). Else the same without online_; with setting 2, then level\[0x7cf4a0]Standard.sam (keep 1), whose failure counts for nothing.
// Then +0x1da70c = 0, +0x1da708 = time(0), World_InitBlock on +0x2d8, 0x4d7af0 on the world, +0x1da714 = 0. The ten things (kThings): each allocated and constructed, a failed block a null thing, whose handle
// read faults (sic); each handle goes to its word. The words +0x1da72a, c, e, +0x1da730 = 0. The objects "gates" ([0x8021b0]) and "lights" ([0x7cf938]), by name among the object definitions (wcsicmp, +0x390):
// a new object (0x450 bytes; cell 1, the definition's id), its handle in +0x1da732 and +0x1da734; none, the word 0. ResearchLab_Init on the thing of +0x1da722, 0x544360, Steering_SetPathBehaviours.
// Message 13 of 0 posted to the message centre [0x788d3c], made first when null (0x244 bytes: 29 of 20 bytes, 0x405460 and 0x40f970; a failed block stays null). +0x1da738 = 0. Returns 0 when one of the first two loads failed, else 1.
u32 __fastcall World_Start(u8 *world, void *, const char *level);
}
