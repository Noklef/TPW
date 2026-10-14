// The game's file and system layer: reimplementation of 0x5f5660-0x5f5fb0.
//
// Paths are resolved against a base directory (0xfa6fd0), by default the folder of the exe
// taken from the command line: "X:\..." and "\\..." (either slash) are used as they are,
// "\..." gets the base directory's drive, anything else is appended to the base directory.
// The functions that report success return a result through a hidden pointer, 0 or -1 (or
// the stream's error), as MSVC does for a class: `Result r = File_Delete(path)`.
//
// Streams are objects with a vtable: +0 deleting destructor (flags), +4 close, +8 read
// (buffer, count), +0xc write (buffer, count), +0x10 seek (offset, origin), +0x18 position,
// +0x24 open (&result, path, mode); FileStream (vtable 0x6fd930) is the file one, its base
// (vtable 0x703100) holds a reference at +4.
#pragma once

#include <cstdint>

#include "../../../re/types.h"

struct Stream {
    void *const *vtable;
};

// The QueryPerformanceCounter clock (0xfa71e0 is the game's).
struct Timer {
    u32 enabled;         // the counter works and ticks between 10 and 32,000,000 times a ms
    u32 ticksPerMs;      // the frequency / 1000, both halved until it fits in 32 bits
    u32 baseMs;          // timeGetTime at Timer_Init
    u32 pad;
    std::int64_t start;  // the counter at Timer_Init
};
static_assert(sizeof(Timer) == 0x18, "Timer is 0x18 bytes");

extern "C" {
// 0x5f5660: GlobalAlloc(GMEM_FIXED, size); 0: none (nullptr). 0x5f5680: GlobalFree.
void *__cdecl Mem_GlobalAlloc(u32 size);
void *__cdecl Mem_GlobalFree(void *block);
// 0x5f5690: the base directory (0xfa6fd0), with a backslash added unless it ends in one
// (either slash); left as it is if that would not fit in MAX_PATH. The original reads the
// char before an empty path.
void __cdecl Dir_SetBase(const char *path);
// 0x5f56f0: the base directory, first set from the command line (Dir_InitBase) while empty.
char *__cdecl Dir_GetBase();
// 0x5f5950: the exe's folder from the command line (quoted or not); nothing without a slash.
// The command line is copied to 0xfa6ec8 (at most 259 chars).
void __cdecl Dir_InitBase();
// 0x5f5710 (&result, out, size, path): path resolved into out; size is not used. nullptr:
// the base directory.
s32 *__cdecl Path_Resolve(s32 *result, char *out, u32 size, const char *path);
// 0x5f5830 (&result, path, buffer, max, &size): the whole file into buffer (nothing if it is
// longer than max); size (if not nullptr) set to its length. A failed read is not noticed.
s32 *__cdecl File_Load(s32 *result, const char *path, void *buffer, s32 max, s32 *size);
// 0x5f5a20: FileStream_Open.
s32 *__cdecl File_Open(s32 *result, Stream **out, const char *path, u32 mode);
// 0x5f5ad0 (&result, &stream, path, mode): a new FileStream, opened (vtable +0x24); not
// freed when that fails (nor checked when the allocation fails).
s32 *__cdecl FileStream_Open(s32 *result, Stream **out, const char *path, u32 mode);
// 0x5f5a50 / 0x5f5a90: CreateDirectoryA / RemoveDirectoryA, the path resolved into 0xfa70d8.
s32 *__cdecl Dir_Create(s32 *result, const char *path);
s32 *__cdecl Dir_Remove(s32 *result, const char *path);
// 0x5f5b80: closes (vtable +4) and deletes the stream.
s32 *__cdecl Stream_Close(s32 *result, Stream *stream);
// 0x5f5bb0: the stream's position (vtable +0x18).
u32 __cdecl Stream_Tell(Stream *stream);
// 0x5f5bc0: CompareFileTime; nullptr is less than any time.
s32 __cdecl FileTime_Compare(const void *a, const void *b);
// 0x5f5bf0: vtable +0x10 (offset, origin).
u32 __cdecl Stream_Seek(Stream *stream, s32 offset, u32 origin);
// 0x5f5c10: SetFileAttributesA on the resolved path.
s32 *__cdecl File_SetAttributes(s32 *result, const char *path, u32 attributes);
// 0x5f5c70: the size (low 32 bits) from FindFirstFileA, or -1.
u32 __cdecl File_GetSize(const char *path);
// 0x5f5cd0: whether GetFileAttributesA finds it. 0x5f5d10: its directory bit (0x10).
u32 __cdecl File_Exists(const char *path);
u32 __cdecl File_IsDirectory(const char *path);
// 0x5f5d60: DeleteFileA on the resolved path. 0x5f5dc0 / 0x5f5df0: MoveFileA / CopyFileA
// (overwriting), the paths not resolved.
s32 *__cdecl File_Delete(s32 *result, const char *path);
s32 *__cdecl File_Move(s32 *result, const char *from, const char *to);
s32 *__cdecl File_Copy(s32 *result, const char *from, const char *to);
// 0x5f5e20 / 0x5f5e50 (&result, stream, buffer, count, &done): vtable +8 / +0xc; done (if
// not nullptr) set to what it did, result 1 when that is count, else 0.
s32 *__cdecl Stream_Read(s32 *result, Stream *stream, void *buffer, s32 count, s32 *done);
s32 *__cdecl Stream_Write(s32 *result, Stream *stream, const void *buffer, s32 count, s32 *done);
// 0x5f5e80 (__thiscall): starts the clock. 0x5f5f10 (__thiscall): milliseconds since then
// plus the timeGetTime it started at, or timeGetTime when the counter is not used.
void __fastcall Timer_Init(Timer *timer);
u32 __fastcall Timer_GetMs(Timer *timer);
// 0x5f5f90 / 0x5f5fa0: the same on the game's clock (0xfa71e0). 0x5f5fb0: Sleep.
void __cdecl Time_Init();
u32 __cdecl Time_GetMs();
void __cdecl Sys_Sleep(u32 ms);
}

// What the layer calls in the game, so that it can run outside it. The Win32 members point at
// the function pointers (the game's: its import table).
struct FileEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    char *(__stdcall *const *getCommandLine)();
    s32(__stdcall *const *createDirectory)(const char *path, void *security);
    s32(__stdcall *const *removeDirectory)(const char *path);
    s32(__stdcall *const *setFileAttributes)(const char *path, u32 attributes);
    void *(__stdcall *const *findFirstFile)(const char *path, void *findData);
    s32(__stdcall *const *findClose)(void *find);
    u32(__stdcall *const *getFileAttributes)(const char *path);
    s32(__stdcall *const *deleteFile)(const char *path);
    s32(__stdcall *const *moveFile)(const char *from, const char *to);
    s32(__stdcall *const *copyFile)(const char *from, const char *to, s32 failIfExists);
    s32(__stdcall *const *compareFileTime)(const void *a, const void *b);
    s32(__stdcall *const *queryPerformanceFrequency)(std::int64_t *frequency);
    s32(__stdcall *const *queryPerformanceCounter)(std::int64_t *count);
    u32(__stdcall *const *timeGetTime)();
    void(__stdcall *const *sleep)(u32 ms);
    void *(__stdcall *const *globalAlloc)(u32 flags, u32 size);
    void *(__stdcall *const *globalFree)(void *block);
    s32(__stdcall *const *findNextFile)(void *find, void *findData);
};
// The layer's calls go to env (nullptr: the game's).
void Files_SetEnv(const FileEnv *env);
// The env in use (for the file library's classes).
const FileEnv *Files_GetEnv();
