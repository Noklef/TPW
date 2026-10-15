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

// 0x42d5a0: the setup. 0 without sections (source + 0x3530, n); else n + 1 section records
// (GlobalMem, zeroed; coaster + 0x88, n at + 0x8c), CrossSection_BuildPoints, _BuildIndices,
// then the extra record (n) a copy of the section numbered at source + 0x3b00 (0: the first;
// unchecked) with its own copy of the index entries (when the copy, its old list and its count
// are non-zero), CrossSection_BuildPieces (flag: some section has second points) and
// _DropUnusedPoints. 0 when a step fails (sic: what it allocated stays), else 1.
u32 __cdecl CrossSection_Setup(u8 *coaster, const u8 *source);

// 0x42d6d0: the points. *shared = 0. Bucket b (0x194 bytes at source + 0x100 + 0x194 b, the
// count at +0x190, signed) holds 20-byte points, copied into a zeroed block of sum(counts) * 20
// (coaster + 0x90) with +4 (a float) increased by coaster + 0x40 (x87). The non-empty buckets
// get a pointer and a count in two packed lists, as in CrossSection_BuildIndices. For each of
// source + 0x3530 sections: + 8 = pointers[bucket - 1], + 0 = counts[bucket - 1]; with a second
// bucket, *shared = 1, its count must equal the first's (else 0), + 0x10 = its pointer and
// + 0x14 = the record's + 0x10. 0 when the counts sum to 0, every bucket is empty, a section
// names bucket 0 or the counts differ (sic: the lists are then not freed); else frees them, 1.
u32 __cdecl CrossSection_BuildPoints(u8 *coaster, u8 *sections, const u8 *source, u32 *shared);

// 0x42d130: the pieces: source + 0x38a4 records of 0x2c (GlobalMem, zeroed; coaster + 0x78,
// the count + 0x7c), each from the source's at + 0x3534: four angle ranges (+0..+0x1c, floats
// in degrees, as radians: * 0x6fde78; a range whose two are both 0 or NaN, as the x87 compares,
// stays -FLT_MAX..FLT_MAX), then up to three sections (+0x20, +0x24, +0x28: numbers from 1,
// as pointers into sections; the first required) -- 0 when one is out of range (sic: the block
// stays). Before that: coaster + 0x84 = the section numbered at source + 0x3b04 (0 for none;
// else coaster |= 0x40), 0 when there are no pieces; then flag: coaster + 0x74 |= 1, and
// coaster + 0x80 = the sections' end (count n). 1 when all are in range.
u32 __cdecl CrossSection_BuildPieces(u8 *coaster, u8 *sections, u32 n, const u8 *source, u32 flag);

// 0x42d390: packs a section's points (+0 of them at +8) to `to` (20 bytes each, copied unless
// already there), keeping those whose used[i] is non-zero and setting used[i] to the point's
// new index. Unless the first index entry (+0xc) already has flag 1, every entry's u16s at +6
// and +8 (point indices) become used[index] and its flag 1 is set. Counts are unsigned.
void __cdecl CrossSection_PackPoints(s32 *used, u8 *section, u8 *to);

// 0x42d440: drops the points no index entry of a section uses, in two passes over the n
// sections: an unshared section (no other section's +8 is its own; and none with second points
// at +0x10) is packed in place; the first pass sums the used points of the shared ones, and if
// there are any, the second allocates them a block (GlobalMem, zeroed; coaster + 0x98) and packs
// each shared section into it (+8, +0 = its used count). coaster + 0x98 = 0 when a pass finds
// none. The used marks are an array that carries over between sections: the original's has
// 20 entries on the stack, uninitialised, and point counts above 20 or indices of 20 and up
// overrun it (its arguments and return address first). That is not reproduced: here it has
// an entry for every u16 index, zeroed, and the counts are clamped to it.
void __cdecl CrossSection_DropUnusedPoints(u8 *coaster, u8 *sections, u32 n);

// 0x42e200: the index triples. Bucket b (0xf4 bytes at source + 0x2090 + 0xf4 b, the count at
// +0xf0, signed) holds 12-byte entries whose u16s at +0, +4, +8 become a packed entry's u16s at
// +4, +6, +8 (zeroed block of sum(counts) * 12, coaster + 0x94). The non-empty buckets get a
// pointer and a count in two lists, packed (sized by the last non-empty bucket). Then for each
// of source + 0x3530 sections (records of 0x14 at source + 0x33a4, +0 the bucket, from 1):
// section + 0xc = pointers[bucket - 1], + 4 = counts[bucket - 1] (sic: positions in the packed
// lists, which skip the empty buckets). 0 when the counts sum to 0, every bucket is empty or a
// section names bucket 0 (sic: the lists are then not freed); else frees the lists, 1.
u32 __cdecl CrossSection_BuildIndices(u8 *coaster, u8 *sections, const u8 *source);

// 0x43fd60: a section's points (+8, count +0, 20 bytes: x, y, u, normal x, y) placed along
// the track as vertices from 0x79c600 on (not advanced) in the mesh at 0x792218 -> +0x38:
// positions (+0x10, x/y/z in groups of four), normals (+0x14, three floats) and uvs (+0x18,
// u/v in groups of four; v = 0x792218 -> +0x74, both moved by fld/fstp). With second points
// (+0x10) the points are first blended (into a 50-point stack buffer: more overrun it) by
// f = (blend - key[-3]) / (key[1] - key[-3]) * key[2], + 1 - key[2] when +0x14 > 0, clamped
// to [0, 1] (NaN: 0): p * (1 - f) + q * f. Each point (and its normal, then normalised) is
// rolled in its plane, pitched and yawed by the sine table, and offset; min and max grow to
// take each position, both tested when separate, else max only when min did not change.
// Nothing for a count <= 0 (signed; with second points the original then blends ~4G points).
void __cdecl CrossSection_Emit(float blend, float yaw, float pitch, float roll, const float *offset, float *min,
                               float *max, u32 separate, const u8 *section, const float *key);
}
