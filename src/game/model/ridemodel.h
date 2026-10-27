// A ride's model as it is loaded (the loader 0x461f10 follows): reimplementation of 0x461ed0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x461ed0: for the instance at 0x7a4610[*index] (+4 flags, +8 holder, whose +4 model): flag
// 0x20 on the model (+0x30), 0x80800 on its root (+0x78, flags at +0), then 0x1000000 on the
// instance, each read through the pointers again.
void __cdecl Ride_MarkModel(const u32 *index);

// 0x461f10 (cdecl (name, flags, out, outName, layers); name unconfirmed): a ride's model and its motions loaded, an animator made for them.
// The path: the first texture directory (TexDirs_First) and the name ("%s%s"), or with none or flags 0x200 the name alone used as sprintf's
// format (sic); ".md2" appended unless it has a '.' past its first character and not before a '\'. The name's copy, cut at its last '.' (unless
// one before a '\'), is the motions' base. The model: Holder_LoadMd2(path, 0, 1, [0x7b4980] & 8, 0); none: 0. The highest of the geometry
// nodes' words +0x58 / +0x5c / +0x5e and the model's +0x42 raised into [0x7a42ac], +0x3a * 4 into [0x7afd90], +0x3e into [0x7afd9c], +0x44 into
// [0x7afe10]. Without flags 0x20000 a model flag 4 (relative animation; its Debug_Printf left out) becomes 0x1000000. outName (unless null)
// gets the path. With flags 8 each geometry node named (+0x54) "Sky" (CRT stricmp) gets 0x4000000. Flags into the animator (+4): 0x100 -> 8,
// 0x1000 -> 0x100000, 0x2000 -> 0x2000000, 0x8000 -> 0x10000000, 0x800000 -> 0x20000000; into the model (+0x30): 0x10 -> 0x800, 0x80 ->
// 0x100, 0x800 -> 0x20000, 0x4000 -> 0x200000.
// The motions, in two passes over the 12 kinds of 0x6fe6bc (a letter each): "%s%s%c%d.md2" (directory, base, letter, number from 1) while
// they exist, else (none) "%s%s%c.md2"; the first pass asks Quickload_Exists(path, [0x7a445c] ? 2 : 0) and counts (the bank's +4 count, its
// flags 1 unnumbered / 2 numbered, the total at +0xa8); the second allocates the total's tables (+0xac blocks, +0xdc models, +0xd8 entries of 8
// bytes, GlobalMem_Alloc zeroed) and loads as many again (MD2_Load(path, 0), only for a bank flagged 1 or 2; one without a motion block (+0x98)
// is freed, GlobalMem_FreeAtB0): each entry (index, block) in its bank's entries (+8, set at the bank's first). With flags 9 the base is copied
// and cut at its '.' again for nothing (sic).
// Then a block of layers * 0x38 + 0xfc bytes (zeroed): *out, its +0xfc the animator's layers (+0x10, the count +0xe); Block_MarkModel for each
// block; with the model there, flags 0x20: model flag 0x20, its root (+0x78) 0x80800, animator 0x1000000 and Animator_SetPlaying; flags 0x40:
// model flag 0x80 and each geometry node 0x20000; flags 0x400: RideBase_TidyAllUVs(animator, 0, 0.02, 0, 0). Each block's tracks (+0x2c, 0x40
// bytes, +0x12 of them): flag 0x1000 -> model flag 0x2000; flag 0x10000 -> on their node (+0x14, a geometry node or past them one of +0x74)
// 0x80 per group (+0x5a of them at +0x6c, 0x10 bytes) and 0x8030 on the word each group points to (unless null). Without flags 8 the part
// (Holder_CreateAt20); with flags 2 or 8: Animator_Reset(animator, 1, flags 0x40000, 0), without flags 8 Holder_Create, flags 2 and 4 ->
// animator 4, with flags 8 and a +0x6c record: model flag 0x400 and +0xa8 = (its +0x18 - 1) & 0xff << 16 | (its +0x1c - 1) & 0xff.
// PartStates_Create(holder); the animator (0xfc bytes, on the stack until here) copied to *out (unless null), with flags 2 or 8 its first dword
// then its own address. Returns 1.
u32 __cdecl RideModel_Load(const char *name, u32 flags, u8 **out, char *outName, u32 layers);
}
