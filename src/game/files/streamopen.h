// The image loaders' stream openers (thiscall on a StreamBase without a source):
// StreamBase_OpenIn (0x62fa40) opens a file by name inside a container, a folder or a WAD
// archive, or by its path when there is none; StreamBase_OpenBuffered (0x62fd30) puts a buffered
// source (BufferedSource, its capacity given) over another stream's source.
//
// The originals' quirks are kept:
// - OpenIn refuses a stream that already has a source. A container is looked up resolved
//   (Path_Resolve, GetFileAttributesA) but a WAD archive is then opened by the path as given;
//   a folder is opened resolved. The entry is opened with flags 0x80000001, its result left for
//   the caller.
// - With no container the path is resolved and split at its last '/' or '\', whichever is later
//   (not DBCS-aware, unlike FileStream_OpenPath); one only at the start, or none, fails.
// - OpenBuffered attaches the other stream's source, then the buffer over it; if the buffer's
//   allocation fails it still succeeds, with just the other stream's source attached.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"
#include "bufsource.h"
#include "handles.h"

struct StreamOpenEnv {
    s32 *(__cdecl *resolve)(s32 *result, char *out, u32 size, const char *path);  // Path_Resolve 0x5f5710
    u32(__stdcall *const *getAttributes)(const char *path);  // the import table's GetFileAttributesA (0x6fd10c)
    Handle *(__fastcall *handleCtor)(Handle *);  // 0x5f8240
    void(__fastcall *handleDtor)(Handle *);      // 0x5f82a0
    s32 *(__fastcall *openWad)(Handle *, void *, s32 *result, u32 path, u32 flags);  // Handle_OpenE0 0x5f85d0
    s32 *(__fastcall *openFolder)(Handle *, void *, s32 *result, u32 path);           // Handle_Open28 0x5f8510
    s32 *(__fastcall *openEntry)(Handle *, void *, s32 *result, u32 stream, u32 name, u32 flags,
                                 u32 d);  // Handle_Vfn3 0x5f83d0
    String *(__fastcall *stringCtor)(String *, void *, const char *);  // 0x5f60b0
    String *(__fastcall *stringCopy)(String *, void *, const StringView *);  // 0x5f6040
    void(__fastcall *stringDtor)(String *);  // 0x5f61e0
    s32(__fastcall *reverseFind)(const String *, void *, const char *c, u32 start);  // 0x5f7b90
    StringView *(__fastcall *left)(const String *, void *, StringView *out, u32 count);  // 0x5f7d60
    StringView *(__fastcall *from)(const String *, void *, StringView *out, u32 start);  // 0x5f7dd0
    void *const *wadHandleVtable;     // 0x6fda18
    void *const *folderHandleVtable;  // 0x6fd9e8
    RefObject *(__fastcall *getSource)(StreamBase *);  // StreamBase_GetSource 0x4217e0
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc 0x67acc0
    BufferedSource *(__fastcall *bufferCtor)(BufferedSource *);  // 0x638ef0
    s32 *(__fastcall *bufferOpen)(BufferedSource *, void *, s32 *result, u32 capacity);  // 0x638f20
    void(__fastcall *attach)(StreamBase *, void *, RefObject *source);  // StreamBase_Attach 0x5f7240
};
// nullptr restores the game's.
void StreamOpen_SetEnv(const StreamOpenEnv *e);

extern "C" {
// (&out, name, container or null/""): 0, or -1 when it can't be opened (an entry's own result).
s32 *__fastcall StreamBase_OpenIn(StreamBase *s, void *edx, s32 *out, const char *name, const char *container);
// (&out, other, capacity): 0, or -1 when the buffer can't be opened (it is deleted).
s32 *__fastcall StreamBase_OpenBuffered(StreamBase *s, void *edx, s32 *out, StreamBase *other, u32 capacity);
}
