// The vertex transformer: the object at [0x87b1d8], made by the engine's start-up (0x578265 / 0x5782dd, one of two kinds, 0x8bd528 = 3)
// and released by Global87b1d8_Delete (core/vwrap). The pointer is to its virtual base, whose vtable (0x701920 / 0x701900) has the
// transforms in slots 1 and 2 (thunks onto 0x5f50c5 / 0x5f4f81 for the first kind), cdecl with the transformer as the first argument.
// Reimplementation of 0x570d20 and 0x5781d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x570d20 (thiscall (object; a, b, c, d, e, f); name unconfirmed): the transformer's slot 1 called (cdecl, 8 arguments) as
// (transformer, object, a, b, c, d, f, e): the last two arguments swapped (sic). The callers (0x56f853, 0x571020, ...) pass the vertices
// twice, 0x10 twice, 0 and a count. Returns what the slot returns (EAX comes through).
u32 __fastcall Transformer_Transform(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f);

// 0x5781d0 (cdecl; called at 0x5727d5 by the engine's start-up; name unconfirmed): [0x87b0d0] = [0x8bcbcc] doubled (x87, 24 bits),
// [0x87b0e4] = 0xc0000000, the matrix pointers [0x87afe8] = 0x87aff0, [0x87b154] = 0x87af68, [0x87b088] = 0x87b158 and [0x87af5c] =
// 0x87b078; then the transformer made (Mem_Alloc of 0x10 bytes): the SSE kind (3: its virtual base's vtable 0x701920) when the CPU
// flags [0x8bd534] have 0x2000000 and Transform_SseProbe says so, else the 3DNow kind (2: 0x7018f8) when [0x8bd53c] is set, else the
// x87 kind (1: 0x7018d0). The block's +0 is the kind's virtual base table (0x701960 / 0x701918 / 0x7018f0), +8 0x701940, then at the
// table's offset (its dword +4, read again each time) the vtable and 4 below it 0; [0x87b1d8] = the block plus that offset (0 when
// the allocation fails), [0x8bd528] = the kind. Returns 1.
u32 __cdecl Transformer_Create();

}
