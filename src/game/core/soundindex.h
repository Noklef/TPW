// The sound resource (vtable 0x70ab00, on the base class 0x703770: +4 a count set to 1; +8 and +0xc zeroed, +0x10 the source, an
// object with AddRef at slot 1 and Release at slot 0) and its two index classes; the class names are invented. The stream index
// (vtable 0x70aa80): a StreamBase at +0x14 (files/handles.h; slot 1 close, 2 read (buffer, count), 4 seek (offset, origin)), +0xc a
// count and +8 a block of that many offsets read from the stream's start. The mapped index (vtable 0x70aac0): a StreamBase at +0x18
// whose slot 7 maps it (the block: a count, then offsets), +8 that block, +0xc its count, +0x14 a block of as many resolved slots
// (0x6d4890 and 0x6d48f0 read it: core/methods, core/fieldops). Reimplementation of 0x6d4910, 0x6c5450, 0x6c54e0, 0x6d46e0, 0x6d4760,
// 0x6d4790, 0x6d47a0, 0x6c55c0, 0x6c55a0, 0x6d47f0, 0x6d4860, 0x6c5520 and 0x6c5500; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d4910 (thiscall (source); pops 4): Class703770_Ctor, +8 = +0xc = 0, +0x10 = source and its slot 1 (AddRef) called; vtable
// 0x70ab00. Returns the object.
u8 *__fastcall SoundResource_Ctor(u8 *self, void *, u8 *source);
// 0x6c5450 (thiscall): vtable 0x70ab00; the source's slot 0 (Release) when there is one; vtable 0x703770.
void __fastcall SoundResource_Dtor(u8 *self);
// 0x6c54e0 (thiscall (flags); pops 4; slot 2 of 0x70ab00): SoundResource_Dtor, then the block freed (Mem_Free) for flags bit 0.
// Returns the object.
u8 *__fastcall SoundResource_DeletingDtor(u8 *self, void *, u32 flags);

// 0x6d46e0 (thiscall (out, a, b); pops 0xc; slot 3): the stream sought to (0, 0); unless that answers -1, 4 bytes read into the count
// +0xc; when that read answers 4, +8 = a block of count * 4 bytes (Mem_Alloc) and count * 4 bytes read into it (its answer not looked at),
// *out = 0. Otherwise the object's slot 4 (Unload) and *out = -1. Returns out. (a and b unused.)
u32 *__fastcall SoundStreamIndex_Load(u8 *self, void *, u32 *out, u32 a, u32 b);
// 0x6d4760 (thiscall; slot 4): the stream's slot 1 (close); the block +8 freed (Mem_Free) and cleared when there is one.
void __fastcall SoundStreamIndex_Unload(u8 *self);
// 0x6d4790 (thiscall (i); pops 4; slot 9): the block's offset i - 1 (unchecked).
u32 __fastcall SoundStreamIndex_Offset(const u8 *self, void *, u32 i);
// 0x6d47a0 (thiscall (out, record, i); pops 0xc; slot 6): *out = -1 when i > the count; else the stream sought to (offset i - 1, 0)
// and 0x28 bytes read into *record (the answers not looked at), *out = 0. Returns out.
u32 *__fastcall SoundStreamIndex_ReadRecord(u8 *self, void *, u32 *out, u8 *const *record, u32 i);
// 0x6c55c0 (thiscall): vtable 0x70aa80; SoundStreamIndex_Unload, StreamBase_Dtor (+0x14), then SoundResource_Dtor's work.
void __fastcall SoundStreamIndex_Dtor(u8 *self);
// 0x6c55a0 (thiscall (flags); pops 4; slot 2): SoundStreamIndex_Dtor, the block freed for flags bit 0. Returns the object.
u8 *__fastcall SoundStreamIndex_DeletingDtor(u8 *self, void *, u32 flags);

// 0x6d47f0 (thiscall (out, a, b); pops 0xc; slot 3): +8 = the stream's slot 7 (map); when that is not null, +0xc = its count (its
// first dword), +0x14 = a block of count * 4 bytes (Mem_Alloc) cleared, *out = 0; otherwise the object's slot 4 (Unmap) and *out = -1.
// Returns out. (a and b unused.)
u32 *__fastcall SoundMapIndex_Map(u8 *self, void *, u32 *out, u32 a, u32 b);
// 0x6d4860 (thiscall; slot 4): the stream's slot 1 (close); the block +0x14 freed (Mem_Free) and cleared when there is one.
void __fastcall SoundMapIndex_Unmap(u8 *self);
// 0x6c5520 (thiscall): vtable 0x70aac0; SoundMapIndex_Unmap, StreamBase_Dtor (+0x18), then SoundResource_Dtor's work.
void __fastcall SoundMapIndex_Dtor(u8 *self);
// 0x6c5500 (thiscall (flags); pops 4; slot 2): SoundMapIndex_Dtor, the block freed for flags bit 0. Returns the object.
u8 *__fastcall SoundMapIndex_DeletingDtor(u8 *self, void *, u32 flags);

// 0x6c5240 (stdcall (out, resource, owner, desc, source); pops 0x14; ECX unused): *resource = 0, then by desc's type (its dword +0): 1 a
// stream index (Mem_Alloc 0x1c: SoundResource_Ctor (source), its StreamBase +0x14 a copy of the one desc +8 points to, vtable 0x70aa80);
// 2 a mapped index (0x20: the same, +0x14 = 0 and the StreamBase at +0x18, vtable 0x70aac0); 3, with an owner, a SoundFont bank (0x1c:
// SfBank_Ctor (source, the owner's +4)); *resource = the object (null when the block is). When *resource (read back) is not null, its
// slot 3 (out, desc +0xc, desc +4); otherwise *out = -1. Returns out.
u32 *__stdcall SoundResource_Create(u32 *out, u8 **resource, const u8 *owner, const u32 *desc, u8 *source);
// 0x6c53c0 (stdcall (owner, desc, source), pops 0xc; an SEH frame; ECX unused; name invented): SoundResource_Create with its out and
// resource the desc and source arguments' own slots (sic); a negative answer: 0. Else a SoundResourceRef (Mem_Alloc 0xc) over the new
// resource, or 0 when the block is.
u8 *__stdcall SoundResource_New(const u8 *owner, const u32 *desc, u8 *source);
// 0x6c8b40 (thiscall (resource), pops 4; name invented): Class703770_Ctor, the vtable 0x70af18, +8 = the resource. Returns the object.
u8 *__fastcall SoundResourceRef_Ctor(u8 *ref, void *, u8 *resource);
// 0x6c8b80 (thiscall; an SEH frame; name invented): the vtable 0x70af18, the resource's (+8, not checked) slot 0, the vtable 0x703770.
void __fastcall SoundResourceRef_Dtor(u8 *ref);
// 0x6c8b60 (thiscall (flags), pops 4; name invented): SoundResourceRef_Dtor, the ref freed for flags bit 0. Returns the ref.
u8 *__fastcall SoundResourceRef_DeletingDtor(u8 *ref, void *, u32 flags);
// The ref's forwards to its resource (+8; names invented, after the resource's slots): without one, 0 (or *out = -1). 0x6c8bd0 / 0x6c8bf0
// / 0x6c8be0 (thiscall): the resource's slot 11 / 12 / 13. 0x6c8c00 (thiscall (i), pops 4): its slot 9 (the offset i).
u32 __fastcall SoundResourceRef_Slot11(u8 *ref);
u32 __fastcall SoundResourceRef_Slot12(u8 *ref);
u32 __fastcall SoundResourceRef_Slot13(u8 *ref);
u32 __fastcall SoundResourceRef_Offset(u8 *ref, void *, u32 i);
// 0x6c8c20 (thiscall (out, a), pops 8): the resource's slot 15 (its out the a argument's own stack slot, sic; a), *out = the dword the
// answer points to; without a resource *out = -1. Returns out.
u32 *__fastcall SoundResourceRef_Slot15(u8 *ref, void *, u32 *out, u32 a);
// 0x6c8c60 (thiscall (record, i), pops 8): the resource's slot 6, the record read (its out the i argument's own stack slot, sic; record,
// i): 1 when the dword the answer points to is not negative, else 0; 0 without a resource.
u32 __fastcall SoundResourceRef_ReadRecord(u8 *ref, void *, u32 record, u32 i);
// 0x6bada0 (stdcall (record, handle), pops 8; ECX unused; name invented): a handle of an index (+0) and a resource reference (+4):
// SoundResourceRef_ReadRecord (record, the index) on the reference when both are not 0; else 0 in AL (with no reference, the index's
// upper bytes left in EAX, sic). Called by SoundStream_BuildChain's caller 0x6ba610.
u32 __stdcall SoundResourceHandle_ReadRecord(u32 record, const u32 *handle);

}  // extern "C"
