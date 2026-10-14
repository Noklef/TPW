// The WAD archive's directory tree: reimplementation of 0x637460-0x638010 and 0x638640-
// 0x638840 (its pools and lists), with the finder's destructor 0x61ade0.
//
// A WadDir reads a WAD's 0x58-byte header and 0x28-byte entry records from a stream and
// files every entry into a tree of folders by its lower-cased, '\'-separated name. The
// folders (0x28 bytes) and files (0x14 bytes) come out of pools of 100-element `new[]`
// blocks; each folder lists its subfolders (newest first) and its files (in order). Names are
// either copies or, when the WAD's name block is loaded whole, pointers into it. The finder
// (0x60 bytes) walks the tree for a pattern (see wadfind.h).
//
// The originals' quirks are kept: an entry without a folder goes into the previous entry's
// folder (the root only for the first); without the name block every name is read into a
// buffer that is copied again and never freed; a failed name-block read frees the block but
// keeps the pointer, so the next Load or Clear frees it again; Load replaces the root and the
// finder but never empties the pools or frees the old finder or name block; a found folder's
// separator is not cut from an in-block name.
#pragma once

#include "../../../re/types.h"
#include "../core/ptrlist.h"
#include "../core/string.h"
#include "handles.h"
#include "textwriter.h"

struct WadFile {
    char *name;
    u8 owned;  // name is a copy (freed by the destructor)
    u32 size;  // the unpacked size when the entry is packed
    u32 offset;  // of the entry's record in the WAD (0x58 + 0x28 * index)
    u32 hash;  // the record's first word: its name's hash (see wadfind.h)
};
static_assert(sizeof(WadFile) == 0x14, "WadFile is 0x14 bytes");

struct WadFolder {
    char *name;
    u8 owned;
    PtrList folders;  // vtable 0x703e94 (base 0x703e98), items WadFolder *
    PtrList files;  // vtable 0x703e90 (base 0x703e9c), items WadFile *
};
static_assert(sizeof(WadFolder) == 0x28, "WadFolder is 0x28 bytes");

// A chain of `new T[100]` blocks, the newest first: {next, items}; used counts the newest.
struct WadPoolBlock {
    WadPoolBlock *next;
    void *items;
};
struct WadPool {
    WadPoolBlock *block;
    s32 used;
};

// A list of Strings, each node a String and the next and previous nodes.
struct StringListNode {
    String text;
    StringListNode *next;
    StringListNode *prev;
};
struct StringList {
    const void *vtable;  // 0x6fe680 (base), 0x6fe67c, 0x70393c
    StringListNode *head;
    StringListNode *tail;
    u32 count;
};

struct WadFinder {
    String pattern;
    WadFolder *folder;
    PtrListNode *cursor;
    PtrListNode *current;
    String path;
    u8 folders;  // +0x34: still walking the subfolders
    u8 exact;  // +0x35: the pattern has no wildcard
    u32 hash;  // +0x38: of an exact pattern
    StringList a;  // +0x3c
    StringList b;  // +0x4c
    u8 dotStar;  // +0x5c
};
static_assert(sizeof(WadFinder) == 0x60, "WadFinder is 0x60 bytes");

struct WadDir {
    WadFolder *root;
    char *names;  // the WAD's name block, when loaded whole
    WadFinder *finder;
    WadPool folderPool;
    WadPool filePool;
};
static_assert(sizeof(WadDir) == 0x1c, "WadDir is 0x1c bytes");

extern "C" {
// __thiscall. 0x637460: empty, each pool holding one block. 0x637550: Clear, then the pools.
// 0x637630: frees the root folder, the name block and the finder.
WadDir *__fastcall WadDir_Ctor(WadDir *d);
void __fastcall WadDir_Dtor(WadDir *d);
void __fastcall WadDir_Clear(WadDir *d);
// 0x637690 (&result, stream, inBlock): a new root and finder, then the header (seek 0, read
// 0x58), the records and, inBlock, the name block; every entry filed by its name. 0 or -1.
s32 *__fastcall WadDir_Load(WadDir *d, void *edx, s32 *result, StreamBase *stream, u32 inBlock);
// 0x637a10, cdecl: the writer's text lower-cased with '/' made '\'; the writer.
TextWriter *__cdecl WadDir_Normalize(TextWriter *w);

// 0x637f60 / 0x637ae0: a nameless folder; the destructor frees its name (when owned) and its
// list nodes, not the folders and files they hold.
WadFolder *__fastcall WadFolder_Ctor(WadFolder *f);
void __fastcall WadFolder_Dtor(WadFolder *f);
// 0x637b50 (name, size, offset, hash, &filePool, inBlock): a file from the pool, appended.
void __fastcall WadFolder_AddFile(WadFolder *f, void *edx, char *name, u32 size, u32 offset, u32 hash,
                                  WadPool *pool, u32 inBlock);
// 0x637be0 (path, &folderPool, inBlock): the folder for path ('\' or '/' separated) under f,
// each missing one made (its separator cut to a NUL) and put first in its parent's list.
WadFolder *__fastcall WadFolder_AddPath(WadFolder *f, void *edx, char *path, WadPool *pool, u32 inBlock);

// 0x637f30 / 0x637f40: a file without a name; the destructor frees an owned one.
WadFile *__fastcall WadFile_Ctor(WadFile *f);
void __fastcall WadFile_Dtor(WadFile *f);

// The folder and file lists' destructors: 0x637fb0 / 0x638010 free the nodes (vtables
// 0x703e98 / 0x703e9c); 0x637f90 / 0x637ff0 (flags; vtables 0x703e94 / 0x703e90) and the
// base ones 0x6387a0 / 0x6387f0 (flags) also free the list when flags & 1.
void __fastcall WadFolderList_Dtor(PtrList *l);
void __fastcall WadFileList_Dtor(PtrList *l);
PtrList *__fastcall WadFolderList_DeletingDtor(PtrList *l, void *edx, u32 flags);
PtrList *__fastcall WadFileList_DeletingDtor(PtrList *l, void *edx, u32 flags);
PtrList *__fastcall WadFolderListBase_DeletingDtor(PtrList *l, void *edx, u32 flags);
PtrList *__fastcall WadFileListBase_DeletingDtor(PtrList *l, void *edx, u32 flags);

// 0x638660 / 0x6386b0: every folder / file block destroyed (last element first) and freed.
// 0x638700: the next file, a new block first when the newest is full.
void __fastcall WadFolderPool_Free(WadPool *p);
void __fastcall WadFilePool_Free(WadPool *p);
WadFile *__fastcall WadFilePool_Alloc(WadPool *p);

// 0x638640 / 0x6380d0: empty string lists (vtables 0x6fe680 / 0x6fe67c). 0x61ade0: the
// finder's destructor (both lists' Strings, then its two Strings).
StringList *__fastcall StringList_Ctor(StringList *l);
StringList *__fastcall StringList_Ctor2(StringList *l);
void __fastcall WadFinder_Dtor(WadFinder *f);
}

struct WadDirEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
    s32(__cdecl *tolower)(s32 c);      // 0x67fa60
};
// The tree's calls go through env (nullptr: the game's).
void WadDir_SetEnv(const WadDirEnv *env);
