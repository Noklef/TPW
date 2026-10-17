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
}
