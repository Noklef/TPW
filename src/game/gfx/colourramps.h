// Five tables of ten colours rising from black, in an object. Reimplementation of 0x46be50 (name invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x46be50 (thiscall (); name invented, unconfirmed; no direct call): for v = 0, 28, 56 ... 255 (i * 255 / 9), each a 5-byte colour (argb with alpha 0xff, then its nearest
// entry of the game's palette 0xfa71f8, Palette_Nearest): at +0 red, +0x38 green, +0x70 blue, +0xa8 yellow (red and green), +0xe0 grey. Then six colours are
// copied to where the object's pointers say: the sixth of the first four tables to [+0x34], [+0x6c], [+0xa4] and [+0xdc]; the tenth grey to [+0x114]; the first
// grey to [+0x118].
void __fastcall ColourRamps_Build(u8 *object);
}
