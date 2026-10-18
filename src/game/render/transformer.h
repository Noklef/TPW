// The vertex transformer: the object at [0x87b1d8], made by the engine's start-up (0x578265 / 0x5782dd, one of two kinds, 0x8bd528 = 3)
// and released by Global87b1d8_Delete (core/vwrap). The pointer is to its virtual base, whose vtable (0x701920 / 0x701900) has the
// transforms in slots 1 and 2 (thunks onto 0x5f50c5 / 0x5f4f81 for the first kind), cdecl with the transformer as the first argument.
// Reimplementation of 0x570d20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x570d20 (thiscall (object; a, b, c, d, e, f); name unconfirmed): the transformer's slot 1 called (cdecl, 8 arguments) as
// (transformer, object, a, b, c, d, f, e): the last two arguments swapped (sic). The callers (0x56f853, 0x571020, ...) pass the vertices
// twice, 0x10 twice, 0 and a count. Returns what the slot returns (EAX comes through).
u32 __fastcall Transformer_Transform(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f);

}
