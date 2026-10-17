// A constructor with two small helper objects, an object factory and a constructor of constants: reimplementation of 0x5dc580, 0x617ff0 and 0x50ffe0. Named by their vtables until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5dc580 (thiscall): the 0x702d48 base constructor (Class702d48_Ctor), the vtable 0x702bc8, a 0x10-byte object {vtable 0x702bc0, three zero dwords} at +8 and another {vtable
// 0x702bbc, three zero dwords} at +0x6c (each 0 when the allocation fails), +0x78 and +0x74 cleared and the floats 0.4, 0.15, 0.5, 300 at +0x1c, +0x20, +0x24, +0x38 and 0.2,
// 0.075, 0.25, 150, 1.5 at +0x28, +0x2c, +0x30, +0x3c, +0x40. Returns the object.
u8 *__fastcall Class702bc8_Ctor(u8 *object);

// 0x617ff0 (cdecl; result code out, object out): a null object pointer gives 0x80004003; otherwise a 0x18-byte object is allocated and constructed (Class703770_Ctor, then +8 cleared
// and the vtable 0x7037c0) and stored, with result 0, or the result is 0xcffe0100 when the allocation fails. Returns the result pointer.
u32 *__cdecl Class7037c0_Create(u32 *result, u8 **object);

// 0x50ffe0 (thiscall): constants and zeroes into the first 0xbc bytes (the final values: +0 0x10000, +4 and +0x1c 0x3333, +0x18 0x6666, the byte at +0x60 1; +0x48 is left
// alone), and the five 8-byte elements at +0x64 constructed (IntPair_Ctor, through CRT_VectorCtorIterator). Returns the object.
u8 *__fastcall Class50ffe0_Ctor(u8 *object);

}  // extern "C"

