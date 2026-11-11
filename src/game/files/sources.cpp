// The file and mmio stream sources: see sources.h.
#include "sources.h"

#include <cstdint>

#include "../core/vcall.h"
#include "file.h"
#include "../core/imports.h"
#include "../core/addrcalls.h"

namespace {

template <typename T>
T at(u32 address) {
    return GameAddress<T>(address);
}
#define SLOT(member, address) at<decltype(SourcesEnv::member)>(address)
const SourcesEnv kGameEnv = {
    SLOT(createFile, IMPORT(Kernel32_CreateFileA)),
    SLOT(readFile, IMPORT(Kernel32_ReadFile)),
    SLOT(writeFile, IMPORT(Kernel32_WriteFile)),
    SLOT(setFilePointer, IMPORT(Kernel32_SetFilePointer)),
    SLOT(getFileSize, IMPORT(Kernel32_GetFileSize)),
    SLOT(getSystemInfo, IMPORT(Kernel32_GetSystemInfo)),
    SLOT(mapViewOfFileEx, IMPORT(Kernel32_MapViewOfFileEx)),
    SLOT(createFileMapping, IMPORT(Kernel32_CreateFileMappingA)),
    SLOT(unmapViewOfFile, IMPORT(Kernel32_UnmapViewOfFile)),
    SLOT(flushFileBuffers, IMPORT(Kernel32_FlushFileBuffers)),
    SLOT(closeHandle, IMPORT(Kernel32_CloseHandle)),
    SLOT(mmioOpen, IMPORT(Winmm_mmioOpenA)),
    SLOT(mmioClose, IMPORT(Winmm_mmioClose)),
    SLOT(mmioRead, IMPORT(Winmm_mmioRead)),
    SLOT(mmioWrite, IMPORT(Winmm_mmioWrite)),
    SLOT(mmioSeek, IMPORT(Winmm_mmioSeek)),
    SLOT(mmioSetBuffer, IMPORT(Winmm_mmioSetBuffer)),
    SLOT(mmioFlush, IMPORT(Winmm_mmioFlush)),
    at<char *>((u32)&gFileSourcePath),
    at<char *>((u32)&gMmioSourcePath),
};
#undef SLOT
const SourcesEnv *g_env = &kGameEnv;

void *const *const kFileSourceVtable = FileSource_vtable;
void *const *const kMmioSourceVtable = MmioSource_vtable;
void *const kInvalid = reinterpret_cast<void *>(static_cast<std::uintptr_t>(0xffffffff));
constexpr u32 kPathSize = 0x104;
constexpr u32 kOpenExisting = 3, kOpenAlways = 4, kCreateNew = 1, kCreateAlways = 2;
constexpr u32 kRandomAccess = 0x10000000;  // FILE_FLAG_RANDOM_ACCESS
constexpr u32 kPageReadOnly = 2, kMapRead = 4;
constexpr u32 kGenericWrite = 0x40000000;
constexpr u32 kSystemInfoWords = 9, kGranularity = 7;  // SYSTEM_INFO.dwAllocationGranularity

}  // namespace

void Sources_SetEnv(const SourcesEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

FileSource *__fastcall FileSource_Ctor(FileSource *f) {
    RefObject_Ctor(&f->base);
    f->handle = kInvalid;
    f->position = 0;
    f->mapping = nullptr;
    f->view = nullptr;
    Semaphore_Ctor(&f->lock);
    f->base.vtable = kFileSourceVtable;
    return f;
}

void __fastcall FileSource_Dtor(FileSource *f) {
    f->base.vtable = kFileSourceVtable;
    Semaphore_Dtor(&f->lock);
    RefObject_Dtor(&f->base);
}

FileSource *__fastcall FileSource_DeletingDtor(FileSource *f, void *, u32 flags) {
    FileSource_Dtor(f);
    if (flags & 1) Handles_GetEnv()->free(f);
    return f;
}

s32 *__fastcall FileSource_Open(FileSource *f, void *, s32 *result, const char *path, u32 mode) {
    vcall<void>(f, 0x24);
    s32 resolved;
    Path_Resolve(&resolved, g_env->filePath, kPathSize, path);
    u32 disposition = kOpenExisting;
    if (mode & 0x10) disposition = (mode & 0x20) ? kOpenAlways : (mode & 0x40) ? kCreateNew : kCreateAlways;
    f->handle = (*g_env->createFile)(g_env->filePath, mode & 0xc0000000, mode & 3, nullptr, disposition,
                                     kRandomAccess, nullptr);
    if (f->handle == kInvalid) {
        *result = -1;
        return result;
    }
    f->position = 0;
    *result = 0;
    return result;
}

void __fastcall FileSource_ReleaseMapping(FileSource *f) {
    (*g_env->closeHandle)(f->mapping);
    f->mapping = nullptr;
    vcall<void>(f, 0x1c, f->view);
    f->view = nullptr;
}

void __fastcall FileSource_Close(FileSource *f) {
    FileSource_ReleaseMapping(f);
    if (f->handle != kInvalid) (*g_env->closeHandle)(f->handle);
    f->handle = kInvalid;
}

u32 __fastcall FileSource_Read(FileSource *f, void *, void *buffer, u32 count) {
    u32 done = 0;
    if (count) (*g_env->readFile)(f->handle, buffer, count, &done, nullptr);
    f->position += done;
    return done;
}

u32 __fastcall FileSource_Write(FileSource *f, void *, const void *buffer, u32 count) {
    u32 done = 0;
    if (count) (*g_env->writeFile)(f->handle, buffer, count, &done, nullptr);
    f->position += done;
    return done;
}

u32 __fastcall FileSource_Seek(FileSource *f, void *, s32 offset, u32 origin) {
    f->position = (*g_env->setFilePointer)(f->handle, offset, nullptr, origin);
    return f->position;
}

u32 __fastcall FileSource_Size(FileSource *f) { return (*g_env->getFileSize)(f->handle, nullptr); }

u32 __fastcall FileSource_Tell(FileSource *f) { return f->position; }

void *__fastcall FileSource_Map(FileSource *f, void *, u32 offset, u32 size) {
    if (f->handle == kInvalid) return nullptr;
    if (!f->mapping) f->mapping = (*g_env->createFileMapping)(f->handle, nullptr, kPageReadOnly, 0, 0, nullptr);
    if (!f->mapping) return nullptr;
    if (!offset && !size) {
        if (!f->view) f->view = (*g_env->mapViewOfFileEx)(f->mapping, kMapRead, 0, 0, 0, nullptr);
        return f->view;
    }
    u32 info[kSystemInfoWords];
    (*g_env->getSystemInfo)(info);
    const u32 granularity = info[kGranularity];
    u32 base = 0, bytes = 0;
    if (static_cast<s32>(offset) > static_cast<s32>(granularity)) {
        base = offset / granularity * granularity;
        bytes = size - base + offset;
    }
    u8 *const view = static_cast<u8 *>((*g_env->mapViewOfFileEx)(f->mapping, kMapRead, 0, base, bytes, nullptr));
    return view ? view + (offset - base) : nullptr;
}

void __fastcall FileSource_Unmap(FileSource *f, void *, void *view) {
    if (f->mapping && view) (*g_env->unmapViewOfFile)(view);
}

s32 *__fastcall FileSource_Flush(FileSource *f, void *, s32 *result) {
    *result = f->handle != kInvalid && (*g_env->flushFileBuffers)(f->handle) ? 0 : -1;
    return result;
}

Semaphore *__fastcall FileSource_Lock(FileSource *f) { return &f->lock; }

MmioSource *__fastcall MmioSource_Ctor(MmioSource *m) {
    RefObject_Ctor(&m->base);
    m->handle = nullptr;
    m->position = 0;
    Semaphore_Ctor(&m->lock);
    m->base.vtable = kMmioSourceVtable;
    return m;
}

void __fastcall MmioSource_Dtor(MmioSource *m) {
    m->base.vtable = kMmioSourceVtable;
    MmioSource_Close(m);
    Semaphore_Dtor(&m->lock);
    RefObject_Dtor(&m->base);
}

MmioSource *__fastcall MmioSource_DeletingDtor(MmioSource *m, void *, u32 flags) {
    MmioSource_Dtor(m);
    if (flags & 1) Handles_GetEnv()->free(m);
    return m;
}

s32 *__fastcall MmioSource_Open(MmioSource *m, void *, s32 *result, const char *path, u32 mode) {
    vcall<void>(m, 0x24);
    s32 resolved;
    Path_Resolve(&resolved, g_env->mmioPath, kPathSize, path);
    if (mode & kGenericWrite) {
        *result = -1;
        return result;
    }
    m->handle = (*g_env->mmioOpen)(g_env->mmioPath, nullptr, 0);
    if (!m->handle) {
        *result = -1;
        return result;
    }
    m->position = 0;
    (*g_env->mmioSetBuffer)(m->handle, nullptr, 0x4000, 0);
    *result = 0;
    return result;
}

void __fastcall MmioSource_Close(MmioSource *m) {
    if (m->handle) (*g_env->mmioClose)(m->handle, 0);
    m->handle = nullptr;
}

s32 __fastcall MmioSource_Read(MmioSource *m, void *, void *buffer, s32 count) {
    s32 done = 0;
    if (count) done = (*g_env->mmioRead)(m->handle, buffer, count);
    m->position += done;
    return done;
}

s32 __fastcall MmioSource_Write(MmioSource *m, void *, const void *buffer, s32 count) {
    s32 done = 0;
    if (count) done = (*g_env->mmioWrite)(m->handle, buffer, count);
    m->position += done;
    return done;
}

s32 __fastcall MmioSource_Seek(MmioSource *m, void *, s32 offset, u32 origin) {
    const s32 from = origin == 0 ? 0 : origin == 1 ? 1 : 2;  // SEEK_SET / SEEK_CUR / SEEK_END
    m->position = static_cast<u32>((*g_env->mmioSeek)(m->handle, offset, from));
    return static_cast<s32>(m->position);
}

s32 __fastcall MmioSource_Size(MmioSource *m) {
    const s32 end = (*g_env->mmioSeek)(m->handle, 0, 2);
    (*g_env->mmioSeek)(m->handle, static_cast<s32>(m->position), 0);
    return end;
}

void *__fastcall MmioSource_Map(MmioSource *, void *, u32, u32) { return nullptr; }

s32 *__fastcall MmioSource_Flush(MmioSource *m, void *, s32 *result) {
    *result = m->handle && (*g_env->mmioFlush)(m->handle, 0) == 0 ? 0 : -1;
    return result;
}

}  // extern "C"
