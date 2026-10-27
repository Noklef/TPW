// The game's virtual file system: files in the mounted WAD archives or on disk. Reimplementation of 0x41cff0, the path taken relative to
// a base (an archive's folder or the archive itself), of 0x5c4d50, a file opened through the mounts, and of 0x46f120, whether one opens.
#pragma once

#include "../../../re/types.h"

struct String;
struct StreamBase;

extern "C" {

// 0x41cff0 (cdecl; path, base, out; name unconfirmed): copies a of path and b of base (each String_SkipDotPrefix'd in place first; the
// copy through the vtable's +0x18, AssignWide, as the game's does), with a's leading "\"s (the global at 0x78bee0) dropped one at a time.
// When b's last chars, as many as the String at 0x78bf40 has, equal ".wad*" (the String at 0x78c810, Names_Equal == 1), b loses as many
// chars as that one has and gets a trailing "\" (String_EnsureTrailingBackslash). Then when a is not shorter than b by more than that
// String's length, and a's first b->length chars equal b (case blind), out = the rest of a with its leading "\"s dropped, and 1 is
// returned; otherwise out = the empty String at 0x78be50 (vtable +0x18) and 0. Each assignment goes through the vtable's +0x1c.
u32 __cdecl Path_StripBase(String *path, String *base, String *out);

// 0x5c4d50 (cdecl; name): a stream for the file `name`. A new StreamBase (8 bytes from the game's heap, StreamBase_Ctor; 0 when that
// failed, passed on as it is). The mounts object [0x786b9c] (+0 a list, +4 the current source; StreamBase_GetSource): with a source,
// whose +0x38 holds an archive, the name is taken relative to the source's root (its WString at +0x18, through the vtable's +0x18, as
// AssignWide does: Path_StripBase) and on 1 the archive opens it (slot 0xc: a result struct returned through its first argument, the
// stream, the relative path's text, 0x80000001, 0), done when the result's first dword is not negative. Otherwise (no archive, not
// under the root, or that failed) the source itself is asked (slot 8: the stream, a String of the name, 0x80000001), done on 1. Then
// (or without a source) the list's objects are asked in turn (ListRef_AnyHandles on [0x786b9c]): the stream on 1, else 0 - the stream
// is not freed then, nor when an earlier step failed.
StreamBase *__cdecl VFS_Open(const char *name);

// 0x5c5110 (cdecl; name, out; a bool in AL; name unconfirmed): a file's name within the current source's archive. VFS_Open(name's text) (the
// game pushes 0x74cfb8 too; the stream got is never freed); none: 0. Then, with no checks, the source (StreamBase_GetSource of the mounts
// [0x786b9c]) and its archive (+0x38): copies (String_CtorCopy) of the source's root at +0x18 (a WString read as a narrow string, sic) and of
// name; when Path_StripBase(name's copy, the root's, out) is 1 the name's copy is assigned out (slot 0x18) and the archive asked (slot 0x10:
// a result struct through its first argument, out, the name's copy; its answer unread): 1. Else 0. The copies destroyed either way.
bool __cdecl VFS_ArchiveName(const String *name, String *out);

// 0x46f120 (cdecl; name): VFS_Open(name) (the game pushes a second argument, 0x74cfb8,
// which VFS_Open does not read); a stream got is finished and deleted (Object_FinishAndDelete) and 1 returned, else 0.
u32 __cdecl VFS_FileExists(const char *name);

// 0x5c5010 (cdecl; from, to; name unconfirmed; the sign styles' font files): VFS_Open(from) (none: 0); its size (slot 5) read (slot 2) into
// a Mem_GlobalAlloc block (none, as for size 0: 0, the stream left open), the stream closed (slot 1) and deleted (slot 0). Then a new
// FileStream (Mem_Alloc(8), StreamBase_Ctor, vtable 0x6fd930: none faults) opened (slot 9: &result, to, 0xc0000010 (write, create
// always)): a negative result through the pointer it returns answers 0 (the stream left); else the block written (slot 3), the stream
// closed (slot 1) and deleted (slot 0); 1. The block is never freed.
u32 __cdecl VFS_ExtractFile(const char *from, const char *to);
}
