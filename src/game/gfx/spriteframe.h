// The sprite frames' and images' constructors and a frame set's texture flags: reimplementation
// of 0x588d70, 0x588320 and 0x5882e0.
//
// A sprite frame (0x48 bytes; a font's at +0x54, 0x48f18b; the decoded textures', 0x56b4f5) is
// readied for drawing by 0x588ef0, which queues it in the sprite queue (spritequeue.h): +0x10 its
// flags, +0x28 its image, +0x34 / +0x36 (s16) the size it was last made at (0x588d90). Its image
// (0x18 bytes; the font's at +0x9c) has two pointers and six u16 sizes from +0xc. A frame set
// (0x587db0) is a count (+0) and its images (+4), each image's +0 its texture (texture.h).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x588d70 (thiscall): +0x10, +0x28, +0x30 and +0x34 = 0, +0x24 = -1, +0x2c = 1.0f; returns the
// frame.
u8 *__fastcall SpriteFrame_Init(u8 *frame);

// 0x588320 (thiscall): all 0x18 bytes but +8 cleared; returns the image.
u8 *__fastcall SpriteImage_Init(u8 *image);

// 0x5882e0 (thiscall): each image's texture gets flag 0x200 and loses 0x100 (Sprite_LoadEsp,
// Sprites_UseGroundFrame, Sprites_UseParkFrames). The count is re-read after each store.
void __fastcall SpriteFrames_MarkTextures(u8 *set);
}
