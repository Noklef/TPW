// The highlighted object: reimplementation of 0x45d620-0x45d68b, 0x469180-0x469404 and
// 0x469470-0x4698ee.
//
// One holder at a time (0x7a1ce0) is drawn highlighted; the next one to highlight is the target
// (0x7afda0). Highlighting is suspended while 0x7afda4 is set. A holder's model is +8 -> +4
// (+0x30 flags, +0x44 meshes of 0xa0 bytes at +0x70). Highlighting sets the holder's flag
// 0x20000 and the model's 0x400000, and points each mesh without flag 0x4000 at a copy of its
// per-vertex block (+0x64, 12-byte entries: the mesh's +0x5c entries after its first +0x5e)
// in the shared buffer 0x7afd00, setting its flag 0x1000000 and keeping the old pointers in
// 0x7afd94; releasing puts them back (and clears 0x1000000 unless the holder has 0x4000000).
//
// Holders can also flash (the track cursor's): flashing ones are listed from 0x7afe14 through
// +0xd0 and have flag 0x4000000 (their model 0x800000). A model still sharing its base's
// (+0x9c) block +0x5c gets its own, zeroed, of +0x4a 12-byte entries (model flag 0x100000),
// carved up among its meshes' +0x64 (+0x58 + +0x5c entries each). Each frame the first +0x5e
// dwords of every flashing mesh's +0x64 are set to a colour pulsing between white and red,
// from the phase 0x7afd04 (0 to 2, up by twice gMotionTimeStep).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x469180: releases the current holder if the target is another one, then highlights the
// target unless it already is, there is none or highlighting is suspended.
void __cdecl Highlight_Update();

// 0x469340: the target.
void __cdecl Highlight_SetTarget(u8 *holder);

// 0x469350: the six tints at 0x7afdb0 (RGB, 16 bytes apart) to white. Nothing reads them.
void __cdecl Highlight_ResetTints();

// 0x469470: releases the current holder, if highlighted.
void __cdecl Highlight_Clear();

// 0x469410 (cdecl; name unconfirmed): with a current holder (0x7a1ce0), the first +0x5e dwords of the +0x64 block of each of its model's meshes without
// flag 0x4000 set to 0xffffffff (white); GetTickCount is called first (the import 0x6fd1d0) and its result dropped.
void __cdecl Highlight_WhitenCurrent();

// 0x45d620: the current holder.
u8 *__cdecl Highlight_Current();

// 0x45d610: the point at 0x7a1c90 (3 floats; the place-staff mode reads it).
float *__cdecl Highlight_Point();

// 0x45d630: 0x7a1c7c = 1 when shown and there is a current holder, else 0.
void __cdecl Highlight_SetShown(u32 shown);

// 0x45d660: suspends highlighting (releasing the current holder) or resumes it.
void __cdecl Highlight_Suspend(u32 suspend);

// 0x469510: the buffer 0x7afd00 ((0x7afd90 + 0x7afd9c) 12-byte entries) and the table 0x7afd94
// (0x7afe10 pointers), zeroed.
void __cdecl Highlight_AllocBuffers();

// 0x469550: frees them.
void __cdecl Highlight_FreeBuffers();

// 0x469580: starts the holder flashing (sets 0x1000000 on every mesh, whatever its 0x4000).
void __cdecl Highlight_Flash(u8 *holder);

// 0x469650: stops it (the holder must be listed). A highlighted holder releases the current
// one and is highlighted again after its meshes are put back: their own block freed for the
// base's pointers, or else the first +0x5e entries copied back from the base's meshes.
void __cdecl Highlight_Unflash(u8 *holder);

// 0x469840: the pulse, for each flashing holder.
void __cdecl Highlight_PulseFlashing();

// 0x4698f0 / 0x469910 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x7afe1c = 1 and 0x7b0464 = 1 / 2^23 (as
// camera's).
void __cdecl Highlight_InitUnusedOne();
void __cdecl Highlight_InitUnusedEpsilon();
}
