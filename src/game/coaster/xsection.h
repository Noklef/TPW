// A coaster's cross sections ("Failed to setup cross sections for coaster", set up by
// 0x42d5a0): reimplementation of 0x42d6d0 and 0x42e200.
//
// The source (the coaster's section description) holds two sets of 20 buckets of up to 20
// entries each; the setup packs the used ones into GlobalMem blocks and points each section
// record at its buckets: {+0 point count, +4 index count, +8 points, +0xc indices, +0x10 the
// second points, +0x14 its value} (0x18 bytes), from the source's records of 0x14 at +0x33a0
// {+0 points bucket, +4 indices bucket, +0xc second points bucket, +0x10 value}.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42d6d0: the points. *shared = 0. Bucket b (0x194 bytes at source + 0x100 + 0x194 b, the
// count at +0x190, signed) holds 20-byte points, copied into a zeroed block of sum(counts) * 20
// (coaster + 0x90) with +4 (a float) increased by coaster + 0x40 (x87). The non-empty buckets
// get a pointer and a count in two packed lists, as in CrossSection_BuildIndices. For each of
// source + 0x3530 sections: + 8 = pointers[bucket - 1], + 0 = counts[bucket - 1]; with a second
// bucket, *shared = 1, its count must equal the first's (else 0), + 0x10 = its pointer and
// + 0x14 = the record's + 0x10. 0 when the counts sum to 0, every bucket is empty, a section
// names bucket 0 or the counts differ (sic: the lists are then not freed); else frees them, 1.
u32 __cdecl CrossSection_BuildPoints(u8 *coaster, u8 *sections, const u8 *source, u32 *shared);

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
