// The x87 control word guard (0x62f980-0x62f9d4, thiscall; destructors 0x5f9fa0-0x5f9ff1): the
// bits it changed are restored when it goes. The game never sets the changed masks (nothing
// writes them but the constructors), so the restore only ever zeroes them.
//
// Inline x87 assembly: 32-bit builds only.
#pragma once

#include "../../../re/types.h"

struct FpuControl {   // vtable 0x7035d8 (the base's 0x7035dc), one slot: the deleting destructor
    void *vtable;
    u16 saved;        // +4: what the changed bits go back to
    u16 changed[2];   // +6, +8: the changed bits
};
static_assert(sizeof(FpuControl) == 0xc, "FpuControl is 0xc bytes");

extern "C" {
// 0x62f980 (control word): cw with the changed bits put back to saved. The original's upper 16
// bits are stale register halves (only FLDCW reads the result); these are cw's.
u32 __fastcall FpuControl_Merge(const FpuControl *f, void *edx, u32 cw);
// 0x62f9a0: when any bits changed, loads the merged control word; then forgets the changes.
void __fastcall FpuControl_Restore(FpuControl *f);
void __fastcall FpuControl_Dtor(FpuControl *f);  // 0x5f9fc0: the base vtable, Restore
// 0x5f9fa0 / 0x5f9fd0 (flags): the deleting destructors (the derived and the base vtable's).
FpuControl *__fastcall FpuControl_DeletingDtor(FpuControl *f, void *edx, u32 flags);
FpuControl *__fastcall FpuControl_BaseDeletingDtor(FpuControl *f, void *edx, u32 flags);
}

struct FpuEnv {
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
    void *const *vtable;               // 0x7035d8
    void *const *baseVtable;           // 0x7035dc
};
// The calls go through env (nullptr: the game's).
void Fpu_SetEnv(const FpuEnv *env);
const FpuEnv *Fpu_GetEnv();
