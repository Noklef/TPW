// The scene's light sources (made by 0x457e70 by kind: 1 the ambient light, its colour 0x879dd8 = {1, r, g, b}; 2 and 3 with parts
// at +4 and +0xc in the renderer's tag 0x70 list): reimplementation of 0x458400.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x458400 (cdecl; light; name unconfirmed): by the light's kind (+0): 0 the M3D2 data error box ("Invalid lightsource creation
// attempted", 0x74d0dc); 1 the ambient colour 0x879dd8 reset to {1, 0, 0, 0}, then as 2; 2 the part at +4 (a virtual base: the pointer
// plus the offset at +4 of its vtable; 0 for none) taken out of the renderer's (0x87a6d0) tag 0x70 list (Renderer_RemoveFromSlot)
// and, when there is one, deleted (its slot 0 with 1), then as 3; 3 the same for the part at +0xc. Other kinds: nothing.
void __cdecl LightSource_Destroy(u8 *light);

// 0x458580 (cdecl; name unconfirmed): LightSource_Destroy of the light 0x7a0b40 points at (the one 0x4584d0 creates from the normalised direction at 0x7a0b30).
void __cdecl LightSource_DestroyMain();

// 0x458890 (cdecl; holder, r, g, b as float bits; name unconfirmed): the colour of the part *holder points at (a virtual base: the
// pointer plus the offset at +8 of its vtable): the scene's flags touched (SceneFlags_Touch of 0x8bd508); +4 = 1.0f, +8 / +0xc / +0x10
// = r, g, b; +0x54 = 0xff << 24 | __ftol(r 255) << 16 | __ftol(g 255) << 8 | __ftol(b 255) (ORed whole, after the shifts).
void __cdecl LightSource_SetColour(u8 *const *holder, u32 r, u32 g, u32 b);

// 0x4587e0 (cdecl; light, scale; name unconfirmed): LightSource_SetColour of light's object (+0) with its colour (+4 -> three floats)
// times scale, each product rounded to a float first (the bytes of +0x54 from those floats).
void __cdecl LightSource_SetColourScaled(u8 *const *light, float scale);

// 0x458350 (thiscall; a, r, g, b as float bits, pops 0x10; name unconfirmed): the same on the part itself, with alpha: +4 = a, +8 / +0xc /
// +0x10 = r, g, b; +0x54 = __ftol(a 255) << 24 | __ftol(r 255) << 16 | __ftol(g 255) << 8 | __ftol(b 255) (ORed whole, after the shifts).
void __fastcall LightPart_SetColour(u8 *part, void *, u32 a, u32 r, u32 g, u32 b);

// 0x458310 (thiscall; mostDerived, pops 4): the light part's constructor (0x457e70 builds parts with it), a class with a virtual base.
// When most derived: vtable 0x6fe570 and the part at +0x60 gets vtable 0x6fdce8. Always: the virtual base (at the vtable's +4) gets
// vtable 0x6fdcd8, the dword before it = its offset - 0x60, and +0x50 = 0. Returns the object.
u8 *__fastcall Obj_CtorVirtual(u8 *object, void *, u32 mostDerived);

// 0x4583d0 (thiscall; a, b, c, pops 12; 0x457e70 calls it on a new part): the scene flags touched (SceneFlags_Touch on 0x8bd508), then
// +0x2c = a, +0x30 = b, +0x34 = c. Returns a.
u32 __fastcall Obj_SetVectorAt2c(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x458910 / 0x458940 (cdecl; a light {object, ...}, as LightSource_SetColour's): flag 0x10000000 of the part the object's vtable (+8)
// reaches, at +0x50, cleared / set, the scene flags (0x8bd508) touched first.
void __cdecl MeshRef_ClearFlag10000000(u8 *const *ref);
void __cdecl MeshRef_SetFlag10000000(u8 *const *ref);

// 0x457e70 (cdecl; kind, colour (0xAARRGGBB), point, direction (three dwords each); name unconfirmed): a light {kind, part (kind 2),
// 0, part (kind 3), colour} in a zeroed GlobalMem block of 0x14 bytes. Kind 0: the M3D2 data error box ("Invalid lightsource creation
// attempted", 0x74d0dc) and 0 (the block kept by nobody, sic). Kind 1: the ambient colour 0x879dd8 = {1, r, g, b} (each byte times
// 0x6fe53c, 1/255). Kinds 2 and 3: a part of 0x68 bytes from Mem_Alloc built in place (2: the virtual base tables 0x6fdd00 / 0x6fdcf8,
// vtables 0x6fdce8, 0x6fdcd8 and 0x6fdcc8, +0x50 of the part 2, its vector (Obj_SetVectorAt2c) 0, 0, 1; 3: tables 0x6fe560 / 0x6fe558
// and Obj_CtorVirtual at +0xc, vtable 0x6fe548, +0x50 4, +0x44..+0x4c 100, 10000, 0.0001), coloured white (LightPart_SetColour), then
// given the colour (+0x54, and +4..+0x10 a, r, g, b as byte times 0x6fe50c times 0x6fe53c) and the direction (2: +0x2c) or the point
// (3: +0x14), and added to the renderer's (0x87a6d0) tag 0x70 list through its virtual base (Renderer_Add). No part (Mem_Alloc failing)
// faults (sic). Other kinds: only the kind and colour. Returns the light.
u32 *__cdecl LightSource_Create(u32 kind, u32 colour, const u32 *point, const u32 *direction);

// 0x4584d0 (cdecl; name unconfirmed): the direction at 0x7a0b30 normalised (each over the float root of x x + y y + z z), then the
// directional light (LightSource_Create kind 2) of the colour 0x7a0b3c in that direction, kept at 0x7a0b40, its part's flag 0x20 (at
// +0x50 of the part the vtable's +8 reaches) set.
void __cdecl LightSource_CreateMain();
}
