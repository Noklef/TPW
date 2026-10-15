// The highlighted object: reimplementation of 0x45d620-0x45d68b, 0x469180-0x469404 and
// 0x469470.
//
// One holder at a time (0x7a1ce0) is drawn highlighted; the next one to highlight is the target
// (0x7afda0). Highlighting is suspended while 0x7afda4 is set. A holder's model is +8 -> +4
// (+0x30 flags, +0x44 meshes of 0xa0 bytes at +0x70). Highlighting sets the holder's flag
// 0x20000 and the model's 0x400000, and points each mesh without flag 0x4000 at a copy of its
// per-vertex block (+0x64, 12-byte entries: the mesh's +0x5c entries after its first +0x5e)
// in the shared buffer 0x7afd00, setting its flag 0x1000000 and keeping the old pointers in
// 0x7afd94; releasing puts them back (and clears 0x1000000 unless the holder has 0x4000000).
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

// 0x45d620: the current holder.
u8 *__cdecl Highlight_Current();

// 0x45d630: 0x7a1c7c = 1 when shown and there is a current holder, else 0.
void __cdecl Highlight_SetShown(u32 shown);

// 0x45d660: suspends highlighting (releasing the current holder) or resumes it.
void __cdecl Highlight_Suspend(u32 suspend);
}
