// The sprite frames' and images' constructors, their textures' release and a frame set's texture flags: reimplementation
// of 0x588d70, 0x588320, 0x5882e0, 0x588ba0, 0x588c20, 0x588ca0, 0x588350, 0x588230, 0x588d90 and 0x588e40.
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

// 0x588940 (thiscall; width, height; name unconfirmed): +0xc / +0xe = the sizes' low words; +4 = a tracked block (TrackedGlobal_Alloc,
// core/gameheap) of the current format's pixel size (0x701fe0 indexed by [0x8bd554]) times both: 1. Not got: the error (2, 0xb0754000)
// pushed (EngineErrors_Push), 0.
u32 __fastcall SpriteImage_AllocPixels(u8 *image, void *, u32 width, u32 height);

// 0x588320 (thiscall): all 0x18 bytes but +8 cleared; returns the image.
u8 *__fastcall SpriteImage_Init(u8 *image);

// 0x5882e0 (thiscall): each image's texture gets flag 0x200 and loses 0x100 (Sprite_LoadEsp,
// Sprites_UseGroundFrame, Sprites_UseParkFrames). The count is re-read after each store.
void __fastcall SpriteFrames_MarkTextures(u8 *set);

// The image's textures (+0, an array of count-prefixed 0x60-byte textures, the count of them as a u16 at +0x16) are freed by these the
// same way: each texture with flag 0x1000 has its cache item unlinked (TexCache_UnlinkItem on the cache at 0x8c4b44), and after each the
// FIRST texture's +0x40 is cleared (sic: never the others'); then the array (when set) is destroyed (CRT_VectorDtorIterator with 0x590a70,
// Texture_Release) and freed 4 bytes below, and +0 and +0x16 cleared.

// 0x588ba0 (thiscall; name unconfirmed): the textures freed when the count is not 0 (the array not checked). Returns 1.
u32 __fastcall SpriteImage_FreeTextures(u8 *image);

// 0x588c20 (thiscall; name unconfirmed): the same when the array is set and the count not 0. Returns 1.
u32 __fastcall SpriteImage_FreeTexturesIfSet(u8 *image);

// 0x588ca0 (thiscall; name unconfirmed): with pixels at +4: the textures freed as SpriteImage_FreeTexturesIfSet does, then the pixels
// (TrackedGlobal_Free, core/gameheap) and +4 cleared. Without: when the array is set, Texture_Release on its first texture, then the array
// destroyed and freed (when still set) and +0 cleared (+0x16 kept). Returns 1.
u32 __fastcall SpriteImage_Release(u8 *image);

// 0x588350 (thiscall; name unconfirmed): the same as SpriteImage_Release, returning nothing (an image array's destructor).
void __fastcall SpriteImage_Dtor(u8 *image);

// 0x588230 (thiscall; name unconfirmed): a frame set's (+0 count, +4 its images, +8 its texture array, +0xc a tracked block) parts
// freed: the textures (when set) destroyed as one array (Texture_DestroyArray) and +8 cleared; with images, the first count images'
// +0 cleared (the count signed, re-read each time; sic: past the images' own count it writes on) and the images (when still set)
// destroyed with 0x588350 and freed, +4 cleared; the tracked block (when set) freed (TrackedGlobal_Free) and +0xc cleared; the count
// cleared. Returns 1.
u32 __fastcall SpriteFrames_Free(u8 *set);

// 0x588d90 (thiscall; width, height, pops 8; name unconfirmed): unless the s16 sizes at +0x34 / +0x36 are already these, the image's
// (+0x28, when set, its array set and count not 0) textures freed; then +0x34 / +0x36 take the low words.
void __fastcall SpriteFrame_SetSize(u8 *frame, void *, s32 width, s32 height);

// 0x588e40 (thiscall; width, height, pops 8; name unconfirmed): the same for the sizes at +0x30 / +0x32, but the height is compared with
// the s16 at +0x30 again (sic) and the image's array is not checked (only the image and the count).
void __fastcall SpriteFrame_SetSize30(u8 *frame, void *, s32 width, s32 height);
}
