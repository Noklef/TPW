// The camera's globals (module KAME): their static initialisers 0x42a0f0-0x42a182 and the
// follow test 0x42a410.
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
}
