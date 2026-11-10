// Sprite definitions: reimplementation of the constructor (0x540bd0) and the lookups at
// 0x541f70-0x541fd0 and 0x5423a0.
//
// A sprite (0x260 bytes, loaded by Sprite_LoadEsp 0x540d90 from esprites.wad) is:
//   +0 its image set (whose +4 is the 24-byte frame table), +0x204 loaded, +0x205/+0x206
//   flags from the file, +0x207 (1), +0x208 the image set's first word, +0x20c the number
//   of special actions, +0x20e 4 special actions (entry + 1, animation + 1, two bytes),
//   +0x21e the number of animations, +0x220 16 animations (u16 first frame, frames per
//   direction, directions).
// gSpriteFolders (0x763f88, 12-byte entries: name, first index in gSprites 0x876b48,
// count) groups them as esprites.wad does.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x540bd0 (__thiscall): clears the counts, actions and animations; +0x207 = 1. Returns
// the sprite.
u8 *__fastcall Sprite_Ctor(u8 *sprite);
// 0x541f70 (folder): a random sprite of the folder, (World_Rand >> 2) % its count.
u32 __cdecl SpriteFolder_RandomSprite(u32 folder);
// 0x541fa0 (folder, sprite): its number of special actions (+0x20c).
u32 __cdecl Sprite_ActionCount(u32 folder, u32 sprite);
// 0x541fd0 (folder, sprite): a random animation, (World_Rand >> 2) % its count (+0x21e).
u32 __cdecl Sprite_RandomAnim(u32 folder, u32 sprite);
// 0x5423a0 (animation, frame, direction) of the sprites of folder 13 (as Sprite_DrawScript
// counts animations, 16 per sprite): the frame's entry in the frame table; no mirroring
// (direction & 7, 0 for a sprite without directions). Nothing calls it.
u32 __cdecl Sprite_FrameEntry(s32 anim, u32 frame, u32 dir);
// 0x540d90 (thiscall on the sprite; entry, files; pops 8): the file named at the entry's +0x24, opened on a new stream by the file system (its vtable's +0xc: (result out, stream, path, 0x80000001, 0),
// the result's dword below 0 a failure): 0x10c bytes skipped, the flags +0x205 / +0x206, 16 animations (word, two bytes; the count of those with the first byte +0x21e), 4 actions of four bytes, the first's
// read failing leaving them clear (the count of those with a first byte +0x20c). The stream is closed; +4 gets the path without the suffix the global string 0x8757a8 is as long as. The path then has that length
// taken off and a suffix put on: by [0x763f28] 0x875fe8 / 0x876270 or else 0x875598 / 0x8757a8, the first of each when [0x8768fc] and +0x205 are set. That file opened, a new 0x10-byte frame set (+0)
// is loaded (SpriteFrames_Load; +0x204 = loaded), +0x208 = its count, +0x207 = [0x877354], and if that is set SpriteFrames_MarkTextures; [0x87626c] counts up. Returns 1 in AL, 0 for a failed open or stream
// block (the stream is not freed: sic).
u32 __fastcall Sprite_LoadEsp(u8 *sprite, void *, const u8 *entry, u8 *files);
}
