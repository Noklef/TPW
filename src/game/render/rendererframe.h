// The renderer's frame: its views drawn in the window or full screen, in stereo or side by side (0x576a00, 0x576ec0; x87 at the game's 24-bit precision). The names are invented (unconfirmed).
//   A view: +0 its flags, +4 .. +0x10 its window's four floats, +0x14 its camera matrix.
//   Flags: 1 clear first (the object [0x8bd52c]'s slot 0), 0x10 the land, 0x20 the lists +0x14 .. +0x28, 0x100 the weather, 0x200 the groups' touch, 0x400 the scroll layers, 0x800 the lights, 0x1000 the backdrop, 0x2000 the tiles, 0x4000 the point sprites.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Test seam: the callees replaced by address (tests/fncmp/rendererframe.py).
void RendererFrame_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x576a00 (thiscall (view), pops 4): the camera 0x87aff0 the view's for the drawing, back after it. Each step in the order of the frame, any failure ending it through the device's slot 0x20; 0 then. Otherwise 0x8bcc44 goes up; 1.
u32 __fastcall Renderer_DrawView(u8 *renderer, void *, const u32 *view);

// 0x576ec0 (thiscall): the frame, unless the window [0x8bd518] has lost the foreground (1). A lost device reset first (else the error pushed, 0). In a window the views as they are; full screen each view twice, through the moved cameras: in stereo (flag 8) by colour mask, otherwise side by side. The precision set for it and set back, the flags 0x8bcbc8 and 0x8bd508 back after it. A view that fails: the flags back but not the precision (sic), EngineErrors_DropSurfaceLost's answer.
u32 __fastcall Renderer_DrawFrame(u8 *renderer);

// 0x44e540 (the jump to it with the renderer [0x87a6d0]).
u32 __cdecl Engine_DrawFrame();

// 0x5774b0 (cdecl; name invented, unconfirmed): the atexit entry of Renderer_DrawFrame's static matrix 0x87a680 (made the first time, guard
// bit 0 of 0x87a6c0): nothing.
void __cdecl RendererFrame_MatrixAtExit();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(float, gRenderer_DrawFrame_8bd478, 0x008bd478, 4);
