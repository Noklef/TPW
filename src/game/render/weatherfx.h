// The weather renderer's small setters, wind, lightning midpoints, texture release and deletion: reimplementation of 0x57d910, 0x57d920,
// 0x57e4b0, 0x57dc40, 0x57dc70, 0x57d720 and 0x57d1a0.
//
// The renderer is the object at [0x8bcbac] (allocated at 0x57d120, built by 0x57d1d0). Its +0 is
// the kinds of weather showing (bit 1 and bit 2; the constructor sets 1), +4 asks for its
// textures to be reloaded (cleared by the loader, 0x57d850), +0x3318 is how many drops fall
// (read by the drawing, 0x57f02b), and +0x3338 / +0x333c are the fall speeds of kinds 1 and 2
// (5.0 and 0.7 from the constructor; 0x580113 picks one by bit 1).
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

extern "C" {

// 0x57d910 (thiscall; count): +0x3318 = count (Weather_Update clamps it to [0, [0x785530]]).
void __fastcall WeatherFx_SetDropCount(u8 *fx, void *, u32 count);

// 0x57dc40 (thiscall; speed, a float moved as a dword): the fall speed of each kind showing
// (+0x3338 when bit 1, +0x333c when bit 2).
void __fastcall WeatherFx_SetFallSpeed(u8 *fx, void *, u32 speed);

// 0x57dc70 (thiscall): +4 = 1, the textures to be reloaded.
void __fastcall WeatherFx_RequestReload(u8 *fx);

// 0x57d720 (thiscall; name unconfirmed): the three texture arrays at +0x3348, +0x334c and +0x3350, each when set: its first texture
// released (Texture_Release), the array destroyed when still set (Texture_DestroyArray), the field cleared. Returns 1.
u32 __fastcall WeatherFx_FreeTextures(u8 *fx);

// 0x57d7f0 (thiscall (which, name), pops 8; name unconfirmed): slot 1 the textures at +0x3350, 2 +0x3348, 3 +0x334c, any other
// value taken as the slot's address itself (sic). An empty slot is loaded by the texture manager at [0x87a268] (TexManager_Load)
// and, got, its flags given 0x80200. Returns whether the slot is set.
u32 __fastcall WeatherFx_LoadTexture(u8 *fx, void *, u32 which, const char *name);

// 0x5508c0 (cdecl; name unconfirmed): with a renderer at [0x8bcbac], its three textures loaded (WeatherFx_LoadTexture, the renderer read
// again for each): "Data\Generic\Weather\snow.tga" into slot 2, "raindrop.tga" into 1, "lightning.tga" into 3, each name through
// Quickload_Locate(name, 1, 0) (the copy at 0x79c9d0 if the file opens, else 0). Returns the last load's answer (EAX as the original
// leaves it), or 0 without a renderer.
u32 __cdecl WeatherFx_LoadTextures();

// 0x550950 (cdecl, no arguments; name unconfirmed): the frame's effect textures again. A local folder handle (vtable 0x6fd9e8). With a renderer at [0x8bcbac], its three
// textures loaded as WeatherFx_LoadTextures does; with the queue [0x8c47a0], SpriteQueue_LoadTexture of the name at 0x765238. The handle opened on the data folder (the
// path at +8 of MemorySource_GetLock(0xf816c0)); on success the first file of the pattern [0x878ec0] asked for into a local entry (+0x20 -> its name at +0x24, emptied),
// and when that fails Handle_Vfn8 of the pattern. ColourRamps_SetFromPalette, then the handle destroyed.
void __cdecl WeatherFx_ReloadEffects();

// 0x57d850 (thiscall; name unconfirmed): WeatherFx_LoadTextures on this renderer (the same three names and slots, each name through
// Quickload_Locate whether or not its slot is set), then its +4 (the reload asked for) cleared. Returns 1.
u32 __fastcall WeatherFx_ReloadTextures(u8 *fx);

// 0x57d1a0 (cdecl; name unconfirmed): with a renderer at [0x8bcbac], its textures freed (WeatherFx_FreeTextures), it freed (GameHeap_Free)
// and [0x8bcbac] cleared: 1. Without one 0.
u32 __cdecl WeatherFx_Delete();

// 0x57d920 (thiscall; angle in degrees, strength: floats; pops 8; name unconfirmed; the game passes 0 and 0.3 at 0x572aa3): x87 in
// the original's order. +0x331c = the direction (x, -1, z) made unit length (unless its length is 0 or a NaN: then as it is), x and z
// cos and sin of the angle times the strength. Then the landscape's points 0-3 (Landscape_SetPoint on 0x8c1828) at (sin a * s, -1,
// cos b * s), s the strength * 0.0017 and a, b the angle + 270 less 0 and 0 (point 0), 1.9 and 2.3, 1.2 and -1.7, -1.8 and -2.9 degrees,
// and +0x331c again from point 3's x and z * 0.2.
void __fastcall WeatherFx_SetWind(u8 *fx, void *, float angle, float strength);

// 0x57e4b0 (cdecl; out, a, b: three floats each, scale: a float; returns out; name unconfirmed: the lightning's subdivision 0x57e810
// passes its +0x3310 as the scale): x87 in the original's order. The midpoint m = (a + b) * 0.5 and d = b - a; q = |m|^2. Five
// numbers from the renderer's ([0x8bcbac]) random seed at +0xa198 (each seed = rol(rol(seed * 0x13d573, 3) + 0xe0423, 4), inline in
// the original): r = (s1 % 0x1fff / 8192)^2 * (s2 % 0x1fff / 8192) (signed remainders), negated when s3 is odd; s4 unused; s5 % 3
// (signed) picks the axis: p = m with p[i] = m[i] - q / d[i] for the first of the axes (0, 1, 2), (1, 2, 0) or (2, 0, 1) (-1 and -2 the
// last) whose d is not 0 (nor a NaN), the last one regardless. p is scaled to |d| * scale * r long; out = p + m + n * (0 / |n|), n the
// cross product of p and d (so 0 * n, sic: a NaN when n is 0 or not finite).
float *__cdecl WeatherFx_BoltMidpoint(float *out, const float *a, const float *b, float scale);

// 0x57e810 (thiscall (a, b), pops 8; name invented, unconfirmed): one lightning segment subdivided at WeatherFx_BoltMidpoint(b, a, +0x3310), x87 in the original's
// order, its random numbers from the seed of [0x8bcbac] (not always this renderer). A draw & 0x1ff below 9 with +0xa188 below +0xa1a0 forks: +0xa188 up by one, a side point
// from a along (b - a) made unit length (unless 0 or a NaN) and the plane normal +0x3020 + [+0x9da0] * 0x30, its ribbon's +0x9da4 entry cleared, recursed (side, a), added to the
// ribbon (Ribbon_AddPoint(none, side)), +0xa188 down again. Each half then ends in Ribbon_AddPoint when the midpoint is within the wobbled limit (+0xa180 + +0xa184 * a draw
// in [-1, 1)) of its end (squared), else is recursed: (m, b), then (a, m). Returns 1, or 0 at once when a call answers 0.
u32 __fastcall WeatherFx_BoltSubdivide(u8 *fx, void *, const float *a, const float *b);

// What WeatherFx_DrawBolt calls (the port by default; its bench runs them as recorders).
struct WeatherDrawEnv {
    u8 *(__fastcall *begin)(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra);  // Batch_Begin
    void(__fastcall *transform)(u8 *object, void *, u32 arg);                                            // Batch_TransformVertices
    u32(__fastcall *close)(u8 *object, void *, float depth);                                             // Batch_Close
    u32(__fastcall *subdivide)(u8 *fx, void *, const float *a, const float *b);                          // WeatherFx_BoltSubdivide
    u32(__fastcall *addPoint)(u8 *object, void *, const float *from, const float *to);                   // Ribbon_AddPoint
};
void WeatherFx_SetDrawEnv(const WeatherDrawEnv *env);  // nullptr: the port's

// 0x57ec50 (thiscall (a, b), pops 8; name invented, unconfirmed; x87 in the original's order): one lightning bolt from a to b into a batch. The texture at +0x334c with an id (+8 bit 0x1000,
// +0x1c): +0xa170, +0xa174 its offsets +0x28, +0x2c and +0xa178, +0xa17c its scales +0x20, +0x24 plus them; else those four 0. Batch_Begin on [0x879ab0] (+0x330c, 4, 0x1c4, 0x200d52, the id):
// none gives 1. +0xa188 and +0x9da4 cleared, WeatherFx_BoltSubdivide(a, b), Ribbon_AddPoint(none, a): either's 0 gives 0. Then Batch_TransformVertices(0) and Batch_Close(0), whose answer is returned.
u32 __fastcall WeatherFx_DrawBolt(u8 *fx, void *, const float *a, const float *b);

// What WeatherFx_DrawDrops calls (the port by default; its bench runs them as recorders).
struct WeatherDropsEnv {
    u8 *(__fastcall *begin)(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra);       // Batch_Begin
    u32(__fastcall *addPolygon)(u8 *object, void *, u8 *vertices, u32 count, u32 stride, const u8 *codes, u32 flags);  // DrawBatch_AddPolygon
    void(__fastcall *transform)(u8 *object, void *, u32 arg);                                                 // Batch_TransformVertices
    u32(__fastcall *close)(u8 *object, void *, float depth);                                                  // Batch_Close
};
void WeatherFx_SetDropsEnv(const WeatherDropsEnv *env);  // nullptr: the port's

// 0x57f490 (thiscall (x, y, z: floats), pops 12; name invented, unconfirmed; x87 in the original's order): the falling drops of +0x3318 at their places +8 (0xc each) less the camera (x, -y..., z),
// each wrapped into [-4, 4] by 8, through the transformer ([0x87b1d8] slot 1: the matrix +0x9d58, in place, 0xc, 0xc, the count, 2), then a triangle for each drop in front (z > 0): the texture at +0x3348
// with an id (+8 bit 0x1000, +0x1c) gives the UVs (+0x28, +0x2c, scales +0x20, +0x24), else 0; the size [0x8bcbcc] * +0x3354 across, +0x3354 high; the second dword of each vertex's colour left unset, sic. With
// [0x8bcbc8] bit 0x100 the places swirl: two gusts from phases +0x3558, +0x355c and sixteen orbits from the first sixteen phases, all through the 128 cubic rows at +0x8558 (+0xa18c, +0xa190, +0xa194 their sizes,
// +0x9d98 the time), and each phase stepped; without it each place is shaken by three rand() draws. No check of the count against the 0x400 places on the stack, as in the original. Returns 1 when the batch
// does not begin, 0 when a triangle is refused, else Batch_Close's answer.
u32 __fastcall WeatherFx_DrawDrops(u8 *fx, void *, float x, float y, float z);

// 0x57efd0 (thiscall (x, y, z: floats), pops 12; name invented, unconfirmed; x87 in the original's order): the drops of +0x3318 as streaks: from each place +8 less the camera, wrapped as the drops', to that
// plus +0x331c.. times +0x3314. All through the transformer (slot 1, the matrix +0x9d58, in place, 0xc, 0xc, twice the count, 2), then a quad for each streak in front (z > 0), a tenth of its length wide:
// the texture at +0x3350 with an id (+8 bit 0x1000, +0x1c) gives the UVs; colours the fog's +0x2c masked by (6 - z) * 12 in each byte and, at the near end alpha 0x80, at the far end 0xff. Batch_Begin on
// [0x879ab0] (+0x330c, 4, 0x1c4, 0x200d22 with [0x8bcbc8] bit 2, else 0x200d52, the id). The second dword of each vertex's colour left unset, sic; no check of the count against the 0x400 streaks on the
// stack, as in the original. Returns 1 when the batch does not begin, 0 when a quad is refused, else Batch_Close's answer; the batch's +0x30 cleared before the close.
u32 __fastcall WeatherFx_DrawStreaks(u8 *fx, void *, float x, float y, float z);
}

struct Handle;

// WeatherFx_ReloadEffects's calls (the bench's recorders; null: the game's).
struct WeatherFxReloadEnv {
    char *(__cdecl *locate)(const char *name, u32 flags, s32 *index);
    u32(__fastcall *load)(u8 *fx, void *, u32 which, const char *name);
    u32(__fastcall *queueLoad)(u8 *queue, void *, const char *name);
    u8 *(__fastcall *getLock)(u8 *source);
    Handle *(__fastcall *ctor)(Handle *h);
    s32 *(__fastcall *open)(Handle *h, void *, s32 *result, u32 path);
    s32 *(__fastcall *first)(Handle *h, void *, s32 *result, u32 pattern, u32 entry);
    s32 *(__fastcall *close)(Handle *h, void *, s32 *result, u32 pattern);
    void(__fastcall *dtor)(Handle *h);
    void(__cdecl *setRamps)();
};
void WeatherFx_SetReloadEnv(const WeatherFxReloadEnv *env);

// (Test seam: the fncmp driver's table replaces the calls by address of WeatherFx_DrawBolts and WeatherFx_AddBolt.)
void WeatherFx_SetBoltCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x57ee30 (thiscall; name invented, unconfirmed; x87 in the original's order): with +4 set Batch_DrawScreenQuad first; its 0 gives 0. The bolts of the list at +0x3008 (0x30 bytes each, the count at +0x3308), last first: +0x24 less +0x9d9c. At 0 or below the
// bolt goes, the last one copied over it. Else +0x9da0 its index. With its +0x2c set: +0xa198 that, +0xa1a0 4, and +0xa19c a shade that fades over each 500 of +0x24, plus rand() & 0x3f, in each channel (sic: a shade over 0xff runs into the next channel). Without: +0xa198 Cpu_Ticks, +0xa19c white, +0xa1a0 0.
// WeatherFx_DrawBolt(its +0xc, it): 0 gives 0. Returns 1.
u32 __fastcall WeatherFx_DrawBolts(u8 *fx);

// 0x580320 (thiscall (start, end by value, life, flash), pops 0x20; name invented, unconfirmed; x87 in the original's order): with fewer than 16 bolts a new one: +0 the start, +0xc the end, +0x18 start less end at unit length (not when its length is 0 or a NaN),
// +0x24 the life, +0x2c 0 or, with flash, Cpu_Ticks | 1. Returns 1 either way.
u32 __fastcall WeatherFx_AddBolt(u8 *fx, void *, u32 ax, u32 ay, u32 az, u32 bx, u32 by, u32 bz, u32 life, u32 flash);

// 0x57ff60 (thiscall; name invented, unconfirmed; x87 in the original's order): the clock's time into +0x3344, the time since the last (at most 500, signed) into +0x9d9c, times 0.001 (0 with the pause bit 4 of [0x8bd508]) into +0x9d98; +0x330c 2;
// WeatherFx_DrawBolts: 0 gives 0. With drops (+0x3318): the view's offset from the camera at [0x87afe8] (+0x30.. times +0x3340, or 0.3 with [0x790ab0] & 0x16; y negated) in fixed point; +0x9d58 the matrix at [0x87b154], its translation cleared; the
// place +0x3328.. moved by the wind +0x331c.. times the speed (+0x3338 with +0 bit 0, else +0x333c) and the time, wrapped into -4..4; +0xa1bc.. the camera's +0x20..; +0xa1c8 0; DrawStreaks with bit 0, else DrawDrops with bit 1 (offset plus place): 0 gives 0. Returns 1.
u32 __fastcall WeatherFx_Frame(u8 *fx);
}
