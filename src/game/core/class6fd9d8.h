// The class with the vtable 0x6fd9d8 (it owns a list of objects and one more object) and its derived classes: reimplementation of its destructor 0x41c8b0, of the derived destructors
// 0x41e2c0 .. 0x41e5c0 (some run the base destructor alone, some also destroy handles of their own), and of their scalar deleting destructors 0x41d880 .. 0x41e250; its folder / WAD handles added to the list (0x41c9e0, 0x41cb10, 0x41cc40) and its Find (0x41d710);
// the 0x6fda98 destructors (0x41e3a0, 0x41dc40); the 0x6fdaa4 class's .sam / .txt opens (0x41e5d0-0x41e720, 0x41f0d0-0x41f350) and its name trim (0x41eb90). Named by its vtable until the class is known.
#pragma once

#include "../../../re/types.h"

struct String;
struct StringView;

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

// 0x41cc40 (thiscall; path, pops 4): a copy of the path (String_CtorCopy) added as a folder (Class6fd9d8_AddFolder), then with the global string at 0x78bb08 appended
// (the vtable's +0x28, String_Append) added as a WAD (the same steps as Class6fd9d8_AddWad, inlined); the copy is destroyed.
void __fastcall Class6fd9d8_AddFolderAndWad(u8 *object, void *, const StringView *path);

// 0x41d710 (thiscall; key, pops 4): with no current object (+0x34) 0. Else 1 when its slot 2 (&result, key) gives a result >= 0; otherwise the list (+0x2c -> +4,
// nodes {object, next}) is searched for the node of the current object (none: the null node is read through) and each later node's object is tried through its slot 1
// (&result, the member's (+0x3c) dword +8, key): the first with a result >= 0 becomes +0x34 and +0x38, 1. None: the member is deleted through its slot 3 (1) when there
// is one, +0x3c and +0x34 cleared, 0.
u32 __fastcall Class6fd9d8_Find(u8 *object, void *, u32 key);

// 0x41e3a0 (thiscall destructor): the vtable 0x6fda98; when there is a list at +0x4c it is emptied (each node popped from the head and freed, its object deleted through
// slot 0 with 1; a node without a link pops nothing), the list deleted through its slot 0 (1) and +0x4c, +0x48 cleared; then the vtable 0x6fda58, the handle at +0x40
// destroyed (Handle_Dtor) and Class6fd9d8_Dtor.
void __fastcall Class6fda98_Dtor(u8 *object);
// 0x41dc40 (thiscall; flags, pops 4): Class6fda98_Dtor, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class6fda98_ScalarDtor(u8 *object, void *, u32 flags);

// The class 0x6fdaa4's (two strings, +4 its name, and a handle at +0x2c) opens of a .sam / .txt file: a name is built in game strings and opened through an opener's
// slot 3 (&result, stream, the name's text, 0x80000001, 0); each returns whether the result is >= 0. The globals are game strings: 0x78c780 / 0x78c738 ".sam",
// 0x78bb98 "Easy_", 0x78c990 "Online_", 0x78c708 "coaster", 0x78bec8 ".txt".
// 0x41e5d0 (stdcall; base, opener, stream): base + ".sam" (0x78c780). 0x41e660 (thiscall; opener, stream, pops 8): that with the object's name.
u32 __stdcall Class6fdaa4_OpenSamAt(const StringView *base, u8 *opener, u32 stream);
u32 __fastcall Class6fdaa4_OpenSamWith(u8 *object, void *, u8 *opener, u32 stream);
// 0x41e680 (stdcall; base, opener, stream): "Online_" + base + ".sam" (0x78c738). 0x41e720 (thiscall; opener, stream, pops 8): that with the object's name.
u32 __stdcall Class6fdaa4_OpenOnlineSamAt(const StringView *base, u8 *opener, u32 stream);
u32 __fastcall Class6fdaa4_OpenOnlineSamWith(u8 *object, void *, u8 *opener, u32 stream);
// 0x41f0d0 (thiscall; name, stream, pops 8): an empty string assigned the name (the vtable's +0x18, String_AssignWide) + ".sam", opened through the handle at +0x2c.
// 0x41f170 / 0x41f330 (thiscall; stream, pops 4): that with the object's name / with "coaster".
u32 __fastcall Class6fdaa4_OpenSam(u8 *object, void *, const String *name, u32 stream);
u32 __fastcall Class6fdaa4_OpenOwnSam(u8 *object, void *, u32 stream);
u32 __fastcall Class6fdaa4_OpenCoasterSam(u8 *object, void *, u32 stream);
// 0x41f190 / 0x41f260 (thiscall; stream, pops 4): "Easy_" / "Online_" + the object's name, assigned to an empty string + ".sam", opened through the handle at +0x2c.
u32 __fastcall Class6fdaa4_OpenEasySam(u8 *object, void *, u32 stream);
u32 __fastcall Class6fdaa4_OpenOnlineSam(u8 *object, void *, u32 stream);
// 0x41f350 (thiscall; stream, pops 4): the language buffer's text (Language_Buffer of 0x78b530) assigned to an empty string + ".txt", opened through the handle at +0x2c.
u32 __fastcall Class6fdaa4_OpenLanguageTxt(u8 *object, void *, u32 stream);

// 0x41eb90 (thiscall, the object unused; s, pops 4): when s has exactly two "_" (0x74c77c; the first found (String_Find from 0) is not the last
// (String_ReverseFind from -1) and the next after it is the last), s = its chars before the last (String_Left, the vtable's +0x1c Assign); then when
// its first char is "p" (0x74c778, String_EqualsStr) it is dropped (String_Right of length - 1). Otherwise s is left as it is.
void __fastcall Class6fdaa4_TrimName(u8 *object, void *, String *s);
}  // extern "C"
