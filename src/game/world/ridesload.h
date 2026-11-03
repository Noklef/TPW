// The rides read back from a save (0x4647a0).
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

struct Stream;

extern "C" {

// Its calls that the bench replaces go by the game's addresses; a table of replacements (tests/fncmp/ridesload.py gives every one a recorder).
void RidesLoad_SetCalls(const AddrCall *table, u32 count);

// 0x4647a0 (cdecl (stream); name invented, unconfirmed): a size and that many bytes read into a GlobalMem_Alloc block (a read's negative result: 0, the block kept). Three dwords to 0x7a4084, 0x7a3ebc, 0x7aedf4, the clocks updated, then the first two's sum of records, each a tag byte, the rest only with tag 1:
// 0x26 bytes (+0 a type's id, looked for from the last in the table 0x7ae6a8 by its +0xf8; +0x1c flags, +0x20 u16, +0x22 dword), 0x10 (+4, +6 u16 part and vertex counts) and a new ride (Ride_Create, with the definition's +0x18 (ObjectDefs_Find) or the size at 0x7a2ed8 when both are set, else null). Its flags, then its
// vertices' (when the counts are its model's and it has them) and its parts' flags from the file, skipped when the counts differ; then its layers: copied, a block set when its motion is there (Animator_HasMotion, Layer_SetBlock) and 0x464580; no layer without bank 0: 0x473550. A layer's bank below 12 sets +4's 0x400000; last 0x472f40 or (without 0x1000000) 0x472ee0.
// The block freed. Returns 1.
u32 __cdecl RideSystem_Load(Stream *stream);
}
