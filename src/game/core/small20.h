// The compiler runtime's array construction and destruction helpers (MSVC6's "eh vector constructor iterator" / "eh vector destructor iterator"):
// reimplementation of 0x67acf0 and 0x67afa0. The unwinding that destroys the constructed elements when a constructor throws is left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x67acf0 (stdcall; array, size, count, ctor, dtor, pops 20): ctor (thiscall on the element, no other arguments) for each of the count (signed) elements of
// size bytes in turn; the dtor is only for the unwinding.
void __stdcall CRT_VectorCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *), void *dtor);

// 0x67afa0 (stdcall; array, size, count, dtor, pops 16): dtor (thiscall) for each of the count elements, the last first.
void __stdcall CRT_VectorDtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *dtor)(void *));

}  // extern "C"
