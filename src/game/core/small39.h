// The inserts of the std::vector of dwords the input method code uses: reimplementation of 0x47c740 and 0x47cf20 (the vector is {allocator byte, first, last, end}, as in
// ime.h). Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47c740 (thiscall; position, pointer to the value, pops 8): inserts one value before the position. With room it is built at the end when the position is the end
// (ImeVector_Copy / ImeVector_Fill), else the last is copied up to the end, the rest shifted up and the value assigned over the position; with none the vector is rebuilt in
// a new block of twice the size (one when it was empty), the old block freed. Returns where the value went (first + the position's index).
u32 *__fastcall ImeVector_InsertOne(u8 *vec, void *, u32 *position, const u32 *value);

// 0x47cf20 (thiscall; position, count, pointer to the value, pops 12): inserts count copies. With room: when fewer elements follow the position than count they are copied
// up past the end, the rest of the new room filled from the value and the position's span assigned; otherwise the last count are copied up, the rest shifted and the count
// values assigned. Without: a new block of the larger of count and the size plus the size. Nothing for a count of 0 (with room).
void __fastcall ImeVector_InsertN(u8 *vec, void *, u32 *position, u32 count, const u32 *value);

}  // extern "C"
