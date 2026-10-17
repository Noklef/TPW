// A blend between three colour records: reimplementation of 0x5f09a0. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5f09a0 (cdecl; out, position, a, b, c; the records are nine dwords): position limited to 0 to 100; below 50 the result is a moved towards b by position / 50, from 50 it is b
// moved towards c by (position - 50) / 50. The result is the first record with its bytes 0 to 2 moved (each by the difference of the bytes times the fraction in 16.16 fixed
// point, arithmetic shift, added to the first byte) and its byte 3 set to 0xff. Returns out.
u32 *__cdecl Colour_Blend3(u32 *out, s32 position, const u32 *a, const u32 *b, const u32 *c);

}  // extern "C"
