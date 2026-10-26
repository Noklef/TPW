// A sound category (the statically linked sound library's: cat_<name>BANK.map and cat_<name>SFX.map, tools/sndmap.py): reimplementation of its
// map loading, 0x6bd7a0, 0x6bf150-0x6c017f, 0x6c0190-0x6c0302 and 0x6c0360-0x6c0850 (the SFX.map header check 0x6c0310 is Obj_CheckId, core/methods).
//
// The category object: +0x30 a Handle (the folder its maps are opened in), three std::deque<u32> (dequeiter.h's layout) at +0x38 (A: each
// category's record, a 0x34-byte object whose +0x24 is a PtrList of the banks its sounds use), +0x68 (B: the banks) and +0x98 (C: the BANK.map's
// banks' indices in B), +0xc8 the category being loaded (its index in A) and +0xcc a byte: the SFX.map's pick weights are delta-coded.
//
// SFX.map is a memory image fixed up in place (offsets in bytes): a category (0x18) {u32 sounds, u32 -> its sounds, ...}; a sound (0x14) {u32
// number, u32 variations, u32 -> its variations, u32, u16 flags at +0x10, ...}; a variation (0x2a) {u16 samples, u16, u32 links, u32 -> its samples,
// ..., u16 at +0x10 and +0x12, u32 pick weight at +0x1e, f32, u32 -> its links at +0x26}; a sample (0x10) {..., u16 bank at +0xc, ...}; a link (8)
// {u32 the next variation (1-based), ...}. The categories come first, then each one's sounds, each sound's variations and each variation's samples
// and links, in the order the fixup walks them.
#pragma once

#include "../../../re/types.h"
#include "../files/handles.h"

// What 0x6bf850 / 0x6bfa60 fill in: whether a handle was found and the handle (the folder or WAD opened, or a bank's that shares the folder).
struct BankFolder {
    u8 found;
    Handle handle;
};

extern "C" {

// 0x6bd7a0 (cdecl; a variation; name unconfirmed): its sample count, the low word of its first dword.
u32 __cdecl SfxVariation_SampleCount(const u8 *variation);

// 0x6c0550 (thiscall, `this` unused; a sound, pops 4; name unconfirmed): a sound flagged 4 and 2 with one variation of one sample, no links (its +0x26 0)
// and both words at +0x10 / +0x12 0 is flagged 8 instead of 4.
void __fastcall SfxMap_MarkSimpleSound(u8 *category, void *, u8 *sound);

// 0x6c06e0 (thiscall; data, variation, pops 8; name unconfirmed): the variation's samples at data (+8) and its links after them (+0x26); each sample's
// bank (1-based in the BANK.map, 0 none) becomes deque C's entry for it (its low word), and a bank the category's record (deque A at +0xc8) does not
// list yet is appended to the record's list (+0x24). Returns the end of the links.
u8 *__fastcall SfxMap_FixVariation(u8 *category, void *, u8 *data, u8 *variation);

// 0x6c0610 (thiscall; data, sound, pops 8; name unconfirmed): the sound's variations at data (+8), each fixed up from the end of the last; then,
// with +0xcc set, each variation's pick weight becomes its difference from the one before (the original values chained), and each link's next
// variation number becomes a pointer to that variation. Returns the end of the last variation's links.
u8 *__fastcall SfxMap_FixSound(u8 *category, void *, u8 *data, u8 *sound);

// 0x6c05b0 (thiscall; data, a category record of the image, pops 8; name unconfirmed): its sounds at data (+4), each fixed up from the end of the
// last, then SfxMap_MarkSimpleSound. Returns the end.
u8 *__fastcall SfxMap_FixCategory(u8 *category, void *, u8 *data, u8 *record);

// 0x6c0510 (thiscall; data, out, count, pops 0xc): *out = data, then each of the count category records there is fixed up, the first from the
// end of the records. Returns the end.
u8 *__fastcall SfxMap_Fixup(u8 *category, void *, u8 *data, u8 **out, u32 count);

// 0x6bf150 (thiscall; name unconfirmed): the first index below deque A's count (+0x64) whose entry is 0 (a free category slot), else the count.
u32 __fastcall SoundCategory_FreeCategorySlot(const u8 *category);

// 0x6bf1c0 (thiscall; name unconfirmed): how many of deque B's entries from its start are not 0 (where the first free bank slot is), stopping at
// its end iterator. (A step to a block's end moves to the next block before the end is compared, so an end iterator left at its block's end is
// passed, sic.)
u32 __fastcall SoundCategory_FreeBankSlot(const u8 *category);

// 0x6bf9e0 (thiscall; stream, table, pops 8; name unconfirmed): the rest of BANK.map (the stream's size - its position, the position asked
// twice) read into a heap block, table[0]. table[1] is the entry count: the 11-byte entries come first, then each one's path (a u32 length and
// the chars); each entry's +5 points at its path (then past its last backslash, Path_AfterLastBackslash), +0 and +4 cleared.
void __fastcall SoundCategory_ReadBankEntries(u8 *category, void *, StreamBase *stream, u32 *table);

// 0x6bf850 (thiscall, `this` unused; found, &folder (a String), pops 8; name unconfirmed): with found->found 0, the folder (with a backslash added
// when it does not end in one) up to its last backslash with ".wad" appended is looked for (File_Exists): there, it is opened as a WAD (a Handle
// with vtable 0x6fda18, Handle_OpenE0 (path, 0)), else the folder itself as a folder (vtable 0x6fd9e8, Handle_Open28 (the folder's chars)); when
// that succeeds the handle goes to found->handle and found->found = 1.
void __fastcall SoundCategory_OpenBankFolder(u8 *category, void *, BankFolder *found, const String *folder);

// 0x6bf260 (thiscall; &result, folder (chars), pops 8; name unconfirmed): the Handle at +0x30 reset (its vtable +0), then the folder opened
// (SoundCategory_OpenBankFolder) into it: *result 0, else -1 (the Handle left empty). Returns result.
s32 *__fastcall SoundCategory_SetFolder(u8 *category, void *, s32 *result, const char *folder);

// 0x6bfa60 (thiscall; &index, path, &folder, found, pops 0x10; name unconfirmed): for each bank in deque B (its count at +0x94; empty slots
// skipped), the path of its Handle (+0x24, vtable +0x10 into a String, an empty String the second argument) is compared with the folder (String's
// AssignWide): equal, found gets that Handle (found->found = 1; found->found is cleared first). Then the bank's name (+0xc) is appended to its path
// and path to the folder: equal, the bank's index is pushed onto deque C, *index = it and the result is false. True when no bank matched.
bool __fastcall SoundCategory_FindBank(u8 *category, void *, u32 *index, const char *path, const String *folder, BankFolder *found);

// 0x6bfe40 (thiscall; &result, stream, folder (chars), pops 0xc; name unconfirmed): BANK.map read: its 0x14-byte header (the BANK GUID at
// 0x70a938, else nothing more), 8 bytes (a dword and the entry count), then the entries (SoundCategory_ReadBankEntries). For each entry a String of
// the folder (with its flag 0x20: the String at the sound system's (*0xfb1ffc) +0x30 and "Global\") is given to SoundCategory_FindBank with the
// entry's path: a bank found is AddRef'd (its vtable +4); else a new bank (vtable 0x70a960; its name the path, its flags the entry's +9) gets the
// folder opened (SoundCategory_OpenBankFolder) and, when that worked, the Handle, the first free slot of deque B (or the back) and that slot's index
// pushed onto deque C (a bank whose folder did not open is left, sic). The entries' block is freed. *result = 0 always. Returns result.
s32 *__fastcall SoundCategory_ReadBankMap(u8 *category, void *, s32 *result, StreamBase *stream, const char *folder);

// 0x6c0210 (thiscall; &result, name, unused, pops 0xc; name unconfirmed): a stream opened on name through the Handle at +0x30 (its vtable +0xc:
// &status, &stream, name, 0x80000001, 0); when the status is not negative, the Handle's path (vtable +0x10 into a String, an empty String the
// second argument) is the folder SoundCategory_ReadBankMap reads it in, and the stream is closed. *result = 0 always. Returns result.
s32 *__fastcall SoundCategory_LoadBankMap(u8 *category, void *, s32 *result, const char *name, u32);

// 0x6bf330 (thiscall; &result, folder, name, &index, pops 0x10; name unconfirmed): a category loaded. *index = 0; the folder set
// (SoundCategory_SetFolder) and, with that and a name, the folder + name looked up in the map at +0xc (StdMapH_FindValue into *index): there,
// that category's record (deque A) is AddRef'd (its vtable +4) and *result = 0. Else +0xc8 = the first free category slot, a new record (0x34
// bytes, vtable 0x70a950: Class703770_Ctor, +8 0, +0xc a String, +0x20 the slot, +0x24 an empty PtrList) named after the folder from its last
// "\data\" on (String_Right of the length - that position) + the name goes to deque A's slot (or the back), then <name>BANK.map
// (SoundCategory_LoadBankMap) and <name>SFX.map (SoundCategory_ReadSfxMap into *index) are read: when both worked, deque C is emptied
// (Deque32_Erase), map[*index] = folder + name (StdMapH_AssignWide) and *result = 0. Otherwise (the folder, no name, either read failing)
// deque C is emptied by the inline erase (the elements after the end moved down, Deque32_Copy, and the back popped once each, Deque32_PopBack)
// and *result = -1 (the record stays). Returns result.
s32 *__fastcall SoundCategory_Load(u8 *category, void *, s32 *result, const char *folder, const char *name, u32 *index);

// 0x6c01b0 (thiscall; name unconfirmed): a bank (0x2c bytes, vtable 0x70a960: Release 0x6338d0, AddRef 0x6338c0 and this; +4 its count, +8, +0xc
// its name (a String), +0x20 its BANK.map flags, +0x24 its folder's Handle) destroyed: the Handle, the name, then the base vtable 0x703770.
// 0x6c0190 (thiscall; flags, pops 4): that, then freed with bit 0 of the flags (Mem_Free). Returns the bank.
void __fastcall Bank_Dtor(u8 *bank);
u8 *__fastcall Bank_DeletingDtor(u8 *bank, void *, u32 flags);
// 0x6c0f70 (thiscall (name), pops 4; name invented): the index in deque A of the first category record (nonzero) whose +8 is nonzero and
// whose String +0xc equals name (String_EqualsStr); 0 when none does (sic: as the first's).
u32 __fastcall SoundCategory_FindRecordIndex(u8 *category, void *, const char *name);
// 0x6bee10 (thiscall; names invented): the destructor: the vtable 0x70a95c; deque A's first record's slot 0 (the release; at its begin
// iterator moved on by 0: DequeIter_Advance), the others' not (sic); deques C, B and A emptied element by element, each block freed as it
// empties (Deque32_FreeFront, inlined for C and B); the folder Handle (+0x30) destroyed, tree J (+0x1c, StdTree_DtorJ inlined) and map H
// (+0xc, StdMapH_ClearDtor).
void __fastcall SoundCategory_Dtor(u8 *category);
// 0x6bed80 (thiscall (flags), pops 4; slot 0 of 0x70a95c): the destructor, then Mem_Free with flags bit 0. Returns the category.
u8 *__fastcall SoundCategory_DeletingDtor(u8 *category, void *, u32 flags);
// 0x6beb10 (atexit, from 0x6bead0's guard): the destructor on the singleton at 0xfb1f28 (src/game/core/soundnames' table).
void __cdecl SoundCategory_SingletonDtor();
// 0x6c2f40 (thiscall; also an unwind funclet's at 0x6e5e19; names of both invented): the destructor of a category record's PtrList of banks
// (+0x24; vtable 0x70a948, the deleting one its slot 0): its nodes (+0 the bank, +4 the next) freed, the banks left; vtable reset, +8, +4
// and the count (+0xc) 0.
void __fastcall BankList_Dtor(u8 *list);
// 0x6c30c0 (thiscall; flags, pops 4): that, then the list freed with bit 0 of the flags. Returns the list.
u8 *__fastcall BankList_DeletingDtor(u8 *list, void *, u32 flags);

// 0x6c0360 (thiscall; &result, stream, &index, pops 0xc): reads the 0x14-byte header (Obj_CheckId; another GUID: *result = -1), then 8 bytes
// (a dword and the category record count), then the rest of the stream (its size - its position, through its vtable +0x14 / +0x18) into a block
// from the heap, which SfxMap_Fixup fixes up with the image pointer going to the current category's record (deque A at +0xc8) +8. *index = +0xc8,
// *result = 0. Returns result. (The reads' results are not checked: a short read leaves the locals as they were.)
s32 *__fastcall SoundCategory_ReadSfxImage(u8 *category, void *, s32 *result, StreamBase *stream, u32 *index);

// 0x6c0450 (thiscall; &result, name, &index, pops 0xc): a stream opened on name through the Handle at +0x30 (its vtable +0xc: &status, &stream,
// name, 0x80000001, 0) and, when the status is not negative, SoundCategory_ReadSfxImage; the stream closed after that. *result = 0 when both
// succeeded, else -1. Returns result.
s32 *__fastcall SoundCategory_ReadSfxMap(u8 *category, void *, s32 *result, const char *name, u32 *index);

}  // extern "C"
