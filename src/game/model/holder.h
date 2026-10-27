// Model holders (flags +0, model +4, slot table +0x28: +0 flags, +4 slots of 0x14 bytes,
// +8 count): reimplementation of 0x45bdc0, 0x45bed0-0x45bf70 and 0x4587b0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x45bdc0: flag 1 (+0), set while the holder is awake (Animator_Play wakes it).
u32 __cdecl Holder_IsAwake(const u8 *holder);

// 0x45bed0: sets flag 0x200 in the model's +0x30 (the holder reached through *ref).
void __cdecl Holder_FlagModel(const u8 *const *ref);

// 0x45bef0: the model's matrix (+0x78, from +0x10) translation (+0x30) to x, y, z.
void __cdecl Holder_SetPosition(const u8 *holder, u32 x, u32 y, u32 z);

// 0x45bf20: the model's matrix (+0x78, from +0x10) axes scaled by scale's x, y, z
// (Matrix_ScaleAxes).
void __cdecl Holder_SetScale(const u8 *holder, const float *scale);

// 0x45bf50, 0x45bf70: the unit's unused statics, 0x7a1cac = 1.0f and 0x7a1cdc = 1 / 2^23.
void __cdecl Holder_InitUnusedOne();
void __cdecl Holder_InitUnusedEpsilon();

// 0x4587b0: slot i's +8 of ride index's holder (table 0x7a4610 -> +8), or 0 when the slot's
// address comes to null.
u32 __cdecl Ride_SlotKey(u32 ride, u32 i);
// 0x45bdd0 (cdecl; the holder): the part at +0x1c (null, or the pointer adjusted by the virtual base offset in the word at +4 of its vtable) leaves the renderer's slot list of
// [0x87a6d0] (tag 0x10 when flag 0x10 is set, else 0x20 when flag 8 is set, else 0x70); the flags lose 0x19.
void __cdecl Holder_Sleep(u8 *holder);

// 0x4254c0 (cdecl; the animator): the holder at +0x24 sleeps (Holder_Sleep) and flag 0x4000 of the animator's flags (+0) is cleared. Returns 1.
u32 __cdecl Animator_Sleep(u8 *animator);

// 0x45bbc0 (cdecl; the holder): its part made: a 0x158-byte block (Mem_Alloc) constructed (LiveObject7019f8_Construct, most derived;
// none when the allocation fails, and the load below then faults in the game too) and given the holder's model (+4; ObjModel_Load, its
// result ignored); the count at +0x34 of the owner (the holder at +8, else itself) raised and the part stored at +0x1c.
void __cdecl Holder_Create(u8 *holder);

// 0x46dd50 (cdecl; the holder; name unconfirmed): as Holder_Create, but the part is stored at +0x20, before its load, and no count is
// raised.
void __cdecl Holder_CreateAt20(u8 *holder);

// 0x45bda0 (cdecl; holder, matrix; name unconfirmed): the holder's part (+0x1c, unadjusted) gets the matrix as its first
// (ObjModel_SetMatrices with 1).
void __cdecl Holder_SetMatrix(u8 *holder, const u32 *matrix);

// 0x45bd30 (cdecl; holder, matrix, grid): the holder's part (+0x1c, unadjusted) gets the matrix as its first (ObjModel_SetMatrices with
// 1); then it is added to the renderer ([0x87a6d0]; Renderer_Add, the part reached through the virtual base offset in the word at +4 of
// its vtable, none for no part) under the tag 0x10 and the holder's flags get 0x11 (grid set), else under 0x20 with 9.
void __cdecl Holder_WakeWithMatrix(u8 *holder, const u32 *matrix, u32 grid);

// 0x425440 (cdecl; the animator; name unconfirmed): with a matrix at +0xc the holder at +0x24 wakes with it (Holder_WakeWithMatrix, grid
// 0), else plainly (Holder_Wake); the animator's flag 0x4000 set. Returns 1.
u32 __cdecl Animator_Wake(u8 *animator);

// 0x45bc60 (cdecl; the holder): with flag 1 the holder's part sleeps (Holder_Sleep); the count at +0x34 of the owner (the holder at +8, else itself) is lowered; the part at +0x1c
// (adjusted by the virtual base offset in the word at +4 of its vtable) is deleted through its slot 0 (with 1) and the pointer cleared.
void __cdecl Holder_Destroy(u8 *holder);

// 0x5d8820 (thiscall on a user of the holder of the object behind [0x79fcb4]): when the dword at +4 is set, the target's holder (its +8) is taken out of the renderer, by its rectangle
// (Obj_RemoveFromRect with the words at +0xc0 and +0xc4) when bit 0x4000 of the target's flags (+4) is set and else by Holder_Sleep; then the dword at +4 is cleared.
void __fastcall HolderUser_Release(u8 *user);

// 0x45bcb0 (cdecl; holder, from, to): the part at +0x1c (null, or adjusted by the virtual base offset in the word at +4 of its vtable) is added to the renderer's grid
// ([0x87a6d0]; Grid_AddObject) over the rectangle whose corners are the high and low words of from and to, with the word 0x70; the holder's flag 1 is set.
void __cdecl Holder_AddToGrid(u8 *holder, u32 from, u32 to);

// 0x455a50 (cdecl; the holder): Holder_Sleep when flag 1 is set, Holder_Destroy, then the block at +0x24 is freed (GlobalMem_Free). Returns 1.
u32 __cdecl Holder_Dispose(u8 *holder);

// 0x455a80 (cdecl; the owner): the same for the holder at +0x24 of the owner, then the dword at +0x1c of the object at +0x3c is lowered and the block at +0x34 freed. Returns 1.
u32 __cdecl HolderOwner_Dispose(u8 *owner);

// 0x425490 (cdecl; animator, from, to): Holder_AddToGrid of the holder at +0x24 over the rectangle, then flag 0x4000 of the animator is set. Returns 1.
u32 __cdecl Animator_AddToGrid(u8 *animator, u32 from, u32 to);

// 0x43eac0 (cdecl; owner): while the owner has a first child (+0x38) it is unlinked (the owner's pointer takes the child's own link at +0x38) and disposed of (HolderOwner_Dispose).
void __cdecl Owner_DisposeChildren(u8 *owner);

// 0x45bd00 (cdecl; the holder; name unconfirmed): the inverse of Holder_Sleep: the part at +0x1c (null, or adjusted by the virtual base offset in the word at +4 of its vtable) is added to
// the renderer's slot list of [0x87a6d0] with the word 0x70 (Renderer_Add); the holder's flag 1 is set.
void __cdecl Holder_Wake(u8 *holder);

// 0x5d87d0 (thiscall on a user of the holder of the object behind [0x79fcb4]; name unconfirmed): the inverse of HolderUser_Release: when the dword at +4 is clear, the target's holder (its +8)
// is put back, into the grid rectangle (Holder_AddToGrid with the words at +0xc0 and +0xc4) when bit 0x4000 of the target's flags (+4) is set and else by Holder_Wake; the dword at +4 is set.
void __fastcall HolderUser_Acquire(u8 *user);

// 0x5d8130 (thiscall; name unconfirmed): the same for a user whose target is the object in table 0x7a4610 at index [user+4], with the done flag at +8.
void __fastcall RideUser_Acquire(u8 *user);

// 0x425510 (cdecl; the animator; name unconfirmed): the holder at +0x24 sleeps when the animator has flag 0x4000 and is destroyed (Holder_Destroy); the count at +0x1c of the
// object at +0x3c is lowered, and when it reaches 0 the +0xc blocks of the holder's model's (+4) +0x36 (u16) 16-byte records at +0x54 are freed and cleared; then the
// block at +0x20 is freed and cleared and flag 0x2000 cleared. Returns 1.
u32 __cdecl Animator_Dispose(u8 *animator);

// 0x455ad0 (cdecl; the instance; name unconfirmed): the instance's owner at +0xb4 (when set) leaves the grid rectangle (+0xc0, +0xc4) when it has flag 0x4000
// (Obj_RemoveRectAndClear14), is disposed of (HolderOwner_Dispose) and cleared; the +0x44 blocks of the +0xa8 parts at +0xac are freed; the holder at +8, when awake, is
// taken out of the renderer (by the rectangle when the instance's flags at +4 have 0x4000, else Holder_Sleep) and disposed of (Holder_Dispose); the block at +0xe0 is freed.
// Returns 1.
u32 __cdecl Instance_Dispose(u8 *instance);

// 0x4255b0 / 0x4255d0 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x78ed8c = 1 and 0x78f47c = 1 / 2^23 (as
// camera's).
void __cdecl Animator_InitUnusedOne();
void __cdecl Animator_InitUnusedEpsilon();

// 0x455bf0 / 0x455c10 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x7a0174 = 1 and 0x7a086c = 1 / 2^23 (as
// camera's).
void __cdecl Instance_InitUnusedOne();
void __cdecl Instance_InitUnusedEpsilon();

// 0x45bac0 (cdecl; the holder; name unconfirmed): the holder freed: Holder_Sleep with flag 1, Holder_Destroy when it has a part (+0x1c);
// the object model at +0x20, when the count at +0x34 is then 0, unloaded (ObjModel_Unload) and deleted through its slot 0 (with 1, the
// pointer adjusted by the virtual base offset in the word at +4 of its vtable; +0x20 kept); the blocks at +0x98 of the model's (+4)
// parts (+0x70, 0xa0 bytes each, the u16 count at +0x44) with flag 0x1000 freed but for the part at +0x2c; the block at +0x14 freed and
// cleared, the part states (PartStates_Free), the model's block at +0xb0, +4 cleared and the holder itself (GlobalMem_Free each).
void __cdecl Holder_Free(u8 *holder);

// 0x463db0 (cdecl; the instance, kept; name unconfirmed): the instance freed: the +0xb0 blocks (GlobalMem_FreeAtB0) of the +0xa8 objects
// of the array at +0xdc; the blocks at +0xac, +0xdc and +0xd8, each cleared; the animator at +0xb4, when there is one, disposed of
// (Animator_Dispose), its block +8 and itself freed and cleared; the object at +0xb0, when there is one, its block +0xc and itself freed
// and cleared; when kept is 0 the block at +0xcc freed (Instance_FreeBlockCC), else +0xcc cleared; the holder at +8 (Holder_Free) and
// the instance itself freed.
void __cdecl Instance_Free(u8 *instance, u32 kept);

// 0x463ed0 (cdecl; name unconfirmed): the instances of the type table at 0x7ae6a8 (entries [0x7a439c] down to 1) freed: the one at
// +0xd0 of each, when there is one, freed with its +0xcc block kept (Instance_Free) and cleared; the holder's block (Instance_FreeHolderBlock)
// and the record block (Instance_FreeRecordBlock) freed, then the instance (Instance_Free). The count is then 0.
void __cdecl InstanceTypes_FreeAll();

// 0x45be80 (cdecl; holder, from, to): the part at the holder's +0x1c (a virtual base: the pointer plus the offset at +4 of its vtable; 0
// for none) is removed from the renderer's grid ([0x87a6d0]) over the rectangle from.hi, from.lo to to.hi, to.lo (the positions' high
// and low words as x and y of the corners: Renderer_RemoveInRect); then bit 0 (awake) of the holder's first dword is cleared. Returns
// that dword.
u32 __cdecl Obj_RemoveFromRect(u8 *object, u32 from, u32 to);

// 0x4254e0 (cdecl; animator, from, to): Obj_RemoveFromRect of the animator's holder at +0x24 (from, to); then bit 14 (0x4000, in the
// grid) of the animator's flags at +0 is cleared. Returns 1.
u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to);

// 0x424ed0 (cdecl; pointer): bit 0 of the dword (a holder's awake flag) cleared; returns 1.
u32 __cdecl Dword_ClearBit0(u32 *p);

// 0x424ef0 (cdecl; animator, record, name, flags; name unconfirmed): the animator's mesh built in one zeroed block (GlobalMem_Alloc,
// the inverse of Animator_Dispose). The animator's u16 node count (+4; parts: 1 for one node, else nodes + 1) and its 0x30-byte nodes
// (+8) give the totals at +0x2a (+6 of each node), +0x2e (+4), +0x2c (+0xa), +0x30 (+8) and +0x32 (+0xc, plus +6 unless the node's
// flag 4); +0x28 is the record's u16 count (+4); the record (+0x3c) is shared (bit 0 of its flags: its use count +0x1c raised) or
// owned (its count set to 1, the animator's flag 0x8000, its records' names copied into blocks of their own). The block: a 0xb8-byte
// header (the name's 0x14 chars at +0x18 when given), the parts (0xa0 bytes; +0x70 and +0x78) and the tables (+0x50 / +0x54, +0x58,
// +0x5c, +0x60, +0x68, +0x64; each only when its count is not 0), then a u16 index per vertex (flags 1: numbered from 0 per part).
// Several parts are chained to the first (its matrix the identity). The holder (+0x24, GlobalMem_AllocCounted with the block) is made
// (Holder_Create) after the texture directories are set (TexDirs_SetForModel with the record); the animator's flag 0x2000 (and 8 with
// flags 1). Returns 1.
u32 __cdecl Animator_Create(u8 *animator, u8 *record, const char *name, u32 flags);

// 0x455690 (cdecl; animator, unused, flags, out; name unconfirmed): a copy of the animator in a block of its own (GlobalMem_Alloc, not
// zeroed; the copy at the block aligned to 8, its nodes after it at +0x40, the block at +0x34): its 0x40 bytes (unless the block or the
// animator is null) and its u16 count (+4) of 0x30-byte nodes; the holder cloned (Holder_Clone with unused, [0x79fb2c] for 0, flags,
// into the copy's +0x24, mask 0) and made (Holder_Create); the tables +0x10-+0x1c from the new model's +0x58, +0x5c, +0x60 and +0x68;
// the nodes' +0x28 and +0x2c the parts' matrices and +0x6c (from the model's +0x70, the first skipped for more than one node); the
// record's (+0x3c) use count +0x1c raised. *out = the copy; returns Holder_Clone's result.
u32 __cdecl Animator_Clone(const u8 *animator, u32 unused, u32 flags, u8 **out);

// 0x4557c0 (cdecl; instance, heap, flags, out, mask; name unconfirmed): 0 for a null instance. Otherwise flag 0x800 cleared, the holder (+8)
// cloned (Holder_Clone with heap, flags, mask) and the animator (+0xb4) when set (Animator_Clone with heap, 0x21df); one block
// (GlobalMem_Alloc, not zeroed) of 0xfc bytes, the u16 count (+0xe) of 0x38-byte entries and, with parts (+0xa8), 12 bytes a part (sic: the
// index arrays may need more): the 0xfc bytes copied, +8 / +0xb4 the clones, +0xe0 the block, +0 the instance, +0x10 the entries after the
// header (copied when +0x10 and the count are set). With parts, the table after the entries (aligned to 4, at +0xac) gets each non-null
// part cloned (ModelPiece_Clone with heap, [0x79fb2c] for 0, and flags), and each of the twelve (count, entries) arrays from +0x1c with a
// count is copied after it (unless its entries are null) and its entries' second dwords set to the parts their first dwords index.
// Holder_Create (through the mesh env) on the new holder for flag 0x2000. *out = the block; returns 1.
u32 __cdecl Instance_Clone(const u8 *instance, u32 heap, u32 flags, u8 **out, u32 mask);
}

// Animator_Create's calls out (and Animator_Clone's holder) (the game's, i.e. the ports', unless swapped; tools/fncmp's driver logs them).
struct AnimatorMeshEnv {
    void(__cdecl *setTexDirs)(u8 *info, u8 *model);  // 0x46e4e0 TexDirs_SetForModel
    void(__cdecl *createHolder)(u8 *holder);          // 0x45bbc0 Holder_Create
};
// nullptr restores the game's.
void Animator_SetMeshEnv(const AnimatorMeshEnv *env);
