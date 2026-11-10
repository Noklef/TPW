// The lobby's models made: a resource found or loaded by name, a model cloned from it, the class with the two models of its character.
// Reimplementation of 0x5d7fd0, 0x5d8870 and 0x5dfa60 (thiscall). Names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default; the bench runs them as recorders).
struct LobbyMakeEnv {
    // The resource's load (0x4629d0, model/rideresource.h). name, pack, 0, 0, flags, id, then the caller's three and 0, 0.
    u32(__cdecl *load)(const char *name, const char *pack, u32, u32, u32 flags, s32 id, u32 a, u32 b, u32 c, u32, u32);
    u32(__cdecl *create)(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);  // Ride_Create (0x463060)
    void(__cdecl *sleep)(u8 *holder);                                  // Holder_Sleep (0x45bdd0)
    u32(__cdecl *removeRect)(u8 *object, u32 from, u32 to);            // Obj_RemoveFromRect (0x45be80)
    void(__fastcall *turnY)(const u8 *model, void *, float angle);     // LobbyModel_TurnY (0x5d8380)
    void(__fastcall *setPosition)(const u8 *model, void *, const float *v);  // LobbyModel_SetPosition (0x5d8190)
    s32(__cdecl *categories)(const char *name);                        // Sound_RegisterLobbyCategories (0x51e8f0)
    void(__fastcall *acquire)(u8 *user);                               // RideUser_Acquire (0x5d8130)
    void(__fastcall *alignTo)(const u8 *model, void *, const float *v);  // LobbyModel_AlignTo (0x5d81c0)
    void(__fastcall *setScale)(const u8 *model, void *, float x, float y, float z);  // LobbyModel_SetScale (0x5d83b0)
};
void LobbyMake_SetEnv(const LobbyMakeEnv *env);

extern "C" {

// 0x5d7fd0 (thiscall, pops 0x1c): a resource of 0x2c bytes: strings +4 = name and +0x18 = pack, +0 = the load's answer with flags 0xc0
// (0x40c0 when animated is not 0) and id 0x4650 + base (Int_Add). Then the scene ([0xf83774])'s slot 7 takes it. Returns it.
u8 *__fastcall LobbyResource_Ctor(u8 *resource, void *, const char *name, const char *pack, s32 base, u32 animated, u32 a, u32 b, u32 c);

// 0x5d8870 (thiscall on the scene, pops 0x18): the resource of that name and pack in the list at the scene's +8 (nodes from +4: the
// resource, then the next), both compared with stricmp, or a new one with the list's +0xc as the base. A model of 0x34 bytes: vtable
// 0x702a50, +4 = Ride_Create(the resource's +0, 0x101, zeros, ride -1), +8 = 0, strings +0xc = name and +0x20 = pack; the new ride's holder
// (+8 of its instance in the table 0x7a4610) leaves the renderer (by the rectangle +0xc0 / +0xc4 with flag 0x4000 at +4, else Holder_Sleep).
// [0xf83774]'s slot 6 takes it. Returns it, 0 when the block's allocation fails. A failed allocation of a new resource faults (sic).
u8 *__fastcall LobbyScene_MakeModel(u8 *scene, void *, const char *name, const char *pack, u32 animated, u32 a, u32 b, u32 c);

// 0x5dfa60 (thiscall, pops 0x30): the object of vtable 0x702ca8: strings +0x28 = title, wide +0x48 = text, models +4 (of name, model1)
// and +8 (of name2 ...), +0x24 = a1, +0x3c the point, +0x64, +0x68, +0x6c = three rand() & 0xff << 16, +0x70 = a9, +0x14 = [0x702c18] *
// [0x702c54] * degrees, to which both models are turned, and both at the point with y 0. +0x18 = the sound categories of its title,
// +0xc = owner, whose slot 12 takes it. Returns it.
// 0x5dc3e0 (thiscall, pops 8; name invented, unconfirmed): the marker of vtable 0x702bb8 for a record: +0xc = record, +4 = the model data\lobby\globe with pack "bullfrog" when the record's name
// (+0x14, wide) is "North Pole" with case folded, else "hpin" (a 1, the rest 0), set up (RideUser_Acquire), aligned to the unit vector of the record's +0x34 and put there; the record's int +0x40
// * 0.0005 + 0.02 kept to 0.02-0.1 its scale z, 0.035 + 0.2 * that its x and y. +8 = owner, whose slot 12 takes it. Returns it.
u8 *__fastcall Class702bb8_Ctor(u8 *self, void *, u8 *owner, const u8 *record);

// 0x5dfa60 (thiscall, pops 0x30) as above.
u8 *__fastcall Class702ca8_Ctor(u8 *self, void *, u8 *owner, u32 a1, const char *title_, const char *name, const char *pack, const char *pack2, float degrees, const u32 *point, const u16 *text, u32 a9, u32 b, u32 c);
}

#include "../core/gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class702bb8_vtable, 0x00702bb8, 0x4);
GAME_DATA(void *[1], Class702ca8_vtable, 0x00702ca8, 0x4);
