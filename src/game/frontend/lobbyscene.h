// The lobby's model owners: methods of three front-end classes that hold lobby models (src/game/frontend/lobbymodel, set up by
// RideUser_Acquire): reimplementation of 0x5da120, 0x5dfca0 and 0x5e1350 (thiscall). Named by their vtables until the classes are known.
#pragma once

#include "../../../re/types.h"

struct String;
struct StringView;

// Class702a50_Dtor's calls (the ports by default; the bench runs them as recorders).
struct LobbySceneEnv {
    void(__cdecl *destroy)(u32 ride);           // Ride_Destroy (0x463c80)
    void(__fastcall *stringDtor)(String *text);  // String_Dtor (0x5f61e0)
    // Class702cb0_Step's calls.
    u32(__fastcall *play)(const u8 *model, void *, u32 entry, u32 now);  // LobbyModel_Play (0x5d83f0)
    u32(__fastcall *timeLeft)(const u8 *model);                          // LobbyModel_TimeLeft (0x5d8440)
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt (0x51bfc0)
    int(__cdecl *rand)();                                                // the CRT's rand (0x67b5c0, by address)
    // 0x580320 (thiscall on the weather, unported: it reads the time stamp counter; by address): two points by value and two dwords.
    void(__fastcall *weatherBurst)(u8 *weather, void *, float x, float y, float z, float x2, float y2, float z2, u32 life, u32 kind);
    // Class702af0_Step's calls.
    u8 *(__fastcall *flightCtor)(u8 *self, void *, u8 *owner, u32 a, u32 b, const u32 *centre, const u32 *spread, u32 e, u32 f);  // LobbyFlight_Ctor (0x5d9830)
    void(__fastcall *acquire)(u8 *user);                                 // RideUser_Acquire (0x5d8130)
    void(__fastcall *flyerStep)(u8 *flyer);                              // LobbyFlyer_Step (0x5d9b50)
    // LobbyCamera_Frame's calls (the floats as their bits, as the original passes them).
    void(__fastcall *perspective)(void *matrix, void *, u32 near_, u32 far_, u32 fov);  // Mat4_Perspective (0x578be0)
    void(__fastcall *setFov)(void *quad, void *, u32 fov);               // ViewQuad_SetFov (0x56b790)
    void(__fastcall *lookAt)(u8 *camera);                                // Camera_SetLookAt (0x5d8de0)
    void(__cdecl *cameraUpdate)();                                       // Camera_Update (0x42b1c0)
    u32(__fastcall *fadeFog)(u8 *fade);                                  // Render_FadeFog (0x5d96c0)
    // Class702a60_Ctor's calls.
    u8 *(__fastcall *baseCtor)(u8 *object);                              // Class7029d0_Ctor (0x5d5700)
    String *(__fastcall *stringCtor)(String *s, void *, const char *text);   // String_CtorStr (0x5f60b0)
    void(__fastcall *joinPath)(u8 *paths, void *, const StringView *name);   // Strings5_JoinPath (0x5c5580)
    void(__cdecl *terrainLoad)(u32 a, u32 b);                            // Terrain_Load (0x4504c0, the game's)
    u32(__cdecl *removeFromRect)(u8 *object, u32 from, u32 to);          // Obj_RemoveFromRect (0x45be80)
    void(__cdecl *holderSleep)(u8 *holder);                              // Holder_Sleep (0x45bdd0)
    // The slots below.
    u32(__cdecl *applySign)(u32 id, u32 a1, u32 a2, u32 a3, u32 a4);     // Ride_ApplySign (0x468420)
    void(__cdecl *releaseCategories)();                                  // Sound_ReleaseLobbyCategories (0x51ea70)
    u32(__fastcall *freeTextures)(u8 *fx);                               // WeatherFx_FreeTextures (0x57d720)
    u32(__fastcall *reloadTextures)(u8 *fx);                             // WeatherFx_ReloadTextures (0x57d850)
};
void LobbyScene_SetEnv(const LobbySceneEnv *env);

extern "C" {
// 0x5d8470 (thiscall (a, b), pops 8; name invented, unconfirmed): Ride_ApplySign(+4, +0x14, +0x28, a, b), its answer.
u32 __fastcall RideUser_ApplySign(u8 *self, void *, u32 a, u32 b);
// 0x5e1340 (thiscall, self unused; name invented, unconfirmed): Sound_ReleaseLobbyCategories, then WeatherFx_FreeTextures of [0x8bcbac], its answer.
u32 __fastcall Class702cb0_FreeWeather(u8 *self);
// 0x5e0460 (thiscall, self unused; name invented, unconfirmed): WeatherFx_ReloadTextures of [0x8bcbac], its answer.
u32 __fastcall Class702cb0_ReloadWeather(u8 *self);
}

extern "C" {

// 0x5d80c0 (class 0x702a50's destructor, thiscall; the SEH frame left out; name unconfirmed): the vtable 0x702a50, the object taken out by the registry [0xf83774] (its slot 8, +0x20, with the object), Ride_Destroy (+4), the strings at
// +0x20 and +0xc destroyed.
void __fastcall Class702a50_Dtor(u8 *object);

// 0x5d80a0 (thiscall, pops 4; name unconfirmed): the destructor, the block freed when bit 0 of the flags is set; returns the object.
u8 *__fastcall Class702a50_DeletingDtor(u8 *object, void *, u32 flags);

// 0x5d84a0 (class 0x702a60's constructor, thiscall; the SEH frame left out; name invented, unconfirmed): the base (Class7029d0_Ctor), the vtable; the text at 0x774c64 joined
// to the paths at 0xf816c0; Terrain_Load(1, 0); while +4 is 1 the renderer's holder ([0x79fcb4]+8) taken out (Obj_RemoveFromRect over +0xc0 / +0xc4 when its flags (+4) have
// 0x4000, else Holder_Sleep). +8 and +0xc: blocks of 16 bytes with the vtables 0x702a58 and 0x702a54, the rest zero (null on a failed block).
u8 *__fastcall Class702a60_Ctor(u8 *object);

// 0x5da120 (class 0x702af0; name unconfirmed): once (the flag at +4 then set to 1), each item of the list at +0x58 (head +4; nodes {item, next})
// whose flag at +0xc is clear has its model at +8 set up (RideUser_Acquire) and the flag set to 1.
void __fastcall Class702af0_AcquireModels(u8 *object);

// 0x5da020 (class 0x702af0; the SEH frame left out; name invented, unconfirmed): with the count +0xc below the limit +8 (signed), flyers made: a block of 0x60, LobbyFlight_Ctor (+0x18, +0x2c, +0x38, +0x44, +0x50, +0x54), its
// model set up when the flag +4 is set and its +0xc clear (a failed block leaves a null flyer: a fault, sic), the count up; at the limit it stops, else the frame's seed +0x34 steps and another comes unless its bits 0-2 are 1.
// Then each flyer of the list +0x58 steps (LobbyFlyer_Step).
void __fastcall Class702af0_Step(u8 *object);

// 0x5da3a0 (thiscall; name invented, unconfirmed): each scene of the list at [+4]+4 (nodes {scene, next}; the next read before the call) steps (Class702af0_Step).
void __fastcall Class702af0_StepAll(u8 *all);

// 0x5d8cb0 (thiscall; name invented, unconfirmed): the perspective at 0x87b110 (0.1, 1000, the angle +4), ViewQuad_SetFov on [0x879d54], Camera_SetLookAt, Camera_Update, Render_FadeFog on the camera. The fade +0x5c moves an eighth
// (arithmetic) towards +0x60. Above 0: the render flag bit 0x80000 of 0x8bcbc8 set and 0x8bd4e4 the fade >> 8, at most 255, in its top byte. Otherwise that bit clear.
void __fastcall LobbyCamera_Frame(u8 *camera);

// 0x5dfca0 (class 0x702ca8; name unconfirmed): once (the flag at +0x10 then set to 1), the models at +4 and +8 set up (RideUser_Acquire).
void __fastcall Class702ca8_AcquireModels(u8 *object);

// 0x5e1350 (slot 6 of the vtable 0x702cb0; name unconfirmed): 0 when the state at +0x14 is 0; otherwise 1, the state cleared, after a pending play
// (state 2) is started: LobbyModel_Play(the model at [+0xc] + 8, entry 1, now 0).
u32 __fastcall Class702cb0_StartPendingPlay(u8 *object);

// 0x5e1210 (class 0x702cb0; its one call 0x5e079b; name unconfirmed): the lobby camera's step. The model at [+0xc]'s +0x3c triple is copied
// to the target (+0x4c) and the eye (+0x34); the eye is then moved by the angle +0x18 and radius +0x20 to (x + sin * r, y + height +0x24,
// z + cos * r). The position +0x28 eases toward the eye by the frame's scale ([0xf82884] + 8) times 0.1, the look-at +0x40 toward the target
// by it times 0.2 (each a + (b - a) * k, x87 in the original's order), and both go to the camera at [0xf83778] (+8 and +0x14).
void __fastcall Class702cb0_StepCamera(u8 *object);

// 0x5e0470 (class 0x702cb0, its callers 0x5e1831 and 0x5e22d3; name unconfirmed): the lobby's step. With a list ([+8]) of a count ([+8] + 0xc) not 0, a
// user slot chosen (UserSlots_Instance() + 0x60 not -1) and a model at +0xc, the camera follows that model by the state at +0x14:
// 0 turning at the rate in the scene's place ([0xf82884] + 0x10) + 8, 1 turning until it faces the model's +0x14 (then LobbyModel_Play, state 2),
// 2 closing in (at under 8 away the camera's +0x60 is 0x10000 and virtual slot 18 runs; before that +0x60 is the rest of the way, 0 to 65535);
// then Class702cb0_StepCamera, and +0x28 and +0x40 go to +0x78 and +0xb4. Otherwise the camera roams: +0x78 and +0xb4 each steer toward their
// targets +0x84 and +0xc0 (a new random +0x84 about +0x58 when it arrives), +0xc is the list's nearest model to +0x28 and +0xc0 its place
// (none: +0x58 with height 10), +0xac speeding up to +0xa8 and slowing near the target. Then, with a count over 0: the drop count of the
// weather [0x8bcbac] by the mean squared distance of the models flagged at +0x60 (1e17 over its 4th power, under 20 is 0, at most 400) and, with a
// model at +0xc: its mask (+0x5c) may give a burst (0x580320, WeatherFx_RequestReload, a sound), 1 in 16 draws plays its sound 3 to 5, its
// +0x64 triple goes to the camera's +0x50; a list's model with no time left plays again (rand() & 1).
void __fastcall Class702cb0_Step(u8 *object);

// 0x5d9db0 (class 0x702af0's constructor, thiscall; count, two names, two 12-byte triples by pointer, two dwords; pops 0x1c; the SEH frame left
// out; name unconfirmed): the strings at +0x10 and +0x24 constructed (String_Ctor), the vtable 0x702af0, a new 0x10-byte list (vtable 0x702aec,
// +4..+0xc cleared; null when the allocation failed) at +0x58, +4 = 0; +8 = count * the percentage at 0x7858dc / 100 (signed, the product
// wrapping), at least 1 and at most the count; +0xc = 0; the names assigned to the strings (their slot 5, +0x14); the triples copied to +0x38
// and +0x44, the dwords to +0x50 and +0x54; the object handed to the registry at [0xf83784] (its slot 6, +0x18). Returns the object.
u8 *__fastcall Class702af0_Ctor(u8 *object, void *, s32 count, const char *name, const char *name2, const u32 *first, const u32 *second, u32 a,
                                u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gLobbyscene_702c78, 0x00702c78, 0x4);
GAME_DATA(const float, gLobbyscene_702c7c, 0x00702c7c, 0x4);
GAME_DATA(const float, gLobbyscene_702c80, 0x00702c80, 0x4);
GAME_DATA(const float, gLobbyscene_702c84, 0x00702c84, 0x4);
GAME_DATA(const float, gLobbyscene_702c88, 0x00702c88, 0x4);
GAME_DATA(const float, gLobbyscene_702c8c, 0x00702c8c, 0x4);
GAME_DATA(const float, gLobbyscene_702c90, 0x00702c90, 0x4);
GAME_DATA(const float, gLobbyscene_702c94, 0x00702c94, 0x4);
GAME_DATA(const float, gLobbyscene_702c98, 0x00702c98, 0x4);
GAME_DATA(const float, gLobbyscene_702ca0, 0x00702ca0, 0x4);
GAME_DATA(u32, gClass702af0_Ctor_7858dc, 0x007858dc, 0x4);
GAME_DATA(u32, gLobbyscene_8bd4e4, 0x008bd4e4, 0x4);

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class702a50_vtable, 0x00702a50, 0x4);
GAME_DATA(void *[1], Class702a54_vtable, 0x00702a54, 0x4);
GAME_DATA(void *[1], Class702a58_vtable, 0x00702a58, 0x4);
GAME_DATA(void *[10], Class702a60_vtable, 0x00702a60, 0x28);
GAME_DATA(void *[1], Class702aec_vtable, 0x00702aec, 0x4);
GAME_DATA(void *[3], Class702af0_vtable, 0x00702af0, 0xc);
