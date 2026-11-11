// Front-end scene objects entering: slot 7 of the vtables 0x702bc8 and 0x702dd0 (sceneleave.h has their leaving). Thiscall; named by their vtables until the classes are known (unconfirmed).
// Slot 9 (+0x24) answers whether the object is active.
#pragma once

#include "../../../re/types.h"

// Their calls go to env (nullptr: the ports).
struct SceneEnterEnv {
    void(__fastcall *setDrops)(u8 *fx, void *, u32 count);                         // WeatherFx_SetDropCount (0x57d910)
    void(__fastcall *fogOff)(void *view);                                          // Render_FogOff (0x5d9690)
    void(__fastcall *ambient)(void *view, void *, u32 on, u32 colour);             // Render_SetAmbient (0x5d9230)
    void(__fastcall *light)(u8 *slot, void *, u32 on, const u32 *colour, const u32 *direction, u32 value);  // LightSlot_Set (0x5d92e0)
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt (0x51bfc0)
    u8 *(__fastcall *makeModel)(u8 *scene, void *, const char *name, const char *pack, u32 animated, u32 a, u32 b, u32 c);  // LobbyScene_MakeModel (0x5d8870)
    void(__fastcall *acquire)(u8 *user);                                           // RideUser_Acquire (0x5d8130)
    void(__fastcall *resetConnections)(u8 *session);                               // Online_ResetConnections (0x5b6070)
    void(__fastcall *start30)(u8 *list, void *, u32 callback, u32 user);           // ListRequest_Start30 (0x5bc7d0)
    void(__fastcall *clearQueue)(u8 *object);                                      // Class702da0_Clear (0x5d6060)
    void(__cdecl *createDialog)();                                                 // Dialog7cc47c_Create (0x4b8520)
    void(__fastcall *holderAcquire)(u8 *user);                                     // HolderUser_Acquire (0x5d87d0)
    void(__fastcall *fogOn)(void *view);                                           // Render_FogOn (0x5d9680)
    void(__fastcall *setFog)(void *view, void *, u32 colour, u32 start, u32 end);  // Render_SetFog (0x5d96a0)
    void(__fastcall *start66)(u8 *list, void *, u32 value, u32 callback, u32 user);  // ListRequest_Start66 (0x5bcd10)
    void(__cdecl *apply)();                                                        // Sound_ApplySettings (0x51bd70)
    void(__cdecl *createButtons)(u32 flag);                                        // Dialog7cdb0c_Create (0x4c4fb0)
    void(__cdecl *createThings)();                                                 // Dialog7cc4c4_Create (0x4b9a70)
};
void SceneEnter_SetEnv(const SceneEnterEnv *env);

extern "C" {

// 0x5dcfe0 (name invented): unless active: +4 = 1, +0x44 to +0x50 and +0x58 and +0x68 = 0, +0x54 = [[[0xf82884] + 0x10] + 0x14], the view [0xf83778]'s +4 = 60.0, no drops ([0x8bcbac]), no fog, +0x10 = the three
// at +0x1c, bit 0x2000000 of [0x8bcbc8], ambient 0x202020, a light (1, white, no direction, 1), +0x34 = +0x38, the view's +0x28 = it, +0x2c = minus it, +0x30 = 0, [[0xf85614] + 0x14]'s +0x18 = 0, the view's
// +0x50 to +0x58 = 0xf30000, 0xcb0000, 0xbf0000, sounds 2 and 3 of [0x803a48] at +0x74, +0x78. +0xc = a new part of 0x18 bytes (vtable 0x702bb4; +4 its model set up, +8 = this; slot 13 given it) or 0. The
// session's connections reset (Online_Instance), its list +0xe0 asked (ListRequest_Start30 with 0x5dcfc0 and this).
void __fastcall ViewCtl702bc8_Enter(u8 *self);

// 0x5dd650 / 0x5dd910 (classes 0x702e70 / 0x702e20; names invented): unless active: Dialog7cdb0c_Create(0 / 1), Dialog7cc4c4_Create, ViewCtl702bc8_Enter, Class702da0_Clear of [[0xf85614] + 0x14].
void __fastcall ViewCtl702e70_Enter(u8 *self);
void __fastcall ViewCtl702e20_Enter(u8 *self);

// 0x5e1380 (name invented): unless active: +4 = 1, HolderUser_Acquire of [0xf83774], ambient 0x444444, a light (1, 0xeeeeee, no direction, 1), the view's +0x28 = 2.0, +0x2c = 8.0, +0x30 = 1,
// +4 = [[[0xf82884] + 0x10] + 4], fog on (0xff44ddff, 50.0, 250.0), +0xc, +0x10, +0x14 = 0, the connections reset.
void __fastcall Class702cb0_Enter(u8 *self);

// 0x5e23f0 (name invented): unless active: the queue [[0xf85614] + 0x14] cleared, Dialog7cc47c_Create, slot 10, Class702cb0_Enter's work (inlined), +0xe4 = 0, the session's list +0xe4 asked
// (ListRequest_Start66 with [[0xf82888] + 8], 0x5e23c0, this), and with a current item (+0xc) its sounds played as 0x5e14a0's, Sound_ApplySettings.
void __fastcall Scene702dd0_Enter(u8 *self);
}

#include "../core/gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class702bb4_vtable, 0x00702bb4, 0x4);

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x14], gText_Sceneenter_774c80, 0x00774c80, 0x14);
GAME_DATA(u8[0x20], gSceneenter_774ca8, 0x00774ca8, 0x20);
