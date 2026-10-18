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
}
