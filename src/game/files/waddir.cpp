// The WAD archive's directory tree: see waddir.h.
#include "waddir.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const WadDirEnv kGameEnv = {
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<s32(__cdecl *)(s32)>(0x0067fa60),
};
const WadDirEnv *g_env = &kGameEnv;

const void *const kFolderListVtable = at<const void *>(0x00703e94);
const void *const kFolderListBaseVtable = at<const void *>(0x00703e98);
const void *const kFileListVtable = at<const void *>(0x00703e90);
const void *const kFileListBaseVtable = at<const void *>(0x00703e9c);
const void *const kStringListVtable = at<const void *>(0x006fe680);
const void *const kStringList2Vtable = at<const void *>(0x006fe67c);
const void *const kFinderListVtable = at<const void *>(0x0070393c);
const char kSeparators[] = "\\/";  // 0x775704
constexpr u32 kHeaderSize = 0x58, kRecordSize = 0x28, kPoolCount = 100;

// The header's words used (the rest are not read here).
constexpr u32 kHeaderCount = 0x48 / 4, kHeaderNamesOffset = 0x4c / 4, kHeaderNamesSize = 0x50 / 4;
// A record's words: [0] the name's hash, [1] the name's offset, [2] its length, [4] the size, [5]
// non-zero when packed, [6] the unpacked size.
constexpr u32 kRecordTag = 0, kRecordName = 1, kRecordNameLength = 2, kRecordSize4 = 4, kRecordPacked = 5,
              kRecordUnpacked = 6;

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

char *duplicate(const char *s) {
    const u32 n = length(s) + 1;
    char *const out = static_cast<char *>(g_env->alloc(n));
    copy(out, s, n);
    return out;
}

// `new T[100]` as MSVC6 lays it out: the count, then the elements, each constructed (the
// CRT's 0x67acf0); nullptr when the allocation failed.
template <typename T>
T *new_block(T *(__fastcall *ctor)(T *)) {
    u32 *const block = static_cast<u32 *>(g_env->alloc(4 + kPoolCount * sizeof(T)));
    if (!block) return nullptr;
    *block = kPoolCount;
    T *const items = reinterpret_cast<T *>(block + 1);
    for (u32 i = 0; i < kPoolCount; ++i) ctor(items + i);
    return items;
}

// `delete[]`: the elements destroyed last first (0x67afa0), then the block freed.
template <typename T>
void delete_block(T *items, void(__fastcall *dtor)(T *)) {
    u32 *const block = reinterpret_cast<u32 *>(items) - 1;
    for (s32 i = static_cast<s32>(*block) - 1; i >= 0; --i) dtor(items + i);
    g_env->free(block);
}

template <typename T>
void free_pool(WadPool *p, void(__fastcall *dtor)(T *)) {
    while (p->block) {
        WadPoolBlock *const next = p->block->next;
        if (p->block->items) delete_block(static_cast<T *>(p->block->items), dtor);
        g_env->free(p->block);
        p->block = next;
    }
}

template <typename T>
T *pool_next(WadPool *p, T *(__fastcall *ctor)(T *)) {
    if (p->used >= static_cast<s32>(kPoolCount)) {
        WadPoolBlock *const b = static_cast<WadPoolBlock *>(g_env->alloc(sizeof(WadPoolBlock)));
        b->next = p->block;
        b->items = new_block(ctor);
        p->block = b;
        p->used = 0;
    }
    return static_cast<T *>(p->block->items) + p->used++;
}

// The node list's nodes freed (the items stay), the list emptied under its base vtable.
void free_nodes(PtrList *l, const void *vtable) {
    PtrListNode *n = l->head;
    l->vtable = vtable;
    l->tail = nullptr;
    l->head = nullptr;
    while (n) {
        PtrListNode *const next = n->next;
        g_env->free(n);
        n = next;
    }
    l->count = 0;
}

void free_strings(StringList *l) {
    StringListNode *n = l->head;
    l->vtable = kStringListVtable;
    l->tail = nullptr;
    l->head = nullptr;
    while (n) {
        StringListNode *const next = n->next;
        String_Dtor(&n->text);
        g_env->free(n);
        n = next;
    }
    l->count = 0;
}

const String *as_string(const TextWriter *w) { return reinterpret_cast<const String *>(w); }

}  // namespace

void WadDir_SetEnv(const WadDirEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

WadDir *__fastcall WadDir_Ctor(WadDir *d) {
    d->root = nullptr;
    d->names = nullptr;
    d->finder = nullptr;
    WadPoolBlock *b = static_cast<WadPoolBlock *>(g_env->alloc(sizeof(WadPoolBlock)));
    d->folderPool.block = b;
    d->folderPool.used = 0;
    b->next = nullptr;
    d->folderPool.block->items = new_block(WadFolder_Ctor);
    b = static_cast<WadPoolBlock *>(g_env->alloc(sizeof(WadPoolBlock)));
    d->filePool.block = b;
    d->filePool.used = 0;
    b->next = nullptr;
    WadFile *const files = new_block(WadFile_Ctor);
    d->filePool.block->items = files;
    return d;
}

void __fastcall WadDir_Dtor(WadDir *d) {
    WadDir_Clear(d);
    WadFilePool_Free(&d->filePool);
    WadFolderPool_Free(&d->folderPool);
}

void __fastcall WadDir_Clear(WadDir *d) {
    if (WadFolder *const root = d->root) {
        WadFolder_Dtor(root);
        g_env->free(root);
    }
    g_env->free(d->names);
    if (WadFinder *const finder = d->finder) {
        WadFinder_Dtor(finder);
        g_env->free(finder);
    }
    d->root = nullptr;
    d->names = nullptr;
    d->finder = nullptr;
}

s32 *__fastcall WadDir_Load(WadDir *d, void *, s32 *result, StreamBase *stream, u32 inBlock) {
    const bool block = static_cast<u8>(inBlock) != 0;
    if (WadFolder *const old = d->root) {
        WadFolder_Dtor(old);
        g_env->free(old);
    }
    WadFolder *const root = static_cast<WadFolder *>(g_env->alloc(sizeof(WadFolder)));
    d->root = root ? WadFolder_Ctor(root) : nullptr;
    WadFinder *const f = static_cast<WadFinder *>(g_env->alloc(sizeof(WadFinder)));
    if (f) {
        String_Ctor(&f->pattern);
        f->folder = nullptr;
        f->cursor = nullptr;
        f->current = nullptr;
        String_Ctor(&f->path);
        f->folders = 1;
        StringList_Ctor(&f->a);
        f->a.vtable = kFinderListVtable;
        StringList_Ctor2(&f->b);
        f->b.vtable = kFinderListVtable;
    }
    d->finder = f;

    if (vcall<u32>(stream, 0x10, 0, 0)) {
        *result = -1;
        return result;
    }
    u32 header[kHeaderSize / 4];
    if (vcall<u32>(stream, 8, header, kHeaderSize) != kHeaderSize) {
        *result = -1;
        return result;
    }
    const u32 count = header[kHeaderCount], namesOffset = header[kHeaderNamesOffset];
    const u32 namesSize = header[kHeaderNamesSize];
    const u32 bytes = count * kRecordSize;
    u32 *const records = static_cast<u32 *>(g_env->alloc(bytes));
    if (vcall<u32>(stream, 8, records, bytes) != bytes) {
        g_env->free(records);
        *result = -1;
        return result;
    }
    if (block) {
        d->names = static_cast<char *>(g_env->alloc(namesSize));
        if (vcall<u32>(stream, 0x10, namesOffset, 0) != namesOffset ||
            vcall<u32>(stream, 8, d->names, namesSize) != namesSize) {
            g_env->free(records);
            g_env->free(d->names);
            *result = -1;
            return result;
        }
    }
    WadFolder *folder = d->root;
    for (u32 i = 0; i < count; ++i) {
        const u32 *const rec = records + i * (kRecordSize / 4);
        char *name;
        if (block) {
            name = d->names + rec[kRecordName] - namesOffset;
        } else {
            const u32 n = rec[kRecordNameLength] + 1;
            name = static_cast<char *>(g_env->alloc(n));
            if (vcall<u32>(stream, 0x10, rec[kRecordName], 0) != rec[kRecordName] ||
                vcall<u32>(stream, 8, name, n) != rec[kRecordNameLength] + 1) {
                g_env->free(name);
                g_env->free(records);
                *result = -1;
                return result;
            }
        }
        TextWriter text;
        TextWriter_Ctor(&text, nullptr, name, length(name) + 1, 0);
        WadDir_Normalize(&text);
        const s32 cut = String_ReverseFindOneOf(as_string(&text), nullptr, kSeparators, 0xffffffff);
        if (cut != -1) {
            name[cut] = 0;
            folder = WadFolder_AddPath(d->root, nullptr, name, &d->folderPool, inBlock);
        }
        const u32 size = rec[kRecordPacked] ? rec[kRecordUnpacked] : rec[kRecordSize4];
        WadFolder_AddFile(folder, nullptr, name + cut + 1, size, kHeaderSize + i * kRecordSize, rec[kRecordTag],
                          &d->filePool, inBlock);
        TextWriter_Dtor(&text);
    }
    g_env->free(records);
    *result = 0;
    return result;
}

TextWriter *__cdecl WadDir_Normalize(TextWriter *w) {
    char *p = w->data;
    for (u32 n = w->length; n; --n, ++p) *p = *p == '/' ? '\\' : static_cast<char>(g_env->tolower(*p));
    return w;
}

WadFolder *__fastcall WadFolder_Ctor(WadFolder *f) {
    f->owned = 0;
    f->folders.head = nullptr;
    f->folders.tail = nullptr;
    f->folders.count = 0;
    f->folders.vtable = kFolderListVtable;
    f->files.head = nullptr;
    f->files.tail = nullptr;
    f->files.count = 0;
    f->files.vtable = kFileListVtable;
    return f;
}

void __fastcall WadFolder_Dtor(WadFolder *f) {
    if (f->owned) g_env->free(f->name);
    free_nodes(&f->files, kFileListBaseVtable);
    free_nodes(&f->folders, kFolderListBaseVtable);
}

void __fastcall WadFolder_AddFile(WadFolder *f, void *, char *name, u32 size, u32 offset, u32 hash, WadPool *pool,
                                  u32 inBlock) {
    WadFile *const e = WadFilePool_Alloc(pool);
    if (static_cast<u8>(inBlock)) {
        e->owned = 0;
        e->name = name;
    } else {
        e->owned = 1;
        e->name = duplicate(name);
    }
    e->hash = hash;
    e->size = size;
    e->offset = offset;
    PtrList_Append(&f->files, nullptr, e);
}

WadFolder *__fastcall WadFolder_AddPath(WadFolder *f, void *, char *path, WadPool *pool, u32 inBlock) {
    TextWriter text;
    TextWriter_Ctor(&text, nullptr, path, length(path) + 1, 0);
    const s32 cut = String_FindOneOf(as_string(&text), nullptr, kSeparators, 0);
    WadFolder *found = nullptr;
    for (PtrListNode *n = f->folders.head; n; n = n->next) {
        WadFolder *const child = static_cast<WadFolder *>(n->item);
        StringView first;
        const StringView *const v = String_Left(as_string(&text), nullptr, &first, static_cast<u32>(cut));
        if (String_EqualsStr(reinterpret_cast<const String *>(v), nullptr, child->name)) {
            found = child;
            break;
        }
    }
    if (!found) {
        if (cut != -1) path[cut] = 0;
        found = pool_next(pool, WadFolder_Ctor);
        if (static_cast<u8>(inBlock)) {
            found->owned = 0;
            found->name = path;
        } else {
            found->owned = 1;
            found->name = duplicate(path);
        }
        PtrListNode *const n = static_cast<PtrListNode *>(g_env->alloc(sizeof(PtrListNode)));
        if (n) {
            n->item = found;
            n->next = nullptr;
            n->prev = nullptr;
        }
        n->next = f->folders.head;
        if (!f->folders.head) {
            f->folders.tail = n;
            f->folders.head = n;
        } else {
            f->folders.head->prev = n;
            f->folders.head = n;
        }
        ++f->folders.count;
    }
    WadFolder *const r = cut != -1 ? WadFolder_AddPath(found, nullptr, path + cut + 1, pool, inBlock) : found;
    TextWriter_Dtor(&text);
    return r;
}

WadFile *__fastcall WadFile_Ctor(WadFile *f) {
    f->owned = 0;
    return f;
}

void __fastcall WadFile_Dtor(WadFile *f) {
    if (f->owned) g_env->free(f->name);
}

void __fastcall WadFolderList_Dtor(PtrList *l) { free_nodes(l, kFolderListBaseVtable); }

void __fastcall WadFileList_Dtor(PtrList *l) { free_nodes(l, kFileListBaseVtable); }

PtrList *__fastcall WadFolderList_DeletingDtor(PtrList *l, void *, u32 flags) {
    WadFolderList_Dtor(l);
    if (flags & 1) g_env->free(l);
    return l;
}

PtrList *__fastcall WadFileList_DeletingDtor(PtrList *l, void *, u32 flags) {
    WadFileList_Dtor(l);
    if (flags & 1) g_env->free(l);
    return l;
}

PtrList *__fastcall WadFolderListBase_DeletingDtor(PtrList *l, void *, u32 flags) {
    free_nodes(l, kFolderListBaseVtable);
    if (flags & 1) g_env->free(l);
    return l;
}

PtrList *__fastcall WadFileListBase_DeletingDtor(PtrList *l, void *, u32 flags) {
    free_nodes(l, kFileListBaseVtable);
    if (flags & 1) g_env->free(l);
    return l;
}

void __fastcall WadFolderPool_Free(WadPool *p) { free_pool(p, WadFolder_Dtor); }

void __fastcall WadFilePool_Free(WadPool *p) { free_pool(p, WadFile_Dtor); }

WadFile *__fastcall WadFilePool_Alloc(WadPool *p) { return pool_next(p, WadFile_Ctor); }

StringList *__fastcall StringList_Ctor(StringList *l) {
    l->vtable = kStringListVtable;
    l->head = nullptr;
    l->tail = nullptr;
    l->count = 0;
    return l;
}

StringList *__fastcall StringList_Ctor2(StringList *l) {
    l->head = nullptr;
    l->tail = nullptr;
    l->count = 0;
    l->vtable = kStringList2Vtable;
    return l;
}

void __fastcall WadFinder_Dtor(WadFinder *f) {
    free_strings(&f->b);
    free_strings(&f->a);
    String_Dtor(&f->path);
    String_Dtor(&f->pattern);
}

}  // extern "C"
