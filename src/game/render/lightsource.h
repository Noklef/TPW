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
}
