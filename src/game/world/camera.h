// The camera's globals (module KAME): their static initialisers 0x42a0f0-0x42a182, the
// follow test 0x42a410 and the small helpers around them.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42a0f0 / 0x42a110: 0x7909e8 = 1.0f, 0x790a94 = 1.0f / 8388608.0f (0x6fdd54 / 0x6fdd10);
// nothing reads either.
void __cdecl Camera_InitUnusedOne();
void __cdecl Camera_InitUnusedEpsilon();

// 0x42a130: 0x7909ec = 110.0f (0x6fdd40; stepped by 0x42a8xx). 0x42a140: the yaw 0x790a38 (set
// to multiples of pi / 2 by 0x42a2xx) = 0.0f; 0x42a150: 0x7909e0 = 0.0f, both from 0x6fdd44.
// 0x42a160: the point at 0x7908f0 = (475, 0, 175) (0x6fdd48).
void __cdecl Camera_Init7909ec();
void __cdecl Camera_InitYaw();
void __cdecl Camera_Init7909e0();
void __cdecl Camera_InitPoint();

// 0x42a410: 1 when the camera follows thing: gCameraFlags (0x790ab0) bit 4, 0x790988 set and
// the followed thing (0x790a44) non-null and thing; else 0.
u32 __cdecl Camera_IsFollowing(const void *thing);

// 0x42b070: 1 when two matrices (4x4 floats) are within 0x74c980 (1e-5) of each other: the
// sum of |a - b| over the first three columns of each row is less (or NaN). The sum runs in
// the order the original's schedule gives, with its running total stored to a float after
// every second term (from the third on), so overflow matches.
u32 __cdecl Camera_MatricesClose(const float *a, const float *b);

// 0x42a4e0: the followed thing (0x790a44) when the camera follows one (flag 4, 0x790988);
// else null.
const void *__cdecl Camera_Followed();

// 0x42a720: the face of part *part of the instance's model ((*instance)+4): +0x70 (0xa0 each)
// below the model's +0x44 count, else +0x74 (0x58 each); the index is offset by +0x46.
u8 *__cdecl Camera_PartFace(u8 *const *instance, const u32 *part);

// 0x42aab0: clears 0x790a88-0x790a90, then sets each given value: the point's x and z
// (0x7908f0 / 0x7908f8; z is read whenever x is given), 0x7909ec, the yaw 0x790a38 and
// 0x74c9bc.
void __cdecl Camera_SetView(const u32 *x, const u32 *z, const u32 *zoom, const u32 *yaw, const u32 *other);

// 0x4868d0 (cdecl; &cell; name unconfirmed): Camera_SetView of the point only: the cell id less 1 (a u16 widened, so 0 is 0xffffffff) split
// into x (its low 7 bits) and z (the rest), each times 10 (wrapping) as a float.
void __cdecl Camera_CentreOnCell(const u16 *cell);

// 0x4867b0 (cdecl; handle; name unconfirmed): for a thing's handle (its low word; 0 nothing) whose slot (0x7cfb90) holds a thing, the camera
// centred (as Camera_CentreOnCell) on the cell id of its cell bytes +5 (x) and +7 (z): z * 128 + x + 1 as a word. (Its debug check of the
// thing table left out.)
void __cdecl Camera_CentreOnThing(u32 handle);

// 0x42afd0: sets (on) or clears gCameraFlags bit 0x80.
void __cdecl Camera_SetFlag80(u32 on);

// 0x42cd40: clamps a cell to [0, 0x7a0314 - 1] x [0, 0x7a0374 - 1] (signed; the lower bound
// first).
void __cdecl Camera_ClampCell(s32 *x, s32 *z);

// 0x48ce80 (cdecl; name unconfirmed): the camera centred on the thing [0x7c2658] points to (Camera_CentreOnThing with its handle).
void __cdecl Camera_CentreOnGlobalThing();
}
