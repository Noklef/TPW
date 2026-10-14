// The directory source (vtable 0x7038d0): reimplementation of 0x619950-0x619fb0.
//
// The RefObject2 Handle_Open28 makes: mounted on a folder (its path, kept with a trailing
// separator), it opens files in it as stream sources, lists it with FindFirst/NextFileA and
// sets attributes, makes, removes and renames there, each by the folder's path + the name
// given (no further resolving, so a name may climb out of it).
//
// The originals' quirks are kept: FindFirst without a pattern leaves an earlier search
// closed and reports -1; the entry's name points into its own copy of the pattern, after
// the last separator, and the file's name is copied over the rest; Open's failed
// allocation hands a null source to the open and the release (as the original, a crash).
#pragma once

#include "../../../re/types.h"
#include "handles.h"

struct DirSource {
    RefObject2 base;
    void *find;  // the FindFirstFileA search, or INVALID_HANDLE_VALUE
};
static_assert(sizeof(DirSource) == 0x28, "DirSource is 0x28 bytes");

struct FindData {  // WIN32_FIND_DATAA
    u32 attributes;
    u32 creationTime[2];
    u32 accessTime[2];
    u32 writeTime[2];
    u32 sizeHigh;
    u32 sizeLow;
    u32 reserved[2];
    char name[260];
    char alternateName[14];
};
static_assert(sizeof(FindData) == 0x140, "FindData is 0x140 bytes");

struct DirEntry {
    u32 size;
    u32 attributes;
    u32 writeTime[2];
    u32 accessTime[2];
    u32 creationTime[2];
    char *name;    // into path, after the pattern's folder
    char path[1];  // the pattern, then the name over its last part
};

extern "C" {
// __thiscall. 0x619950: no search. 0x619990 / 0x619970 (flags; vtable +0): destructors,
// closing the search.
DirSource *__fastcall DirSource_Ctor(DirSource *d);
void __fastcall DirSource_Dtor(DirSource *d);
DirSource *__fastcall DirSource_DeletingDtor(DirSource *d, void *edx, u32 flags);
// 0x619c20 (&result, stream, name, mode, unused; vtable +4): -1 when the stream has a source;
// else a CreateFileA source (mode & 0x8000: an mmio one, with the bit cleared) opened on the
// folder + name and attached, 0; when the open fails it is released, -1.
s32 *__fastcall DirSource_Open(DirSource *d, void *edx, s32 *result, StreamBase *stream, const char *name, u32 mode,
                               u32 unused);
// 0x6199e0 (&result, pattern, entry; +8) / 0x619b30 (&result, entry; +0xc): the search (the
// last one closed through vtable +0x34 first); 0 with the entry filled, or -1.
s32 *__fastcall DirSource_FindFirst(DirSource *d, void *edx, s32 *result, const char *pattern, DirEntry *entry);
s32 *__fastcall DirSource_FindNext(DirSource *d, void *edx, s32 *result, DirEntry *entry);
// 0x619ba0 (+0x34): FindClose when searching.
void __fastcall DirSource_Close(DirSource *d);
// 0x619bc0 (entry, findData; ECX unused): sizes, attributes and times, then the name.
void __stdcall DirSource_FillEntry(DirEntry *entry, const FindData *find);
// 0x619d70 (&result, path; Handle_Open28's open): the resolved path, when it exists, becomes
// the folder (a '\' added unless it ends in one); else -1.
s32 *__fastcall DirSource_Mount(DirSource *d, void *edx, s32 *result, const char *path);
// 0x619e10 (&result, name, attributes; +0x1c), 0x619e90 (&result, name; +0x20: make a
// folder), 0x619f10 (&result, name; +0x24: remove the folder or file), 0x619fb0 (&result,
// from, to; +0x28: rename): the file layer's calls on the folder + name(s).
s32 *__fastcall DirSource_SetAttributes(DirSource *d, void *edx, s32 *result, const char *name, u32 attributes);
s32 *__fastcall DirSource_MakeDir(DirSource *d, void *edx, s32 *result, const char *name);
s32 *__fastcall DirSource_Remove(DirSource *d, void *edx, s32 *result, const char *name);
s32 *__fastcall DirSource_Rename(DirSource *d, void *edx, s32 *result, const char *from, const char *to);
}

// The stream sources Open makes (RefObjects; the open takes &result, path, mode).
struct DirSourceEnv {
    RefObject *(__fastcall *fileCtor)(void *self);  // 0x636e40 (0x20 bytes, CreateFileA)
    s32 *(__fastcall *fileOpen)(RefObject *o, void *edx, s32 *result, const char *path, u32 mode);  // 0x636f10
    RefObject *(__fastcall *mmioCtor)(void *self);  // 0x6371e0 (0x18 bytes, mmioOpenA)
    s32 *(__fastcall *mmioOpen)(RefObject *o, void *edx, s32 *result, const char *path, u32 mode);  // 0x6372b0
};
// The calls go through env (nullptr: the game's).
void DirSource_SetEnv(const DirSourceEnv *env);
