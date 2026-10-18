// A copy of a thing's sprite script shown over a GUI window (the panels 0x4b5340 and 0x4b6b90 show the selected thing's,
// [0x7c2658], over their window's rectangle): the thing at [0x7c24d8] and the copy's script slot at [0x7c24a0] (0x486610,
// Global7c24d8_CopyScript, keeps the copy in step). Reimplementation of 0x486530, 0x486610 and 0x486630. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x486530 (cdecl (rect, thing, depth, scale: a float)): any copy shown is killed (SpriteScript_Kill [0x7c24a0] while [0x7c24d8]
// is set), then [0x7c24d8] = thing; with no script at the thing's +0xc it is 0 again and the thing is returned. Else the script is
// copied (SpriteScript_Clone) to x = (left + (right - left) / 2 - 1016) / 1024 (the float 0x6ff154), y = ((bottom - top) / 4
// (arithmetic shifts) - __ftol(scale * -0.25) - bottom + 0x300) / 768 (0x6ff150), z = Sprite_FlipY(depth) and r8 the scale's bits, the rect's
// words +4..+0xa read sign-extended; the new slot is stored at [0x7c24a0] and returned.
u32 __cdecl ThingPreview_Show(const u8 *rect, u8 *thing, s32 depth, u32 scale);

// 0x486630 (cdecl; also reached through the jump 0x4b6b80): with [0x7c24d8] set the copy (slot [0x7c24a0]) is killed and
// [0x7c24d8] cleared.
void __cdecl ThingPreview_Hide();

// 0x486610 (cdecl): when the object at [0x7c24d8] has its +0xc set the sprite script slot [0x7c24a0] is
// made a copy of that script slot (SpriteScript_CopyFrom).
void __cdecl Global7c24d8_CopyScript();
}
