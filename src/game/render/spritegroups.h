// A frame's camera-facing sprites, by group: reimplementation of 0x57c620.
#pragma once

#include "../../../re/types.h"

// What SpriteGroups_Draw calls through (the port by default; the bench runs it as a recorder).
struct SpriteGroupsEnv {
    u32(__fastcall *drawQuad)(u8 *object, void *, u8 *spec, u32 mask);  // Batch_DrawQuad (0x565f90)
};
// The call goes to env (nullptr: the port).
void SpriteGroups_SetEnv(const SpriteGroupsEnv *env);

extern "C" {

// 0x57c620 (thiscall; returns 1; name invented, unconfirmed). The object: +4, +8 the frame last drawn (the globals 0x8bcc40, 0x8bcc44; the same frame: nothing), +0xc a byte for each of 120 groups (0x40: draw it),
// +0x94 the group's list, +0x98 its release, +0x9c its count (cdecl function pointers). A list's sprite, 0x1c bytes: +0 / +4 / +8 place (integers in 4096ths), +0xc colour, +0x10 holder (-1: none) of a texture and a
// float size, +0x14 size, +0x16 turn, +0x18 flags: 0x40000 the place is on screen already (else through the matrix at 0x87b088), 4 / 0x2000 / 0x20000 / 0x4000 batch flags, 0x10000 polygon flag 2 (spec +0x28).
// With no holder the width is scaled by [0x8bcbcc], the flags by the alpha. Each is a Batch_DrawQuad with mask -1: its answer 0 returns at once without the list's release (sic). The polygon flag 4 of a texture's
// 0x100000 stays for the later sprites of all groups (sic).
u32 __fastcall SpriteGroups_Draw(u8 *object, void *);
}
