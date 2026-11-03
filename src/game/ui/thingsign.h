// The thing panels' slot that puts the shown thing's sign over the panel's window. Reimplementation of 0x497060, 0x4997f0, 0x4ad7f0, 0x4afb70, 0x4b0fa0 and 0x4b64a0 (one vtable slot each, the code the same); the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls the bench records (the ports by default; their own benches have them).
struct ThingSignEnv {
    u8 *(__fastcall *def)(const u8 *object);                 // Object_GetDef
    void(__cdecl *sign)(const u8 *rect, u32 type, s32 row);  // Sign_PlaceOverRect
};
void ThingSign_SetEnv(const ThingSignEnv *env);

extern "C" {

// 0x4997f0 (thiscall): with a window +0x18 and a shown thing [0x7c2658]: Sign_PlaceOverRect over the window's saved rectangle, the type the thing's definition's +0x490, the row the window's depth +0xe0 + 10.
void __fastcall ThingSign_Show4997f0(u8 *self);
void __fastcall ThingSign_Show4ad7f0(u8 *self);  // 0x4ad7f0: the same
void __fastcall ThingSign_Show4afb70(u8 *self);  // 0x4afb70: the same
void __fastcall ThingSign_Show4b0fa0(u8 *self);  // 0x4b0fa0: the same
void __fastcall ThingSign_Show4b64a0(u8 *self);  // 0x4b64a0: the same
void __fastcall ThingSign_Show497060(u8 *self);  // 0x497060: the same with the row the depth + 1
}
