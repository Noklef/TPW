// The directory source: see dirsource.h.
#include "dirsource.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "file.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const DirSourceEnv kGameEnv = {
    at<RefObject *(__fastcall *)(void *)>(0x00636e40),
    at<s32 *(__fastcall *)(RefObject *, void *, s32 *, const char *, u32)>(0x00636f10),
    at<RefObject *(__fastcall *)(void *)>(0x006371e0),
    at<s32 *(__fastcall *)(RefObject *, void *, s32 *, const char *, u32)>(0x006372b0),
};
const DirSourceEnv *g_env = &kGameEnv;

void *const *const kDirSourceVtable = reinterpret_cast<void *const *>(0x007038d0);
void *const kNoSearch = reinterpret_cast<void *>(static_cast<std::uintptr_t>(0xffffffff));
constexpr u32 kMmio = 0x8000;

u32 length(const char *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}

// REP MOVSD then REP MOVSB, forward.
void copy(char *dst, const char *src, u32 n) {
    for (u32 i = 0; i < n / 4; ++i, dst += 4, src += 4) {
        u32 v;
        std::memcpy(&v, src, 4);
        std::memcpy(dst, &v, 4);
    }
    for (u32 i = 0; i < n % 4; ++i) *dst++ = *src++;
}

// The folder + name, in a String the caller destroys.
void folder_plus(String *s, const DirSource *d, const char *name) {
    String_CtorCopy(s, nullptr, reinterpret_cast<const StringView *>(&d->base.path));
    String_AppendStr(s, nullptr, name);
}

}  // namespace

void DirSource_SetEnv(const DirSourceEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

DirSource *__fastcall DirSource_Ctor(DirSource *d) {
    RefObject2_Ctor(&d->base);
    d->base.vtable = kDirSourceVtable;
    d->find = kNoSearch;
    return d;
}

void __fastcall DirSource_Dtor(DirSource *d) {
    d->base.vtable = kDirSourceVtable;
    DirSource_Close(d);
    RefObject2_Dtor(&d->base);
}

DirSource *__fastcall DirSource_DeletingDtor(DirSource *d, void *, u32 flags) {
    DirSource_Dtor(d);
    if (flags & 1) Handles_GetEnv()->free(d);
    return d;
}

s32 *__fastcall DirSource_Open(DirSource *d, void *, s32 *result, StreamBase *stream, const char *name, u32 mode,
                               u32) {
    if (stream->source) {
        *result = -1;
        return result;
    }
    String path;
    folder_plus(&path, d, name);
    RefObject *source;
    s32 opened;
    const s32 *r;
    if (mode & kMmio) {
        void *const block = Handles_GetEnv()->alloc(0x18);
        source = block ? g_env->mmioCtor(block) : nullptr;
        r = g_env->mmioOpen(source, nullptr, &opened, path.data, mode & ~kMmio);
    } else {
        void *const block = Handles_GetEnv()->alloc(0x20);
        source = block ? g_env->fileCtor(block) : nullptr;
        r = g_env->fileOpen(source, nullptr, &opened, path.data, mode);
    }
    if (*r < 0) {
        RefObject_Release(source);
        *result = -1;
    } else {
        StreamBase_Attach(stream, nullptr, source);
        *result = 0;
    }
    String_Dtor(&path);
    return result;
}

s32 *__fastcall DirSource_FindFirst(DirSource *d, void *, s32 *result, const char *pattern, DirEntry *entry) {
    vcall<void>(d, 0x34);
    if (!pattern || !*pattern) {
        *result = -1;
        return result;
    }
    // the last separator, or the char before the pattern
    const u32 n = length(pattern);
    const char *p = pattern + n - 1;
    char c = *p;
    while (c != '\\' && c != '/' && p >= pattern) {
        c = p[-1];
        --p;
    }
    copy(entry->path, pattern, n + 1);
    entry->name = entry->path + (p - pattern) + 1;
    String path;
    folder_plus(&path, d, pattern);
    FindData find;
    d->find = (*Files_GetEnv()->findFirstFile)(path.data, &find);
    if (d->find == kNoSearch) {
        *result = -1;
    } else {
        DirSource_FillEntry(entry, &find);
        *result = 0;
    }
    String_Dtor(&path);
    return result;
}

s32 *__fastcall DirSource_FindNext(DirSource *d, void *, s32 *result, DirEntry *entry) {
    FindData find;
    if (d->find != kNoSearch && (*Files_GetEnv()->findNextFile)(d->find, &find)) {
        DirSource_FillEntry(entry, &find);
        *result = 0;
        return result;
    }
    *result = -1;
    return result;
}

void __fastcall DirSource_Close(DirSource *d) {
    if (d->find != kNoSearch) {
        (*Files_GetEnv()->findClose)(d->find);
        d->find = kNoSearch;
    }
}

void __stdcall DirSource_FillEntry(DirEntry *entry, const FindData *find) {
    entry->size = find->sizeLow;
    entry->attributes = find->attributes;
    entry->writeTime[0] = find->writeTime[0];
    entry->writeTime[1] = find->writeTime[1];
    entry->accessTime[0] = find->accessTime[0];
    entry->accessTime[1] = find->accessTime[1];
    entry->creationTime[0] = find->creationTime[0];
    entry->creationTime[1] = find->creationTime[1];
    copy(entry->name, find->name, length(find->name) + 1);
}

s32 *__fastcall DirSource_Mount(DirSource *d, void *, s32 *result, const char *path) {
    s32 resolved;
    char buffer[0x104];
    Path_Resolve(&resolved, buffer, sizeof buffer, path);
    if ((*Files_GetEnv()->getFileAttributes)(buffer) == 0xffffffff) {
        *result = -1;
        return result;
    }
    String *const folder = &d->base.path;
    String_AssignStr(folder, nullptr, buffer);
    const char last = folder->data[folder->length - 1];
    if (last != '\\' && last != '/') {
        const char separator = '\\';
        String_AppendChar(folder, nullptr, &separator);
    }
    *result = 0;
    return result;
}

s32 *__fastcall DirSource_SetAttributes(DirSource *d, void *, s32 *result, const char *name, u32 attributes) {
    String path;
    folder_plus(&path, d, name);
    File_SetAttributes(result, path.data, attributes);
    String_Dtor(&path);
    return result;
}

s32 *__fastcall DirSource_MakeDir(DirSource *d, void *, s32 *result, const char *name) {
    String path;
    folder_plus(&path, d, name);
    Dir_Create(result, path.data);
    String_Dtor(&path);
    return result;
}

s32 *__fastcall DirSource_Remove(DirSource *d, void *, s32 *result, const char *name) {
    String path;
    folder_plus(&path, d, name);
    if (File_IsDirectory(path.data))
        Dir_Remove(result, path.data);
    else
        File_Delete(result, path.data);
    String_Dtor(&path);
    return result;
}

s32 *__fastcall DirSource_Rename(DirSource *d, void *, s32 *result, const char *from, const char *to) {
    String a;
    folder_plus(&a, d, from);
    String b;
    folder_plus(&b, d, to);
    File_Move(result, a.data, b.data);
    String_Dtor(&b);
    String_Dtor(&a);
    return result;
}

}  // extern "C"
