// The clip codes of a run of points (hand-written assembly at 0x561bc0): reimplementation.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x561bc0 (out, codes, points, count, stride, mask; name unconfirmed; its own register use): for each of count points (count 0 runs 2^32 times,
// sic), stride bytes apart, of four dwords x, y, z, w (floats: doubling a pattern drops its sign, so the unsigned compares are of magnitudes), the
// 7-bit index sign(w), sign(z), |w| < |z|, sign(y), |w| < |y|, sign(x), |w| < |x| (first the top) picks a byte of the table at 0x879970; that byte
// and mask's low byte is the point's code, stored in turn from codes. The dword at out gets the AND of the codes (byte 0, starting at 0xff), their
// OR (byte 1) and the caller's ECX's high half (bytes 2-3: the local the function's PUSH ECX makes, sic), so it is taken here as a fastcall's
// first argument (the second, EDX, is shifted out). Returns out (EAX); pops 24 bytes.
u8 *__fastcall ClipCodes_Compute(u32 ecx, u32 edx, u8 *out, u8 *codes, const u8 *points, u32 count, u32 stride, u32 mask);
}
