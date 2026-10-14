// The file and mmio stream sources (vtables 0x703e30 / 0x703e60): reimplementation of
// 0x636e40-0x637450.
//
// The RefObject sources DirSource opens: one over a CreateFileA handle (read/write, seek, size,
// read-only mapped views, flush) and one over a buffered mmioOpenA handle (read-only opens).
// Each opens the resolved path (Path_Resolve into a global buffer), keeps the position the
// last read, write or seek left and carries a semaphore for its users (vtable +0x2c).
//
// The originals' quirks are kept: the file source's destructor leaves the handle open (the
// release closes it first, vtable +0x24); dropping the mapping closes it before unmapping the
// whole-file view through vtable +0x1c, which then does nothing, so the view stays mapped; a
// view at an offset up to the allocation granularity maps the whole file; an mmio read or
// write error (-1) moves the position back one.
#pragma once

#include "../../../re/types.h"
#include "../core/semaphore.h"
#include "handles.h"

struct FileSource {
    RefObject base;
    void *handle;  // INVALID_HANDLE_VALUE when closed
    u32 position;
    void *mapping;
    void *view;  // the whole-file view
    Semaphore lock;
};
static_assert(sizeof(FileSource) == 0x20, "FileSource is 0x20 bytes");

struct MmioSource {
    RefObject base;
    void *handle;  // HMMIO, 0 when closed
    u32 position;
    Semaphore lock;
};
static_assert(sizeof(MmioSource) == 0x18, "MmioSource is 0x18 bytes");

extern "C" {
// __thiscall. 0x636e40: closed. 0x636ec0 / 0x636ea0 (flags; vtable +0): destructors.
FileSource *__fastcall FileSource_Ctor(FileSource *f);
void __fastcall FileSource_Dtor(FileSource *f);
FileSource *__fastcall FileSource_DeletingDtor(FileSource *f, void *edx, u32 flags);
// 0x636f10 (&result, path, mode): closes (+0x24), then CreateFileA on the resolved path with
// access mode & 0xc0000000, sharing mode & 3 and, from mode & 0x70, OPEN_EXISTING (no 0x10),
// OPEN_ALWAYS (0x30), CREATE_NEW (0x50) or CREATE_ALWAYS; 0, or -1 when it failed.
s32 *__fastcall FileSource_Open(FileSource *f, void *edx, s32 *result, const char *path, u32 mode);
// 0x636fb0: closes the mapping, view dropped (see the header). 0x636fe0 (+0x24): that, and
// the handle.
void __fastcall FileSource_ReleaseMapping(FileSource *f);
void __fastcall FileSource_Close(FileSource *f);
// 0x637000 (+4) / 0x637040 (+8) (buffer, count): ReadFile / WriteFile; the bytes moved.
u32 __fastcall FileSource_Read(FileSource *f, void *edx, void *buffer, u32 count);
u32 __fastcall FileSource_Write(FileSource *f, void *edx, const void *buffer, u32 count);
// 0x637080 (offset, origin; +0xc): SetFilePointer; the new position. 0x6370a0 (+0x10):
// GetFileSize. 0x6370b0 (+0x14, the mmio source's too): the position.
u32 __fastcall FileSource_Seek(FileSource *f, void *edx, s32 offset, u32 origin);
u32 __fastcall FileSource_Size(FileSource *f);
u32 __fastcall FileSource_Tell(FileSource *f);
// 0x6370c0 (offset, size; +0x18): a read-only view of size bytes at offset (0, 0: the whole
// file, kept); nullptr when closed or it failed. 0x637180 (view; +0x1c): unmaps it.
void *__fastcall FileSource_Map(FileSource *f, void *edx, u32 offset, u32 size);
void __fastcall FileSource_Unmap(FileSource *f, void *edx, void *view);
// 0x6371b0 (&result; +0x20): FlushFileBuffers; 0 or -1. 0x6371a0 (+0x2c): the semaphore.
s32 *__fastcall FileSource_Flush(FileSource *f, void *edx, s32 *result);
Semaphore *__fastcall FileSource_Lock(FileSource *f);

// 0x6371e0: closed. 0x637250 / 0x637230 (flags; +0): destructors (these close).
MmioSource *__fastcall MmioSource_Ctor(MmioSource *m);
void __fastcall MmioSource_Dtor(MmioSource *m);
MmioSource *__fastcall MmioSource_DeletingDtor(MmioSource *m, void *edx, u32 flags);
// 0x6372b0 (&result, path, mode): closes (+0x24); -1 for a write mode (0x40000000), else
// mmioOpenA (read) on the resolved path with a 16 KB buffer; 0, or -1 when it failed.
s32 *__fastcall MmioSource_Open(MmioSource *m, void *edx, s32 *result, const char *path, u32 mode);
// 0x637340 (+0x24): mmioClose.
void __fastcall MmioSource_Close(MmioSource *m);
// 0x637360 (+4) / 0x637390 (+8) (buffer, count): mmioRead / mmioWrite; their result.
s32 __fastcall MmioSource_Read(MmioSource *m, void *edx, void *buffer, s32 count);
s32 __fastcall MmioSource_Write(MmioSource *m, void *edx, const void *buffer, s32 count);
// 0x6373c0 (offset, origin 0/1/else end; +0xc): mmioSeek; the new position. 0x6373f0
// (+0x10): the end, the position kept.
s32 __fastcall MmioSource_Seek(MmioSource *m, void *edx, s32 offset, u32 origin);
s32 __fastcall MmioSource_Size(MmioSource *m);
// 0x637450 (offset, size; +0x18): no views, nullptr. 0x637420 (&result; +0x20): mmioFlush.
void *__fastcall MmioSource_Map(MmioSource *m, void *edx, u32 offset, u32 size);
s32 *__fastcall MmioSource_Flush(MmioSource *m, void *edx, s32 *result);
}

// The import table slots the sources call through, and the path buffers they open from.
struct SourcesEnv {
    void *(__stdcall *const *createFile)(const char *path, u32 access, u32 share, void *security, u32 disposition,
                                        u32 flags, void *temp);  // 0x6fd0d8
    s32(__stdcall *const *readFile)(void *h, void *buffer, u32 count, u32 *done, void *overlapped);         // 0x6fd0dc
    s32(__stdcall *const *writeFile)(void *h, const void *buffer, u32 count, u32 *done, void *overlapped);  // 0x6fd0e0
    u32(__stdcall *const *setFilePointer)(void *h, s32 offset, s32 *high, u32 origin);                     // 0x6fd0e4
    u32(__stdcall *const *getFileSize)(void *h, u32 *high);                                                // 0x6fd0e8
    void(__stdcall *const *getSystemInfo)(void *info);                                                     // 0x6fd0ec
    void *(__stdcall *const *mapViewOfFileEx)(void *mapping, u32 access, u32 high, u32 low, u32 size,
                                             void *at);                                                    // 0x6fd0f0
    void *(__stdcall *const *createFileMapping)(void *h, void *security, u32 protect, u32 high, u32 low,
                                               const char *name);                                         // 0x6fd0f4
    s32(__stdcall *const *unmapViewOfFile)(const void *view);                                              // 0x6fd0f8
    s32(__stdcall *const *flushFileBuffers)(void *h);                                                      // 0x6fd0fc
    s32(__stdcall *const *closeHandle)(void *h);                                                           // 0x6fd13c
    void *(__stdcall *const *mmioOpen)(char *path, void *info, u32 flags);                                 // 0x6fd3e4
    u32(__stdcall *const *mmioClose)(void *h, u32 flags);                                                  // 0x6fd3f0
    s32(__stdcall *const *mmioRead)(void *h, void *buffer, s32 count);                                     // 0x6fd3e0
    s32(__stdcall *const *mmioWrite)(void *h, const void *buffer, s32 count);                              // 0x6fd3dc
    s32(__stdcall *const *mmioSeek)(void *h, s32 offset, s32 origin);                                      // 0x6fd3d8
    u32(__stdcall *const *mmioSetBuffer)(void *h, char *buffer, s32 size, u32 flags);                      // 0x6fd3e8
    u32(__stdcall *const *mmioFlush)(void *h, u32 flags);                                                  // 0x6fd3d4
    char *filePath;  // 0xfaa2c8 (0x104 bytes)
    char *mmioPath;  // 0xfaa3d0
};
// The calls go through env (nullptr: the game's).
void Sources_SetEnv(const SourcesEnv *env);
