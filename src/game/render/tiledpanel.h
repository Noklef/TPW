// A panel of tiles on the screen: reimplementation of 0x58a010.
#pragma once

#include "../../../re/types.h"

// What TiledPanel_Draw calls (the port by default; the bench runs it as a recorder).
struct TiledPanelEnv {
    u32(__fastcall *drawQuad)(u8 *object, void *, u8 *spec, u32 mask);  // Batch_DrawQuad (0x565f90)
};
// The call goes to env (nullptr: the port).
void TiledPanel_SetEnv(const TiledPanelEnv *env);

extern "C" {

// 0x58a010 (thiscall; returns 1; name invented, unconfirmed). The object: +0x10 anchor flags, +0x14 / +0x18 / +0x1c place, +0x24 colour, +0x28 the panel: +0 its tiles' textures (0x60 bytes each, one after another, a
// null first one tested as such and the next 0x60 on), +0xc / +0xe / +0x10 / +0x12 image and pixel sizes; +0x2c scale, +0x30 / +0x32 anchor point. Flags 0x7000000 and 0x70000000 move the place by the anchor, in y and x
// (each top bit's case 1 not at all, 2 by half, 4 in full); the top left rounds to a pixel (fistp through the dword 0xfb34a4). The tile size is [[0x8c4b44] + 0x18f0]; each tile is a Batch_DrawQuad of a pixel-exact square, cut
// to the panel's size, with mask -1; an answer 0 returns. The polygon flag 4 of a texture's 0x100000 stays for the later tiles (sic). Texture_FormatBits' answer is overwritten in the original: left out.
u32 __fastcall TiledPanel_Draw(u8 *object, void *);
}

#include "../core/gamedata.h"

// Globals by address, by hand (named for their values). 1 at 0x702030 and -1 at 0x702038 stay literals: core/staticinit.cpp
// (agent A's) and gfx/spriteframe.cpp (agent B's) read them too.
GAME_DATA(const float, gTiledPanel_LessHalf_70204c, 0x0070204c, 0x4);
GAME_DATA(const float, gTiledPanel_Half_702050, 0x00702050, 0x4);
