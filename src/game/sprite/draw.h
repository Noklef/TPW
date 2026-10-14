// The sprite draw lists: reimplementation of 0x540ab0, 0x540b30 and 0x542010, which
// SpriteScripts_Draw (0x475430) calls each frame around the scripts, and of the switch
// 0x5408e0/0x5408f0.
//
// A shown script is not drawn here: its draw record (+0xd0 of the script, 0x44 bytes) is
// filled in and appended to a list for the renderer. The map is split into a 4x4 grid of
// cells (cell size 0x8762b4 x 0x87614c, in tenths of the position); each cell has a list
// (pointers at 0x875528, counts at 0x876490, at most 0x800 records), and scripts with flag
// 0x40000 go to one more list (0x87692c, count 0x875864), unsorted. The record:
//   +0x10 flags (r16's low 24 bits; 0x100000 for folder 5, 0x10 mirrored), +0x14 x,
//   +0x18 height, +0x1c z, +0x24 colour (r7 << 24 | 0xffffff), +0x28 the frame (an entry
//   of the 24-byte frame table at +4 of the sprite's image set, the sprite's first word),
//   +0x2c r8, +0x30 r9; with flag 0x200 also +0x34 the image set, +0x38 the animation's
//   first frame, +0x3a directions << 4 | frames per direction, +0x3b r15's direction << 4
//   | frame.
// A sprite (gSprites, 0x876b48) has 16 animations per entry: animation n of the script's
// sprite is animation n % 16 of the sprite n / 16 entries on; each is 4 bytes at +0x220
// (u16 first frame, frames per direction, directions). Frames are stored per direction;
// directions beyond the sprite's are drawn mirrored (8 - d).
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x540ab0: empties the lists and sets the camera heading (0x876b24) the directions are
// taken from: Matrix_Heading of the camera matrix (0x790a98) when the camera flags
// (0x790ab0) have 0x16, else the camera angle (0x790a38) wrapped to 0..2pi.
void __cdecl SpriteDraw_Begin();
// 0x540b30: copies the list counts for the renderer (0x8751f4 + 0x24 * cell, 0x876424),
// then picks the thing under the cursor (0x532bd0) and hands the last frame's pick to
// 0x7b05a8 (0x423b50).
void __cdecl SpriteDraw_End();
// 0x542010 (script): fills the shown script's draw record and lists it (see above). The
// height is r2 above the ground (0x4527f0 at r1, r3), or r2 itself with flag 0x1000000.
// The direction is r15 plus the camera heading in eighths (flag 0x40000: none; flag
// 0x2000000 or a sprite without directions: direction 0). The direction offset is kept
// at 0x87734c.
void __cdecl Sprite_DrawScript(u8 *script);
// 0x5408e0 / 0x5408f0: sprites are drawn (byte 0x763f84 = 1) / not (0).
void __cdecl SpriteDraw_Enable();
void __cdecl SpriteDraw_Disable();
}

// What the draw path calls in the game, so that it can run outside it.
struct SpriteDrawEnv {
    double(__cdecl *height)(u32 x, u32 z);  // 0x4527f0 (float bits): the ground height
    u32(__cdecl *pick)();                   // 0x532bd0: the thing under the cursor
    void(__cdecl *setPicked)(u32 thing);    // 0x423b50 (__thiscall) on 0x7b05a8
};
// The draw path's calls go to env (nullptr: the game's).
void SpriteDraw_SetEnv(const SpriteDrawEnv *env);
