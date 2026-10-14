// The WAD archive source: see wadsource.h.
#include "wadsource.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "file.h"
#include "wadfind.h"
#include "wrapsources.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const WadSourceEnv kGameEnv = {
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
};
const WadSourceEnv *g_env = &kGameEnv;

void *const *const kWadSourceVtable = at<void *const *>(0x00703908);
void *const *const kFileStreamVtable = at<void *const *>(0x006fd930);
const void *const kWadStringListVtable = at<const void *>(0x0070393c);
const void *const kStringListVtable = at<const void *>(0x006fe680);

constexpr u32 kMagic = 0x42465744;  // "DWFB"
constexpr u32 kDirectory = 0x10;    // FILE_ATTRIBUTE_DIRECTORY

// A DirEntry with room for a path (the originals' are on the stack).
struct EntryBuffer {
    DirEntry entry;
    char rest[0x103];
};

// The low byte is the version test; the rest is whatever word it was set in.
u32 in_block(u32 word, u32 version) {
    return (word & ~0xffu) | (version == 2 ? 1u : 0u);
}

void free_nodes(StringListNode *n) {
    while (n) {
        StringListNode *const next = n->next;
        String_Dtor(&n->text);
        g_env->free(n);
        n = next;
    }
}

const StringView *view(const String *s) {
    return reinterpret_cast<const StringView *>(s);
}

}  // namespace

void WadSource_SetEnv(const WadSourceEnv *env) {
    g_env = env ? env : &kGameEnv;
}

extern "C" {

WadSource *__fastcall WadSource_Ctor(WadSource *w) {
    RefObject2_Ctor(&w->base);
    String_CtorStr(&w->folder, nullptr, "");
    w->unknown38 = -1;
    w->list.head = nullptr;
    w->list.tail = nullptr;
    w->list.count = 0;
    w->list.vtable = kWadStringListVtable;
    String_Ctor(&w->pattern);
    StreamBase_Ctor(&w->file);
    w->file.vtable = kFileStreamVtable;
    w->source = nullptr;
    WadDir_Ctor(&w->dir);
    w->base.vtable = kWadSourceVtable;
    return w;
}

void __fastcall WadSource_Dtor(WadSource *w) {
    w->base.vtable = kWadSourceVtable;
    WadSource_Close(w);
    WadDir_Dtor(&w->dir);
    StreamBase_Dtor(&w->file);
    String_Dtor(&w->pattern);
    StringList_Dtor(&w->list);
    String_Dtor(&w->folder);
    RefObject2_Dtor(&w->base);
}

WadSource *__fastcall WadSource_DeletingDtor(WadSource *w, void *, u32 flags) {
    WadSource_Dtor(w);
    if (flags & 1) g_env->free(w);
    return w;
}

void __fastcall WadSource_Close(WadSource *w) {
    WadDir_Clear(&w->dir);
    vcall<void>(&w->file, 4);
    String_AssignStr(&w->folder, nullptr, "");
    String_AssignStr(&w->base.path, nullptr, "");
    w->source = nullptr;
}

s32 *__fastcall WadSource_Open(WadSource *w, void *, s32 *result, const char *path, u32 flags) {
    vcall<void>(w, 0x30);
    String_AssignStr(&w->base.path, nullptr, "");
    String_AssignStr(&w->folder, nullptr, "");
    String s;
    String_CtorStr(&s, nullptr, path);
    char *const buffer = String_GetBuffer(&s, nullptr, s.length);
    for (s32 i = String_Find(&s, nullptr, "/", 0); i > -1; i = String_Find(&s, nullptr, "/", i)) buffer[i] = '\\';
    String_ReleaseBuffer(&s, nullptr, 0x7fffffff);
    const s32 star = String_Find(&s, nullptr, "*\\", 0);
    if (star > -1) {
        StringView part;
        String_Assign(&w->folder, nullptr, String_From(&s, nullptr, &part, star + 1));
        const char separator = '\\';
        if (String_NotEqualsChar(reinterpret_cast<const String *>(String_Right(&w->folder, nullptr, &part, 1)),
                                 nullptr, &separator))
            String_AppendStr(&w->folder, nullptr, "\\");
        String_Assign(&s, nullptr, String_Left(&s, nullptr, &part, star));
    }
    s32 status;
    char full[0x104];
    Path_Resolve(&status, full, sizeof full, s.data);
    bool opened = false;
    if (!((*Files_GetEnv()->getFileAttributes)(full) & kDirectory)) {
        const u32 mode = flags & 0x8000 ? 0x80008001 : 0x80000001;
        if (*vcall<s32 *>(&w->file, 0x24, &status, full, mode) >= 0) {
            w->source = w->file.source;
            void *const block = g_env->alloc(0x10);
            TrackedSource *const shared = block ? SharedSource_Ctor(static_cast<TrackedSource *>(block)) : nullptr;
            StreamBase_Attach(&w->file, nullptr, reinterpret_cast<RefObject *>(shared));
            if (vcall<s32>(&w->file, 8, &w->header, 0x58) == 0x58 && w->header.magic == kMagic &&
                w->header.version <= 2) {
                WadDir_Load(&w->dir, nullptr, &status, &w->file,
                            in_block(static_cast<u32>(reinterpret_cast<std::uintptr_t>(block)), w->header.version));
                EntryBuffer found;
                found.entry.path[0] = 0;
                found.entry.name = found.entry.path;
                bool ok = true;
                if (String_NotEqualsStr(&w->folder, nullptr, "")) {
                    ok = *WadDir_FindFirst(&w->dir, nullptr, &status, &found.entry, &w->folder) >= 0 &&
                         found.entry.attributes == kDirectory;
                }
                if (ok) {
                    String_AssignStr(&w->base.path, nullptr, full);
                    String_AppendStr(&w->base.path, nullptr, "*");
                    String_Append(&w->base.path, nullptr, view(&w->folder));
                    opened = true;
                }
            }
            if (!opened) vcall<void>(&w->file, 4);
        }
    }
    *result = opened ? 0 : -1;
    String_Dtor(&s);
    return result;
}

s32 *__fastcall WadSource_FindFirst(WadSource *w, void *, s32 *result, const char *pattern, DirEntry *e) {
    if (!w->source || !pattern || !*pattern) {
        *result = -1;
        return result;
    }
    // Back to the pattern's last separator; one byte before the pattern is read when it has none.
    const char *p = pattern + std::strlen(pattern) - 1;
    char c = *p;
    while (c != '\\' && c != '/' && reinterpret_cast<std::uintptr_t>(p) >= reinterpret_cast<std::uintptr_t>(pattern))
        c = *--p;
    std::memcpy(e->path + std::strlen(e->path), pattern, std::strlen(pattern) + 1);
    e->name = e->path + (p - pattern) + 1;
    String unused;
    String_CtorCopy(&unused, nullptr, view(&w->base.path));
    String_AppendStr(&unused, nullptr, pattern);
    String_AssignWide(&w->pattern, nullptr, reinterpret_cast<const WString *>(&w->folder));
    String_AppendStr(&w->pattern, nullptr, e->path);
    s32 status;
    if (*WadDir_FindFirst(&w->dir, nullptr, &status, e, &w->pattern) < 0) {
        *result = -1;
    } else {
        WadSource_FillEntry(w, nullptr, e);
        *result = 0;
    }
    String_Dtor(&unused);
    return result;
}

void __fastcall WadSource_FillEntry(const WadSource *w, void *, DirEntry *e) {
    const s32 length = static_cast<s32>(w->folder.length);
    if (length > 0) {
        const char *const from = e->path + length - 1;
        std::memmove(e->path, from, std::strlen(from) + 1);
    }
}

s32 *__fastcall WadSource_FindNext(WadSource *w, void *, s32 *result, DirEntry *e) {
    s32 status;
    if (!w->source || *WadDir_FindNext(&w->dir, nullptr, &status, e) < 0) {
        *result = -1;
        return result;
    }
    WadSource_FillEntry(w, nullptr, e);
    *result = 0;
    return result;
}

s32 *__fastcall WadSource_OpenEntry(WadSource *w, void *, s32 *result, StreamBase *stream, const char *name,
                                    u32 flags, u32) {
    if (!w->source || (flags & 0x40000012) || stream->source) {
        *result = -1;
        return result;
    }
    StreamBase_Attach(stream, nullptr, w->source);
    return WadSource_OpenEntryStream(w, nullptr, result, stream, name);
}

s32 *__fastcall WadSource_OpenEntryStream(WadSource *w, void *, s32 *result, StreamBase *stream,
                                          const char *name) {
    String path;
    String_CtorCopy(&path, nullptr, view(&w->folder));
    String_AppendStr(&path, nullptr, name);
    void *const block = g_env->alloc(0x10);
    TrackedSource *const shared = block ? SharedSource_Ctor(static_cast<TrackedSource *>(block)) : nullptr;
    StreamBase_Attach(stream, nullptr, reinterpret_cast<RefObject *>(shared));
    s32 status;
    if (!w->dir.root) WadDir_Load(&w->dir, nullptr, &status, &w->file, in_block(w->header.version, w->header.version));
    u32 offset;
    WadFinder finder;
    WadFinder_Ctor(&finder, nullptr, &w->dir.root, &path, &offset);
    if (static_cast<u8>(WadFinder_Done(&finder))) {  // its low byte (the rest is the cursor's)
        vcall<void>(stream, 4);
        *result = -1;
    } else {
        WadRecord record;
        vcall<s32>(stream, 0x10, offset, 0);
        vcall<s32>(stream, 8, &record, 0x28);
        vcall<s32>(stream, 0x10, record.offset, 0);
        void *const sliceBlock = g_env->alloc(0x1c);
        SliceSource *const slice = sliceBlock ? SliceSource_Ctor(static_cast<SliceSource *>(sliceBlock)) : nullptr;
        if (*SliceSource_Open(slice, nullptr, &status, record.size, record.offset) < 0) {
            if (slice) vcall<void *>(slice, 0, 1);
            *result = -1;
        } else {
            StreamBase_Attach(stream, nullptr, reinterpret_cast<RefObject *>(slice));
            const u8 kind = static_cast<u8>(record.flags);
            if (kind & 1) {
                *result = -1;
            } else if (kind & 6) {
                void *const bufferBlock = g_env->alloc(0x18);
                BufferSource *buffer = nullptr;
                if (bufferBlock)
                    buffer = kind & 2 ? ZlibBufferSource_Ctor(static_cast<BufferSource *>(bufferBlock))
                                      : BufferSource_Ctor(static_cast<BufferSource *>(bufferBlock));
                if (*BufferSource_Open(buffer, nullptr, &status, record.unpacked) < 0) {
                    if (buffer) vcall<void *>(buffer, 0, 1);
                    *result = -1;
                } else {
                    StreamBase_Attach(stream, nullptr, reinterpret_cast<RefObject *>(buffer));
                    *result = 0;
                }
            } else {
                *result = 0;
            }
        }
    }
    WadFinder_Dtor(&finder);
    String_Dtor(&path);
    return result;
}

s32 *__fastcall WadSource_Vfn7(WadSource *, void *, s32 *result, u32, u32) {
    *result = -1;
    return result;
}

s32 *__fastcall WadSource_Vfn8(WadSource *, void *, s32 *result, u32) {
    *result = 0;
    return result;
}

s32 *__fastcall WadSource_Vfn9(WadSource *, void *, s32 *result, u32) {
    *result = -1;
    return result;
}

void __fastcall StringList_Dtor(StringList *l) {
    StringListNode *const head = l->head;
    l->vtable = kStringListVtable;
    l->tail = nullptr;
    l->head = nullptr;
    free_nodes(head);
    l->count = 0;
}

void __fastcall StringList_Clear(StringList *l) {
    StringListNode *const head = l->head;
    l->tail = nullptr;
    l->head = nullptr;
    free_nodes(head);
    l->count = 0;
}

StringList *__fastcall StringList_DeletingDtor(StringList *l, void *, u32 flags) {
    StringList_Dtor(l);
    if (flags & 1) g_env->free(l);
    return l;
}

StringListNode *__fastcall StringListNode_DeletingDtor(StringListNode *n, void *, u32 flags) {
    String_Dtor(&n->text);
    if (flags & 1) g_env->free(n);
    return n;
}

}  // extern "C"
