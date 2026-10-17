// The blending canvas (0x6173e0-0x6175b3): a surface, a clip rectangle and the primitive driver
// for its depth, one of four static CanvasDrivers (0xfa7e3c 8 bits, 0xfa7e38 16, 0xfa7e34 24,
// 0xfa7e30 32; vtables 0x703650/0x703680/0x7036b0/0x7036e0, set by the static initialisers
// 0x617370-0x6173d0). Unlike the DrawContext primitives these take the surface, clip and flags
// as arguments and blend by the colour's alpha through the format's channel masks.
//
// The originals' quirks are kept:
// - SetSurface flags a change when the masks, depth or palettedness differ from the old surface
//   and never clears it; the clip becomes the whole surface. A paletted surface, or a depth
//   other than 8/16/24/32, gets no driver.
// - The constructor zeroes the surface's bits a second time (after SurfaceInfo_Ctor).
#pragma once

#include "../../../re/types.h"
#include "../math/rect.h"
#include "surface.h"

struct CanvasDriver {
    void *const *table;
};

struct Canvas {
    CanvasDriver *driver;  // the depth's, or null
    u32 flags;             // +4: 8/0x10 blend (0x10 inverting the alpha)
    SurfaceInfo target;    // +8
    SurfaceInfo format;    // +0x30: a copy of the target
    Rect clip;             // +0x58
    u8 unknown68[0x800];
    u32 changed;  // +0x868
};
static_assert(sizeof(Canvas) == 0x86c, "Canvas is 0x86c bytes");

struct CanvasEnv {
    SurfaceInfo *(__fastcall *infoCtor)(SurfaceInfo *);  // SurfaceInfo_Ctor 0x5fa680
    CanvasDriver *drivers[4];                            // 8, 16, 24, 32 bits: 0xfa7e3c, 0xfa7e38, 0xfa7e34, 0xfa7e30
    void *const *tables[4];                              // 0x703650, 0x703680, 0x7036b0, 0x7036e0
};
// nullptr restores the game's.
void Canvas_SetEnv(const CanvasEnv *e);
// The env in use (for tools/fncmp's driver, which resolves the game code it calls).
const CanvasEnv *Canvas_GetEnv();

extern "C" {
Canvas *__fastcall Canvas_Ctor(Canvas *c);
// The surface drawn to (ignored when null); see above.
void __fastcall Canvas_SetSurface(Canvas *c, void *edx, const SurfaceInfo *info);
Rect *__fastcall Canvas_Clip(Canvas *c);
// The static initialisers: each driver's table.
void __cdecl CanvasDriver8_Init();
void __cdecl CanvasDriver16_Init();
void __cdecl CanvasDriver24_Init();
void __cdecl CanvasDriver32_Init();
}
