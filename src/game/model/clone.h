// Holder and model piece cloning: reimplementations of 0x454a10 and 0x4554d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x454a10: a copy of a holder and its model in one block (GlobalMem_Alloc, aligned to 8):
// the holder (0x40 bytes), the model (0xb8), then by flag the model's arrays, each section
// rounded up to 8 -- 1 the nodes (0xa0 primary, 0x58 secondary; their +4/+8/+0xc pointers
// moved by the distance the nodes moved; without it only the root +0x78), 4 (with 0x100)
// +0x58 (0x30 each), 8 +0x5c (12), 0x20 +0x68 (8), 0x40 +0x64 (16), 0x80 +0x60 (32) or 0x10
// each flagged node's +0x68 run, 0x200 +0x50 (8), the model's 0x18000 +0x54 (16, with
// 22-byte records) and the slot table (+0x28 of the holder) when the model has slots. The
// node pointers into the copied arrays are rebased. Flag 4 implies 1 and 0x100; 0x40 comes
// with 0x200, the model's 0x1000, or its 8 while the terrain flags (0x7a1a8c) have 0x2000.
// *out = the copy, whose faces are then marked (0x1000: Model_MarkFacesNotValue, else
// Model_MarkFacesByMask, with mask). Returns 1. The second argument is unused.
u32 __cdecl Holder_Clone(const u8 *holder, u32 unused, u32 flags, u8 **out, u32 mask);

// 0x4554d0: a copy of a model piece in one block (GlobalMem_Alloc, not zeroed): its 0x48-byte
// header (+0x44 = the copy), its +0x12 parts (0x40 each, from +0x2c, when it has them), and
// for each part with flag 0x1000 but not 0x4000 (+4) its +0x28 record (0x34 bytes) and that
// record's +2 entries of 20 bytes (+0xc), the copies' pointers moved to them. Sized in a
// first pass. *out = the copy; returns 1. heap (0x79fb2c when 0) and unused are unused.
u32 __cdecl ModelPiece_Clone(u32 piece, u32 heap, u32 unused, u32 *out);
}
