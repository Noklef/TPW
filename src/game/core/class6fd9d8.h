// The class with the vtable 0x6fd9d8 (it owns a list of objects and one more object) and its derived classes: reimplementation of its destructor 0x41c8b0, of the derived destructors
// 0x41e2c0 .. 0x41e5c0 (some run the base destructor alone, some also destroy handles of their own), and of their scalar deleting destructors 0x41d880 .. 0x41e250; its folder / WAD handles added to the list (0x41c9e0, 0x41cb10). Named by its vtable until the class is known.
#pragma once

#include "../../../re/types.h"

struct String;

extern "C" {

// 0x41c8b0 (thiscall destructor on the object; not freed): sets the vtable pointer to 0x6fd9d8, deletes the object at +0x3c through its slot 0xc (with 1) and clears the
// pointer, and when there is a list at +0x2c empties it (each held object popped from the head is deleted through its slot 0x2c with 1; a node without a link ends the pop
// of that round with nothing deleted) and deletes the list itself through its slot 0 (with 1); then clears +0x2c (only when there was a list) and frees the strings at
// +0x18 and +4 (String_Dtor).
void __fastcall Class6fd9d8_Dtor(u8 *object);

// 0x41e2c0, 0x41e320, 0x41e4d0, 0x41e4e0, 0x41e5b0, 0x41e5c0 (thiscall): each sets the vtable pointer named and runs Class6fd9d8_Dtor on the object.
void __fastcall Class6fda48_Dtor(u8 *object);
void __fastcall Class6fda68_Dtor(u8 *object);
void __fastcall Class6fdaa8_Dtor(u8 *object);
void __fastcall Class6fdab8_Dtor(u8 *object);
void __fastcall Class6fdae8_Dtor(u8 *object);
void __fastcall Class6fdaf8_Dtor(u8 *object);

// 0x41d880, 0x41da50, 0x41ddc0, 0x41dea0, 0x41e170, 0x41e250: the matching scalar deleting destructors (thiscall; flags, pops 4): the destructor, then the object freed when bit 0 of the flags is set. Return the object.
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

// 0x41d970 (thiscall; flags, pops 4): Class6fda58_DtorB, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class6fda58_ScalarDtorB(u8 *object, void *, u32 flags);

// 0x41db50 (thiscall; flags, pops 4): Class6fda88_FullDtor, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class6fda88_FullScalarDtor(u8 *object, void *, u32 flags);

// 0x41dc60 (thiscall; flags, pops 4): Class6fdaa4_Dtor, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class6fdaa4_ScalarDtor(u8 *object, void *, u32 flags);

// 0x41dfa0 (thiscall; flags, pops 4): Class6fdac8_FullDtor, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class6fdac8_FullScalarDtor(u8 *object, void *, u32 flags);

// 0x41e090 (thiscall; flags, pops 4): Class6fda58_DtorC, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class6fda58_ScalarDtorC(u8 *object, void *, u32 flags);

// 0x41c9e0 / 0x41cb10 (thiscall; path, a String, pops 4): a new handle (Mem_Alloc 8, Handle_Ctor) with the vtable 0x6fd9e8 / 0x6fda18 opened on the path's text as a folder /
// a WAD (Handle_Open28 / Handle_OpenE0 with flags 0); on failure it is deleted through its slot 0x2c (FolderHandle_DeletingDtor, 1), else appended to the list at +0x2c
// (a node {handle, next, prev} of 0xc bytes; when that allocation fails the null node is written through). The folder form also constructs and destroys a StreamBase on the
// stack around the append. Nothing happens when the handle's allocation fails.
void __fastcall Class6fd9d8_AddFolder(u8 *object, void *, const String *path);
void __fastcall Class6fd9d8_AddWad(u8 *object, void *, const String *path);
}  // extern "C"
