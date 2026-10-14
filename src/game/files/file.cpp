// The game's file and system layer: reimplementation of 0x5f5660-0x5f5fb0 (the 0x617xxx
// manager's wrappers before it are not here).
#include "file.h"

#include <cstring>

#include "../core/vcall.h"

namespace {

constexpr u32 kMaxPath = 0x104;
char *commandLine() { return reinterpret_cast<char *>(0x00fa6ec8); }
char *base() { return reinterpret_cast<char *>(0x00fa6fd0); }
char *pathBuffer() { return reinterpret_cast<char *>(0x00fa70d8); }
Timer *clock() { return reinterpret_cast<Timer *>(0x00fa71e0); }

void *const *const kStreamVtable = reinterpret_cast<void *const *>(0x00703100);
void *const *const kFileStreamVtable = reinterpret_cast<void *const *>(0x006fd930);

template <typename T>
T iat(u32 slot) {
    return reinterpret_cast<T>(static_cast<uintptr_t>(slot));
}
const FileEnv kGameEnv = {
    reinterpret_cast<void *(__cdecl *)(u32)>(0x0067acc0),
    iat<char *(__stdcall *const *)()>(0x006fd1e4),
    iat<s32(__stdcall *const *)(const char *, void *)>(0x006fd120),
    iat<s32(__stdcall *const *)(const char *)>(0x006fd114),
    iat<s32(__stdcall *const *)(const char *, u32)>(0x006fd108),
    iat<void *(__stdcall *const *)(const char *, void *)>(0x006fd100),
    iat<s32(__stdcall *const *)(void *)>(0x006fd104),
    iat<u32(__stdcall *const *)(const char *)>(0x006fd10c),
    iat<s32(__stdcall *const *)(const char *)>(0x006fd0c4),
    iat<s32(__stdcall *const *)(const char *, const char *)>(0x006fd11c),
    iat<s32(__stdcall *const *)(const char *, const char *, s32)>(0x006fd158),
    iat<s32(__stdcall *const *)(const void *, const void *)>(0x006fd110),
    iat<s32(__stdcall *const *)(std::int64_t *)>(0x006fd0ac),
    iat<s32(__stdcall *const *)(std::int64_t *)>(0x006fd0cc),
    iat<u32(__stdcall *const *)()>(0x006fd3ec),
    iat<void(__stdcall *const *)(u32)>(0x006fd1d4),
    iat<void *(__stdcall *const *)(u32, u32)>(0x006fd1cc),
    iat<void *(__stdcall *const *)(void *)>(0x006fd26c),
};
const FileEnv *g_env = &kGameEnv;

u32 str_len(const char *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}
// The inlined strcpy and strcat: the length measured first, then dwords (each read before it
// is written) and the last bytes.
void copy_dwords(char *to, const char *from, u32 count) {
    u32 i = 0;
    for (; i + 4 <= count; i += 4) {
        char d[4];
        std::memcpy(d, from + i, 4);
        std::memcpy(to + i, d, 4);
    }
    for (; i < count; ++i) to[i] = from[i];
}
void str_cpy(char *to, const char *from) { copy_dwords(to, from, str_len(from) + 1); }
void str_cat(char *to, const char *from) {
    const u32 n = str_len(from) + 1;
    copy_dwords(to + str_len(to), from, n);
}
// sprintf(to, "%s", from), with a backslash after it: the chars one at a time.
void print_path(char *to, const char *from, bool slash) {
    const u32 n = str_len(from);
    for (u32 i = 0; i < n; ++i) to[i] = from[i];
    u32 end = n;
    if (slash) to[end++] = '\\';
    to[end] = 0;
}
char *str_rchr(char *s, char c) {
    char *last = nullptr;
    for (; *s; ++s)
        if (*s == c) last = s;
    return last;
}
bool is_slash(char c) { return c == '\\' || c == '/'; }

s32 *pass_fail(s32 *result, s32 ok) {
    *result = ok ? 0 : -1;
    return result;
}

}  // namespace

void Files_SetEnv(const FileEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void *__cdecl Mem_GlobalAlloc(u32 size) { return size ? (*g_env->globalAlloc)(0, size) : nullptr; }

void *__cdecl Mem_GlobalFree(void *block) { return (*g_env->globalFree)(block); }

void __cdecl Dir_SetBase(const char *path) {
    const u32 length = str_len(path);
    if (length >= kMaxPath) return;
    if (is_slash(path[static_cast<s32>(length) - 1]))
        print_path(base(), path, false);
    else if (length + 1 < kMaxPath)
        print_path(base(), path, true);
}

char *__cdecl Dir_GetBase() {
    if (!base()[0]) Dir_InitBase();
    return base();
}

void __cdecl Dir_InitBase() {
    char *const line = commandLine();
    const char *const from = (*g_env->getCommandLine)();
    u32 i = 0;
    for (; i < kMaxPath - 1 && from[i]; ++i) line[i] = from[i];
    for (; i < kMaxPath - 1; ++i) line[i] = 0;
    line[kMaxPath - 1] = 0;
    char end = ' ';
    if (line[0] == '"') {
        end = '"';
        const s32 length = static_cast<s32>(str_len(line));
        for (s32 j = 0; j < length; ++j) line[j] = line[j + 1];
    }
    char *p = line;
    while (*p && *p != end) ++p;
    *p = 0;
    char *const slash = str_rchr(line, '/');
    char *const backslash = str_rchr(line, '\\');
    char *const last = reinterpret_cast<uintptr_t>(backslash) > reinterpret_cast<uintptr_t>(slash) ? backslash : slash;
    if (last) {
        last[1] = 0;
        Dir_SetBase(line);
    }
}

s32 *__cdecl Path_Resolve(s32 *result, char *out, u32, const char *path) {
    if (!path) {
        str_cpy(out, Dir_GetBase());
        *result = 0;
        return result;
    }
    if ((path[1] == ':' && is_slash(path[2])) || (is_slash(path[0]) && is_slash(path[1]))) {
        str_cpy(out, path);
        *result = 0;
        return result;
    }
    if (is_slash(path[0])) {
        // _splitpath's drive: the first two chars when the second is a colon.
        const char *const from = Dir_GetBase();
        char drive[4] = {};
        if (from[0] && from[1] == ':') {
            drive[0] = from[0];
            drive[1] = ':';
        }
        str_cpy(out, drive);
    } else {
        str_cpy(out, Dir_GetBase());
    }
    str_cat(out, path);
    *result = 0;
    return result;
}

s32 *__cdecl File_Load(s32 *result, const char *path, void *buffer, s32 max, s32 *size) {
    s32 r;
    char resolved[kMaxPath];
    Path_Resolve(&r, resolved, kMaxPath, path);
    const s32 length = static_cast<s32>(File_GetSize(resolved));
    if (length == -1 || length > max) {
        *result = -1;
        return result;
    }
    Stream *stream;
    FileStream_Open(&r, &stream, resolved, 0x80000001);
    if (r < 0) {
        *result = -1;
        return result;
    }
    Stream_Read(&r, stream, buffer, length, nullptr);
    Stream_Close(&r, stream);
    if (size) *size = length;
    *result = 0;
    return result;
}

s32 *__cdecl File_Open(s32 *result, Stream **out, const char *path, u32 mode) {
    return FileStream_Open(result, out, path, mode);
}

s32 *__cdecl FileStream_Open(s32 *result, Stream **out, const char *path, u32 mode) {
    Stream *const stream = static_cast<Stream *>(g_env->alloc(8));
    if (stream) {
        // The base's constructor (0x5f7180) sets its vtable and reference, then the FileStream's.
        reinterpret_cast<u32 *>(stream)[1] = 0;
        stream->vtable = kStreamVtable;
        stream->vtable = kFileStreamVtable;
    }
    s32 r;
    vcall<s32 *>(stream, 0x24, &r, path, mode);
    if (r < 0) {
        *result = r;
        return result;
    }
    *out = stream;
    *result = 0;
    return result;
}

s32 *__cdecl Dir_Create(s32 *result, const char *path) {
    s32 r;
    Path_Resolve(&r, pathBuffer(), kMaxPath, path);
    return pass_fail(result, (*g_env->createDirectory)(pathBuffer(), nullptr));
}

s32 *__cdecl Dir_Remove(s32 *result, const char *path) {
    s32 r;
    Path_Resolve(&r, pathBuffer(), kMaxPath, path);
    return pass_fail(result, (*g_env->removeDirectory)(pathBuffer()));
}

s32 *__cdecl Stream_Close(s32 *result, Stream *stream) {
    vcall<void>(stream, 4);
    if (stream) vcall<void *>(stream, 0, 1u);
    *result = 0;
    return result;
}

u32 __cdecl Stream_Tell(Stream *stream) { return vcall<u32>(stream, 0x18); }

s32 __cdecl FileTime_Compare(const void *a, const void *b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    return (*g_env->compareFileTime)(a, b);
}

u32 __cdecl Stream_Seek(Stream *stream, s32 offset, u32 origin) { return vcall<u32>(stream, 0x10, offset, origin); }

s32 *__cdecl File_SetAttributes(s32 *result, const char *path, u32 attributes) {
    s32 r;
    char resolved[kMaxPath];
    Path_Resolve(&r, resolved, kMaxPath, path);
    return pass_fail(result, (*g_env->setFileAttributes)(resolved, attributes));
}

u32 __cdecl File_GetSize(const char *path) {
    s32 r;
    char resolved[kMaxPath];
    Path_Resolve(&r, resolved, kMaxPath, path);
    u32 findData[0x50];  // WIN32_FIND_DATAA; nFileSizeLow at +0x20
    void *const find = (*g_env->findFirstFile)(resolved, findData);
    if (find == reinterpret_cast<void *>(-1)) return 0xffffffff;
    (*g_env->findClose)(find);
    return findData[8];
}

u32 __cdecl File_Exists(const char *path) {
    s32 r;
    char resolved[kMaxPath];
    Path_Resolve(&r, resolved, kMaxPath, path);
    return (*g_env->getFileAttributes)(resolved) != 0xffffffff;
}

u32 __cdecl File_IsDirectory(const char *path) {
    s32 r;
    char resolved[kMaxPath];
    Path_Resolve(&r, resolved, kMaxPath, path);
    const u32 attributes = (*g_env->getFileAttributes)(resolved);
    return attributes == 0xffffffff ? 0 : attributes & 0x10;
}

s32 *__cdecl File_Delete(s32 *result, const char *path) {
    s32 r;
    char resolved[kMaxPath];
    Path_Resolve(&r, resolved, kMaxPath, path);
    return pass_fail(result, (*g_env->deleteFile)(resolved));
}

s32 *__cdecl File_Move(s32 *result, const char *from, const char *to) {
    return pass_fail(result, (*g_env->moveFile)(from, to));
}

s32 *__cdecl File_Copy(s32 *result, const char *from, const char *to) {
    return pass_fail(result, (*g_env->copyFile)(from, to, 0));
}

s32 *__cdecl Stream_Read(s32 *result, Stream *stream, void *buffer, s32 count, s32 *done) {
    const s32 n = vcall<s32>(stream, 8, buffer, count);
    if (done) *done = n;
    *result = n == count;
    return result;
}

s32 *__cdecl Stream_Write(s32 *result, Stream *stream, const void *buffer, s32 count, s32 *done) {
    const s32 n = vcall<s32>(stream, 0xc, buffer, count);
    if (done) *done = n;
    *result = n == count;
    return result;
}

void __fastcall Timer_Init(Timer *timer) {
    timer->enabled = 0;
    timer->baseMs = (*g_env->timeGetTime)();
    std::int64_t frequency;
    if (!(*g_env->queryPerformanceFrequency)(&frequency)) return;
    if (!(*g_env->queryPerformanceCounter)(&timer->start)) return;
    // The frequency and 1000 halved together until the frequency fits in 32 bits.
    u32 high = static_cast<u32>(static_cast<std::uint64_t>(frequency) >> 32) & 0x7fffffff;
    u32 low = static_cast<u32>(frequency);
    u32 divisor = 1000;
    while (high > 0) {
        low = low >> 1 | high << 31;
        high >>= 1;
        divisor >>= 1;
    }
    if (divisor < 125) return;
    timer->ticksPerMs = low / divisor;
    if (timer->ticksPerMs < 10 || timer->ticksPerMs > 32000000) return;
    timer->enabled = 1;
}

u32 __fastcall Timer_GetMs(Timer *timer) {
    std::int64_t now;
    if (!timer->enabled || !(*g_env->queryPerformanceCounter)(&now)) return (*g_env->timeGetTime)();
    // The original divides with one IDIV, which faults when the quotient passes 31 bits
    // (after 24 days).
    const s32 ms = static_cast<s32>((now - timer->start) / static_cast<s32>(timer->ticksPerMs));
    return static_cast<u32>(ms) + timer->baseMs;
}

void __cdecl Time_Init() { Timer_Init(clock()); }

u32 __cdecl Time_GetMs() { return Timer_GetMs(clock()); }

void __cdecl Sys_Sleep(u32 ms) { (*g_env->sleep)(ms); }

}  // extern "C"
