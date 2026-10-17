// The class with the vtable 0x6fd9d8 (it owns a list of objects and one more object) and its derived classes: reimplementation of its destructor 0x41c8b0, of the derived destructors
// 0x41e2c0 .. 0x41e5c0 (some run the base destructor alone, some also destroy handles of their own), and of their scalar deleting destructors 0x41d880 .. 0x41e250. Named by its vtable until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41c8b0 (thiscall destructor on the object; not freed): sets the vtable pointer to 0x6fd9d8, deletes the object at +0x3c through its slot 0xc (with 1) and clears the
// pointer, and when there is a list at +0x2c empties it (each held object popped from the head is deleted through its slot 0x2c with 1; a node without a link ends the pop
// of that round with nothing deleted) and deletes the list itself through its slot 0 (with 1); then clears +0x2c (only when there was a list) and frees the strings at
// +0x18 and +4 (String_Dtor).
void __fastcall Class6fd9d8_Dtor(u8 *object);

// Each sets the vtable pointer named and runs Class6fd9d8_Dtor (the first six) or Class702cb0_Dtor (the last two) on the object.
void __fastcall Class6fda48_Dtor(u8 *object);

void __fastcall Class6fda68_Dtor(u8 *object);

void __fastcall Class6fdaa8_Dtor(u8 *object);

void __fastcall Class6fdab8_Dtor(u8 *object);

void __fastcall Class6fdae8_Dtor(u8 *object);

void __fastcall Class6fdaf8_Dtor(u8 *object);

// The matching scalar deleting destructors (thiscall; flags, pops 4): the destructor, then the object freed when bit 0 of the flags is set. Return the object.
u8 *__fastcall Class6fda48_ScalarDtor(u8 *object, void *, u32 flags);

u8 *__fastcall Class6fda68_ScalarDtor(u8 *object, void *, u32 flags);

u8 *__fastcall Class6fdaa8_ScalarDtor(u8 *object, void *, u32 flags);

u8 *__fastcall Class6fdab8_ScalarDtor(u8 *object, void *, u32 flags);

u8 *__fastcall Class6fdae8_ScalarDtor(u8 *object, void *, u32 flags);

u8 *__fastcall Class6fdaf8_ScalarDtor(u8 *object, void *, u32 flags);

// 0x41e270 / 0x41e2d0 / 0x41e560 (thiscall; the same body): the vtable 0x6fda58 is set, the handle at +0x40 is destroyed (Handle_Dtor), then the 0x6fd9d8 base destructor runs.
void __fastcall Class6fda58_DtorA(u8 *object);

void __fastcall Class6fda58_DtorB(u8 *object);

void __fastcall Class6fda58_DtorC(u8 *object);

// 0x41e330 / 0x41e4f0 (thiscall): the vtable (0x6fda88 / 0x6fdac8) is set and the handle at +0x48 destroyed; then the vtable 0x6fda58, the handle at +0x40 and the base destructor.
void __fastcall Class6fda88_FullDtor(u8 *object);

void __fastcall Class6fdac8_FullDtor(u8 *object);

// 0x41e470 (thiscall): the vtable 0x6fdaa4 is set, the handle at +0x2c destroyed, then the strings at +0x18 and +4 (String_Dtor).
void __fastcall Class6fdaa4_Dtor(u8 *object);

}  // extern "C"
