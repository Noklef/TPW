// A coaster's cross sections ("Failed to setup cross sections for coaster", set up by
// 0x42d5a0): reimplementation of 0x42e200.
//
// The source (the coaster's section description) holds 20 buckets of up to 20 entries each; the
// setup packs the used ones into GlobalMem blocks and points each section record (0x18 bytes)
// at its bucket.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42e200: the index triples. Bucket b (0xf4 bytes at source + 0x2090 + 0xf4 b, the count at
// +0xf0, signed) holds 12-byte entries whose u16s at +0, +4, +8 become a packed entry's u16s at
// +4, +6, +8 (zeroed block of sum(counts) * 12, coaster + 0x94). The non-empty buckets get a
// pointer and a count in two lists, packed (sized by the last non-empty bucket). Then for each
// of source + 0x3530 sections (records of 0x14 at source + 0x33a4, +0 the bucket, from 1):
// section + 0xc = pointers[bucket - 1], + 4 = counts[bucket - 1] (sic: positions in the packed
// lists, which skip the empty buckets). 0 when the counts sum to 0, every bucket is empty or a
// section names bucket 0 (sic: the lists are then not freed); else frees the lists, 1.
u32 __cdecl CrossSection_BuildIndices(u8 *coaster, u8 *sections, const u8 *source);
}
