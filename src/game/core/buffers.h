// Counted buffers (a block with a count in front, shared between objects) and the containers and objects that hold them: release, clear, share. Reimplementation of 0x413340, 0x4791c0, 0x47aba0, 0x47b170, 0x47b1a0, 0x47b5b0, 0x47bfe0, 0x47db20, 0x47db80, 0x4ef7f0, 0x4f04d0, 0x4f23f0, 0x4f5d10, 0x4f5d40 and others. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b15a0 (thiscall): every node of the circular list at +4 (count at +8) is unlinked, its counted buffer at +8 reset (CountedBuf_Reset, releasing)
// and the node freed; then the sentinel is freed and the dwords at +4 and +8 = 0.
void __fastcall CountedList_Clear(u8 *object);

// 0x47db80 (thiscall; flags, pops 4): CountedVector14_Clear (core/buffers); when bit 0 of the flags is set the object is freed. Returns the object.
u8 *__fastcall CountedVector14_Deleter(u8 *object, void *, u32 flags);

// 0x47bfe0 (stdcall; first, last, pops 8): CountedVector14_Clear of each 0x24-byte element from first up to last.
void __stdcall CountedVector14_ClearRange(u8 *first, u8 *last);

// 0x4f04d0 (thiscall): every node of the circular list at +4 (count at +8) is unlinked, CountedBuf_ReleaseAndClear (core/buffers) of its +8 is done and
// the node freed; then the sentinel is freed and the dwords at +4 and +8 = 0.
void __fastcall CountedList_ClearRelease(u8 *object);

// 0x5c9970 (thiscall): the block at +0x20 is freed (when there is one) and the pointer cleared; the counted buffers at +0x14 and +4 (the count is the
// block's first byte: freed 1 byte below the pointer) are released and their three-dword fields cleared.
void __fastcall Counted2_Clear(u8 *object);

// 0x4e51a0 (thiscall; the destructor of a 0x24-byte element: a wide string at +0x10, then a narrow one at +0): the counted buffer at +0x14 (its block
// starts 2 bytes below the pointer) is released, the dwords at +0x14, +0x18 and +0x1c = 0; then the one at +4 (block 1 byte below), +4, +8 and +0xc = 0.
void __fastcall CountedPair_Clear(u8 *object);

// 0x4f36f0 (stdcall; first, last, pops 8): CountedPair_Clear of each 0x24-byte element from first up to (not including) last.
void __stdcall CountedPair_ClearRange(u8 *first, u8 *last);

// 0x4f0480 (thiscall): CountedPair_Clear of each 0x24-byte element from [+4] up to [+8]; then the block at +4 freed and the dwords at +4, +8 and
// +0xc = 0. Returns 0.
u32 __fastcall CountedPairVector_Clear(u8 *object);

// 0x4f7430 (thiscall; flags, pops 4): CountedPairVector_Clear; when bit 0 of the flags is set the object is freed. Returns the object.
u8 *__fastcall CountedPairVector_Deleter(u8 *object, void *, u32 flags);

// 0x4f7480 (thiscall; flags, pops 4): CountedPair_Clear; when bit 0 of the flags is set the object is freed. Returns the object.
u8 *__fastcall CountedPair_Deleter(u8 *object, void *, u32 flags);

// 0x5d46d0 (thiscall): for each of the seven 16-byte groups from +0 (last first): the dwords at +0, +4 and +8 = 0 and the block at +0xc, when there is
// one, freed and cleared.
void __fastcall Blocks7_Free(u8 *object);

// 0x5d45b0 (thiscall): all 0x70 bytes of the object (seven groups of 16 from +0; see Blocks7_Free) cleared. Returns the object.
u8 *__fastcall Blocks7_Ctor(u8 *object);

// 0x598760 (thiscall on a pointer variable): the block it points to is freed when it is set; the pointer is left.
void __fastcall Ptr_FreeIfSet(u8 **ref);

// 0x4f5d10 (thiscall on a counted object): the count at +4 is raised unless it is -1.
void __fastcall Counted_AddRef(u8 *counted);

// 0x4ef7f0 / 0x5d0dc0 (thiscall; out, pops 4): the counted object the dword at +0x20 / +0x34 points to is stored in out and gets a use. Returns out.
u8 **__fastcall Obj_ShareMember20(u8 *object, void *, u8 **out);

u8 **__fastcall Obj_ShareMember34(u8 *object, void *, u8 **out);

// 0x4f5d40 (thiscall; from, pops 4): the counted object from points to is shared (+0 = it, a use added). Returns the object.
u8 *__fastcall Counted_CopyCtor(u8 *object, void *, const u8 *from);

// 0x5b9fd0 / 0x5ba290 (thiscall): the shared record at +0x30 (when there is one) loses a use; at 0 its array (elements of 0x66 / 0x10 bytes, destroyed by the function the dword
// at 0x6fd504 / 0x6fd50c holds) and the array's block are freed, +4 is cleared and the record freed. +0x30 is cleared.
void __fastcall Shared_Release66(u8 *object);

void __fastcall Shared_Release10(u8 *object);

// 0x413340 (thiscall): the four buffers the dwords from +0x4f4 point to are freed (Mem_GlobalFree) and the dwords cleared, except those that are 0 or the shared buffer at
// 0x6fd880.
void __fastcall Obj_FreeFourBuffers(u8 *object);

// 0x5b97a0 / 0x5b9820 (thiscall; a, b, pops 8): +4 = -1, +8, +0xc, +0x10, +0x14 and +0x18 cleared, +0x1c = a, +0x20 = b; the shared record at +0x30, when there is one, loses a use
// and at 0 its array (elements of 0x26 / 0x34 bytes destroyed by the function the dword at 0x6fd508 / 0x6fd510 holds) and the record are freed; +0x30 is cleared.
void __fastcall Obj_ResetAndRelease26(u8 *object, void *, u32 a, u32 b);

void __fastcall Obj_ResetAndRelease34(u8 *object, void *, u32 a, u32 b);

// 0x598630 (thiscall on {data, size, max, grow by}; new size, grow by, pops 8): a grow by other than -1 is stored. A new size of 0 frees the data and clears the data, size and
// max. With no data a block of the new size (zeroed) is made. When the new size fits the max the new elements are zeroed and the size is the new size. Otherwise the block
// grows to max + (the grow by, or when 0 an eighth of the size limited to 4 to 0x400), at least the new size: the elements are copied, the new ones zeroed, the old block freed.
void __fastcall IntArray_SetSize(u8 *array, void *, s32 newSize, s32 growBy);

// 0x47b1a0 (thiscall): when the byte at +0 is not 0 the block at +4 is freed (the fields are left).
void __fastcall Buffer_FreeIfOwned(u8 *object);

// 0x47b170 (thiscall): the block at +4 is freed; the dwords at +4, +8 and +0xc = 0. Returns 0.
u32 __fastcall Buffer_FreeAndClear(u8 *object);

// 0x47b5b0 (stdcall; first, last): for each 16-byte entry from first up to (not including) last: the counted buffer at +4 is released
// (one use less, or the block freed when the count is 0 or 0xff), then the dwords at +4, +8 and +0xc = 0. Nothing when first == last.
void __stdcall CountedEntries_Release(u8 *first, u8 *last);

// 0x4f23f0 (thiscall; release, pops 4): when the byte release is not 0 the counted buffer at +4 (when set) is released; then the dwords at
// +4, +8 and +0xc = 0.
void __fastcall CountedBuf_Reset(u8 *object, void *, u32 release);

// 0x47db20 (thiscall; flags, pops 4): the counted buffer at +4 (when set) is released; the dwords at +4, +8 and +0xc = 0; when bit 0 of
// the flags is set the object is freed. Returns the object.
u8 *__fastcall CountedBuf_Deleter(u8 *object, void *, u32 flags);

// 0x55f1e0 (thiscall): the block at +0xc, when there is one, is freed; the dwords at +0, +4, +8 and +0xc = 0.
void __fastcall BufferC_FreeAndClear(u8 *object);

// 0x4f75e0 (thiscall): the counted buffer at +4 (see core/buffers), when set, is released (one use less, or the block freed when the count
// is 0 or 0xff); then the dwords at +4, +8 and +0xc = 0.
void __fastcall CountedBuf_ReleaseAndClear(u8 *object);

// 0x5101f0 (thiscall): a new block of 0x50 bytes cleared (when there is one); the dwords at +0 and +4 = the block (0 for none) and the dword at
// +8 = the block + 0x280 (sic: 0x280 for none). Returns the object.
u8 *__fastcall Block50_Ctor(u8 *object);

// 0x4791c0 (thiscall): the counted buffer (see core/buffers) at +4 of each 16-byte entry from [+0x14] up to [+0x18] released and its fields cleared;
// then the block at +0x14 freed and the dwords at +0x14, +0x18 and +0x1c = 0.
void __fastcall CountedVector14_Clear(u8 *object);

// 0x47aba0 (thiscall): the same with the entries from [+4] up to [+8] and the fields +4, +8 and +0xc.
void __fastcall CountedVector4_Clear(u8 *object);
}  // extern "C"
