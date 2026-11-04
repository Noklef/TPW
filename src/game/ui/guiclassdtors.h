// The GUI library's classes' destructors, each its vtable stored and its base's destructor run, and the deleting ones.
// Reimplementation of 0x65acd0, 0x65bc30, 0x66195e, 0x661e00, 0x66a255, 0x66a490, 0x66ae57, 0x66bf90, 0x66c82e, 0x66c940, 0x66caa7, 0x66d090, 0x66da21 and 0x66daf0. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the game's by default; the bench's recorders: their own benches have them).
struct GuiClassDtorsEnv {
    void(__fastcall *windowDtor)(u8 *self);     // Window_Dtor
    void(__fastcall *clearList)(u8 *self);      // Window705950_Clear
    void(__fastcall *rectDtor)(u8 *rect);       // GuiRect_Dtor
    void(__fastcall *cursorDtor)(u8 *self);     // Fn_6618a6: the base cursor's vtable
    void(__cdecl *memFree)(void *block);        // Mem_Free
};
void GuiClassDtors_SetEnv(const GuiClassDtorsEnv *env);

extern "C" {

// Thiscall: the vtable stored, then the base's: Window_Dtor (the list window: its nodes cleared and its rectangle +0x138 destroyed first), the cursor's Fn_6618a6.
void __fastcall Window705950_Dtor(u8 *self);   // 0x65acd0
void __fastcall SysCursor_Dtor(u8 *self);      // 0x66195e (vtable 0x7059d0)
void __fastcall Window705ac0_Dtor(u8 *self);   // 0x66a255
void __fastcall Window705b20_Dtor(u8 *self);   // 0x66ae57
void __fastcall Window705b98_Dtor(u8 *self);   // 0x66c82e
void __fastcall Window705bc8_Dtor(u8 *self);   // 0x66caa7
void __fastcall Window705c58_Dtor(u8 *self);   // 0x66da21

// Thiscall (flags), pops 4: as the destructor; with the flags' bit 0 the object freed (Mem_Free). Returns the object.
u8 *__fastcall Window705950_Delete(u8 *self, void *, u32 flags);   // 0x65bc30
u8 *__fastcall SysCursor_Delete(u8 *self, void *, u32 flags);      // 0x661e00
u8 *__fastcall Window705ac0_Delete(u8 *self, void *, u32 flags);   // 0x66a490
u8 *__fastcall Window705b20_Delete(u8 *self, void *, u32 flags);   // 0x66bf90
u8 *__fastcall Window705b98_Delete(u8 *self, void *, u32 flags);   // 0x66c940
u8 *__fastcall Window705bc8_Delete(u8 *self, void *, u32 flags);   // 0x66d090
u8 *__fastcall Window705c58_Delete(u8 *self, void *, u32 flags);   // 0x66daf0
}
