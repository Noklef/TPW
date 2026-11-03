// The handyman's, mechanic's, entertainer's and guard's constructors: reimplementation of 0x4d6b60, 0x4d9eb0, 0x4d9db0, 0x4d4340, 0x4d4250, 0x4d5d40 and 0x4d5de0. The names are invented. (The original's assertion of the name's range, 0x5da3c0, does nothing and is left out.)
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default; the bench's recorders: their own benches have them).
struct StaffCtorsEnv {
    u8 *(__fastcall *typed)(u8 *staff, void *, u32 cell, u32 kind, u32 type, u32 unused);   // Staff_CtorTyped
    u8 *(__fastcall *plain)(u8 *staff, void *, u32 cell, u32 kind);                         // Staff_Ctor
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);                 // Charset_FormatWide
    void(__fastcall *addHandyman)(u8 *world, void *, u8 *thing);                            // World_AddHandyman
    void(__fastcall *addMechanic)(u8 *world, void *, u8 *thing);                            // World_AddMechanic
    void(__fastcall *addEntertainer)(u8 *world, void *, u8 *thing);                         // World_AddEntertainer
    void(__fastcall *mechanicState)(u8 *mechanic, void *, u32 state);                       // Mechanic_SetState
    void(__fastcall *spread)(u8 *world, void *, u32 index, u32 cell);                       // Map_SpreadAdd
    u8(__fastcall *addThing)(u8 *centre, void *, const u8 *thing, s32 slot);                // MessageCentre_AddThing
    void(__fastcall *handymanWork)(u8 *handyman);                                           // Handyman_PickWork
    void(__fastcall *mechanicWork)(u8 *mechanic);                                           // Mechanic_PickWork
    void(__fastcall *entertainerWork)(u8 *entertainer);                                     // Entertainer_PickWork
    void(__fastcall *addGuard)(u8 *world, void *, u8 *thing);                               // World_AddGuard
    u32(__cdecl *randomSprite)(u32 folder);                                                 // SpriteFolder_RandomSprite
    u32(__fastcall *busy)(u8 *staff);                                                       // 0x506a40
    u32(__fastcall *randomDest)(u8 *person);                                                // 0x4f9490
    void(__fastcall *guardState)(u8 *guard, void *, u32 state);                             // Guard_SetState
};
void StaffCtors_SetEnv(const StaffCtorsEnv *env);

extern "C" {

// 0x4d6b60 (thiscall (cell, type, sprite, name), pops 0x10): Staff_CtorTyped of kind 5, the name (Charset_FormatWide of the charset [0x78baa8], entry name) into +0x1a0, the word +0x210 cleared, World_AddHandyman, +0x214, the words +0x218 and +0x21a cleared,
// folder +0x24 5, sprite +0x20 the low byte, +0xc started at entry 3 (ScriptRef_Start), slot 0xa of the message centre (made on the first call, inlined), Handyman_PickWork. Returns it.
u8 *__fastcall Handyman_Ctor(u8 *handyman, void *, u32 cell, u32 type, u32 sprite, u32 name);

// 0x4d9eb0 (thiscall (cell, type, sprite, name), pops 0x10): as Handyman_Ctor with kind 4, the charset [0x78baac] and World_AddMechanic; folder 6, the word +0x218 and +0x214 cleared, Mechanic_SetState 0, then slot 0xa and Mechanic_PickWork.
u8 *__fastcall Mechanic_Ctor(u8 *mechanic, void *, u32 cell, u32 type, u32 sprite, u32 name);

// 0x4d9db0 (thiscall (cell), pops 4): Staff_Ctor of kind 4; then as Mechanic_Ctor from the word +0x210 on, the sprite the byte [0x7cdbec].
u8 *__fastcall Mechanic_CtorLoad(u8 *mechanic, void *, u32 cell);

// 0x4d4340 (thiscall (cell, type, sprite, name), pops 0x10): as Handyman_Ctor with kind 6, the charset [0x78bab0] and World_AddEntertainer; folder 4, the speed word +0xc0 the word [0x75c7f8], the spread record 0 at his cell (word (+7 << 7) + +5 + 1; Map_SpreadAdd on [0x8023a0];
// the upper half the stack's in the original), +0x214 cleared, Entertainer_PickWork; no message centre.
u8 *__fastcall Entertainer_Ctor(u8 *entertainer, void *, u32 cell, u32 type, u32 sprite, u32 name);

// 0x4d4250 (thiscall (cell), pops 4): Staff_Ctor of kind 6; then as Entertainer_Ctor from the word +0x210 on, the sprite the byte [0x7cdb60]; last [0x7cdb60] = ([0x7cdb60] + 1) % 3 (signed).
u8 *__fastcall Entertainer_CtorLoad(u8 *entertainer, void *, u32 cell);

// 0x4d5d40 (thiscall (cell, type, sprite, name), pops 0x10): Staff_CtorTyped of kind 7, the name of the charset [0x78bab4], Guard_Init.
u8 *__fastcall Guard_Ctor(u8 *guard, void *, u32 cell, u32 type, u32 sprite, u32 name);

// 0x4d5de0 (thiscall): the word +0x210 cleared, World_AddGuard, folder +0x24 7, sprite +0x20 a random one (SpriteFolder_RandomSprite), +0xc started at entry 3, the spread record 3 at his cell (as the entertainer's), the word +0x212 and +0x214 cleared, slot 0x1b of the message centre.
// Then, unless 0x506a40 answers: with the ticks & 3 and 0x4f9490 answering, state 1 (Guard_SetState), else 0.
void __fastcall Guard_Init(u8 *guard);
}
