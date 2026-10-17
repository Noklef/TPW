// A coaster type set up from its description (0x430000 builds one): the setup 0x42f390 and
// its pylon controls and cars and trains steps (the cross sections are in xsection.h). Each
// step fails with 0, after which the setup reports it to the empty Debug_Printf and fails.
// "Non-zero" floats are by the x87 compare against 0: NaN counts as zero.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42f390: +0x3b08 (when non-zero) to +0x40 first; 0 when the description has no track
// entries (+0xfc). Else four GlobalMem blocks: a (0x30; +0 = 3), b (0x40; +0 = 3, u16 +4 = 1,
// +8 = a), c (0x20; +0 = 0, u16 +4 = the entry count, +8 = id, +0x10 = the entries) and the
// entries (0x10 each): one per track entry (0xc bytes at +0xc: name, flag, float) with +0 6
// (flag) or 2, |0x20 and +0xc = the float when non-zero (and coaster flag 4), +4 a GlobalMem
// copy of the name; then the static ones at 0x74cde0 (count 0x74ce10). The coaster gets a
// (+0x14), b (+0x18), c (+0x1c), the count (+0x20), then CrossSection_Setup,
// CoasterPylons_Setup and CoasterTrains_Setup (0 if one fails), the +0x3ac8 pairs at +0x3a98
// in a GlobalMem block (+0x9c, count +0xa0), non-zero fields (+0x3ae8, +0x3aec -> +0x2e8,
// +0x2ec; floats +0x3ae4 -> +0xa4, +0x3acc/+0x3ad0 -> +8/+0xc, +0x3ad8..+0x3ae0 -> +0x24..+0x2c,
// +0x3b0c..+0x3b18 -> +0x44..+0x50, +0x3b28..+0x3b34 -> +0x60..+0x6c, +0x3b38..+0x3b70 ->
// +0x320..+0x358, +0x3b1c..+0x3b24 -> +0x54..+0x5c; +0x3b78 -> +0x30 and +0x34, +0x3b7c ->
// +0x3c), +0x3b74 (+0x31c) and its flags: +0x3af0 sets 0x8000 (clearing 0x4000) in +0x1c8 and
// 0x100 in +0; then 0xa0, 0x10 unless +0x3af8, and 8 (clearing 0x10) when +0x3af4. Then 1.
u32 __cdecl CoasterType_Setup(u8 *coaster, u32 id, const u8 *source);

// 0x42eb70: for each of source + 0x3a20 descriptions (0xbc bytes at source + 0x38a8; two fit
// before the count; the first fills the control at coaster + 0xa8, every later one the one at
// coaster + 0x1c8): 0 unless the description has a name (+0), +4 and a float +0xc that are
// non-zero. The control gets a GlobalMem copy of the name (+0x10), +4 (+0xc), flag 0x10 when
// +8 is non-zero, +0xc (+0x58) and +0x60..+0x68 (+0x5c..+0x64); the rest only when non-zero:
// floats +0x10 (+0xb0), +0x20..+0x28 (+0x68..+0x70), +0x6c..+0xb8 (+0xd0..+0x11c), +0x2c..+0x50
// in degrees (+0x74..+0x98, as radians: * 0x6fde78), +0x5c (+0xac, * 10 * 0x6fde78); u32s
// +0x14, +0x18, +0x1c (+0xb4, +0xb8, +0xcc); ints +0x54 and +0x58 squared (+0x9c, +0xa0) and
// as floats (+0xa4, +0xa8). 1 when all have them (or there are none).
u32 __cdecl CoasterPylons_Setup(u8 *coaster, const u8 *source);

// 0x42f1a0: 0 when source + 0x3a78 has no cars. Otherwise a GlobalMem array of that many cars
// (0x34 bytes) from the descriptions (0x1c bytes at source + 0x3a24; three fit before the
// count): each car cleared to flags 0 (+0), +4, name (+8), +0x10..+0x18 zero and +0x1c 0.1f
// (+0xc left alone), then 0 (the array kept by nobody, sic) unless the description has +4 and
// a name (+8); flag 1 when +0 is non-zero, +4, a GlobalMem copy of the name, +0xc..+0x14
// (+0x10..+0x18) and +0x18 when non-zero (+0x1c). The coaster gets the array (+0x308), the
// cars numbered at source + 0x3a80, + 0x3a84 and + 0x3a88 (+0x2fc, +0x300, +0x304; flags
// +0x2f0: 1 when the first differs from the second, 2 when the third does), source + 0x3a7c
// when non-zero, else 3 (+0x2f4), and source + 0x3a8c..+0x3a94 (+0x30c..+0x314). Then 1.
u32 __cdecl CoasterTrains_Setup(u8 *coaster, const u8 *source);

// 0x43ba20 (coaster, count): nothing when count is 0. Otherwise, with a bit of 0xc in the flags at +8, the cars from count up to the current count (+0xf4) are unloaded on both
// sides (CoasterTrain_Unload, the trains of 0x80 bytes from the pointer at +0x128); then +0xf4 is count.
void __cdecl Coaster_SetTrainCount(u8 *coaster, u32 count);
}
