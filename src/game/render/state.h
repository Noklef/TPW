// Small render state setters: reimplementation of 0x5836b0 and 0x5d9680-0x5d96a0.
//
// 0x8bcc40 is the display device's generation, bumped each time the device is restored
// (0x583730, 0x583790). 0x8bcbc8 is the engine's render flags (0x2000: fog), and the fog's
// colour, start and end are 0x8bd47c, 0x8bd490 and 0x8bd494 (the vertex fog, 0x5747e0; the lobby's
// globe sets 0xff44ddff, 50 and 250, 0x5e1407).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5836b0 (thiscall on a flags pair, the scene's at 0x8bd508): flag 0x40000000 set and +4 =
// the device generation (0x8bcc40).
void __fastcall SceneFlags_Touch(u32 *flags);

// 0x5d9680 / 0x5d9690 (thiscall, the object unused): fog (0x8bcbc8 flag 0x2000) on / off.
void __fastcall Render_FogOn(void *);
void __fastcall Render_FogOff(void *);

// 0x5d96a0 (thiscall, the object unused; colour, start, end): the fog's settings.
void __fastcall Render_SetFog(void *, void *, u32 colour, u32 start, u32 end);
}
