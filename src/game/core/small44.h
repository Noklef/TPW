// The array of dwords with a grow step (the MFC style CArray): reimplementation of its SetSize 0x598630. Named by what it does until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x598630 (thiscall on {data, size, max, grow by}; new size, grow by, pops 8): a grow by other than -1 is stored. A new size of 0 frees the data and clears the data, size and
// max. With no data a block of the new size (zeroed) is made. When the new size fits the max the new elements are zeroed and the size is the new size. Otherwise the block
// grows to max + (the grow by, or when 0 an eighth of the size limited to 4 to 0x400), at least the new size: the elements are copied, the new ones zeroed, the old block freed.
void __fastcall IntArray_SetSize(u8 *array, void *, s32 newSize, s32 growBy);

}  // extern "C"
