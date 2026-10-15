// Sample times along a coaster ride.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43f030: writes n evenly spaced times start + i/(n-1)*span (i = 0..n-1; start is the
// float at 0x79240c) to out, and after each but the last, the key times (the first float
// of each 0x10-byte key, from the first one not before start) that fall in [that sample,
// the next) other than one equal to the sample, copied as they are. n is **(b+0x90) when
// a == b, else 11. Returns how many floats it wrote.
// sic: out is unbounded; n = 1 gives 0/0, a NaN sample; the keys must end with one at or
// past the last sample (and the first skip with one not before start), or the reads run on.
u32 __cdecl Keys_MergeSamples(const u8 *a, const u8 *b, const float *keys, float *out, float span);
}
