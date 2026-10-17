// The WAD archive source (0x61a060-0x61ae00, vtable 0x703908): a RefObject2 over one .wad
// file. Open takes "archive.wad" or "archive.wad*\folder" ('/' made '\'), opens the file through
// its own FileStream (vtable +0x24, 0x5f7ed0), wraps it in a shared source, checks the 0x58-byte
// "DWFB" header (version 0-2; 2 keeps the name block whole) and loads the directory tree
// (waddir.h). FindFirst/FindNext search the tree (wadfind.h) under the folder; OpenEntry seeks
// the archive to the entry's 0x28-byte record and hands the stream a slice of the archive, then
// (record flag 2) a zlib or (flag 4) RefPack buffer that decodes the slice whole
// (wrapsources.h). Flag 1 marks an entry that can't be opened.
//
// The originals' quirks are kept: Open ignores the path resolver's and the directory load's
// results; a failed header or folder check closes the file but leaves the source pointer set
// over a cleared tree, so a later FindFirst/FindNext/OpenEntry walks a null finder; the folder
// check needs the folder to be found as a folder; the inBlock word passed
// to the directory load is the version test in the low byte over the shared source's pointer
// (Open) or the version (OpenEntry); the left/right/from views of the path are never freed (the
// views own no memory); FindFirst builds the path plus pattern and drops it, assigns the folder
// to its pattern through the wide-string assign (it copies length bytes either way), scans the
// pattern back for its folder one byte before its start, and points the entry's name at the
// pattern's offset into the entry's path even when that path held text already; OpenEntry
// leaves the stream on the slice when the record is flagged 1, and on the shared source when
// the slice fails; the entry's buffer and slice are made even when their allocation failed.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"
#include "dirsource.h"
#include "handles.h"
#include "waddir.h"

struct WadHeader {
    u32 magic;  // "DWFB" (0x42465744)
    u32 version;
    u8 rest[0x50];
};
static_assert(sizeof(WadHeader) == 0x58, "WadHeader is 0x58 bytes");

struct WadRecord {
    u32 hash;
    u32 unknown[2];
    u32 offset;    // +0xc: of the entry's data in the archive
    u32 size;      // +0x10: in the archive
    u32 flags;     // +0x14: 1 can't open, 2 zlib, 4 RefPack (the low byte is read)
    u32 unpacked;  // +0x18
    u32 rest[3];
};
static_assert(sizeof(WadRecord) == 0x28, "WadRecord is 0x28 bytes");

struct WadSource {
    RefObject2 base;    // vtable 0x703908; base.path is the archive path plus "*"
    String folder;      // +0x24: the folder inside the archive, ending '\' ("" for none)
    s32 unknown38;      // -1
    StringList list;    // +0x3c: vtable 0x70393c, unused here
    String pattern;     // +0x4c: FindFirst's folder plus pattern
    WadHeader header;   // +0x60
    StreamBase file;    // +0xb8: a FileStream (vtable 0x6fd930)
    RefObject *source;  // +0xc0: the archive file's source, while open
    WadDir dir;         // +0xc4
};
static_assert(sizeof(WadSource) == 0xe0, "WadSource is 0xe0 bytes");

extern "C" {
WadSource *__fastcall WadSource_Ctor(WadSource *w);
void __fastcall WadSource_Dtor(WadSource *w);
WadSource *__fastcall WadSource_DeletingDtor(WadSource *w, void *edx, u32 flags);
// 0x61a200 (vtable +0x30): the tree cleared, the file closed, both paths emptied.
void __fastcall WadSource_Close(WadSource *w);
// 0x61a250 (&result, path, flags): flag 0x8000 adds 0x8000 to the file's open mode
// (0x80000001). 0 or -1.
s32 *__fastcall WadSource_Open(WadSource *w, void *edx, s32 *result, const char *path, u32 flags);
// 0x61a550 (&result, pattern, entry; vtable +8): 0 or -1; entry->path holds the pattern on
// entry (usually empty), the match's path inside the folder after.
s32 *__fastcall WadSource_FindFirst(WadSource *w, void *edx, s32 *result, const char *pattern, DirEntry *e);
s32 *__fastcall WadSource_FindNext(WadSource *w, void *edx, s32 *result, DirEntry *e);  // 0x61a700 (+0xc)
// 0x61a6c0: the folder's length - 1 characters dropped from the front of entry->path.
void __fastcall WadSource_FillEntry(const WadSource *w, void *edx, DirEntry *e);
// 0x61a760 (&result, stream, name, flags, unused; vtable +4): -1 unless open, flags clear of
// 0x40000012 and the stream without a source; then OpenEntryStream.
s32 *__fastcall WadSource_OpenEntry(WadSource *w, void *edx, s32 *result, StreamBase *stream, const char *name,
                                    u32 flags, u32 unused);
// 0x61a800 (&result, stream, name): the stream on the archive's source gets a shared source,
// then the entry's slice, then its decoder. 0 or -1.
s32 *__fastcall WadSource_OpenEntryStream(WadSource *w, void *edx, s32 *result, StreamBase *stream,
                                          const char *name);
// The unsupported slots: 0x61adb0 (+0x1c, +0x28; 2 args) -1, 0x61adc0 (+0x20; 1 arg) 0,
// 0x61add0 (+0x24; 1 arg) -1.
s32 *__fastcall WadSource_Vfn7(WadSource *w, void *edx, s32 *result, u32 a, u32 b);
s32 *__fastcall WadSource_Vfn8(WadSource *w, void *edx, s32 *result, u32 a);
s32 *__fastcall WadSource_Vfn9(WadSource *w, void *edx, s32 *result, u32 a);

// A list of Strings: 0x45d820 destructor (vtable 0x6fe680, nodes freed), 0x61aeb0 clear (the
// same without the vtable), 0x61a7e0 (flags) the deleting destructor of vtable 0x70393c and
// 0x6fe67c, 0x61af00 (flags) a node's deleting destructor.
void __fastcall StringList_Dtor(StringList *l);
void __fastcall StringList_Clear(StringList *l);
StringList *__fastcall StringList_DeletingDtor(StringList *l, void *edx, u32 flags);
StringListNode *__fastcall StringListNode_DeletingDtor(StringListNode *n, void *edx, u32 flags);
// 0x45f8e0 (thiscall; flags): the deleting destructor of vtable 0x6fe680 (its one slot), the same as 0x61a7e0 with the
// destructor inline.
StringList *__fastcall StringListBase_DeletingDtor(StringList *l, void *edx, u32 flags);
}

struct WadSourceEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
};
// The calls go through env (nullptr: the game's).
void WadSource_SetEnv(const WadSourceEnv *env);
