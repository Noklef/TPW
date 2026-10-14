// The image loaders' stream openers: see streamopen.h.
#include "streamopen.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const StreamOpenEnv kGameEnv = {
    at<s32 *(__cdecl *)(s32 *, char *, u32, const char *)>(0x005f5710),
    at<u32(__stdcall *const *)(const char *)>(0x006fd10c),
    at<Handle *(__fastcall *)(Handle *)>(0x005f8240),
    at<void(__fastcall *)(Handle *)>(0x005f82a0),
    at<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32)>(0x005f85d0),
    at<s32 *(__fastcall *)(Handle *, void *, s32 *, u32)>(0x005f8510),
    at<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32, u32, u32)>(0x005f83d0),
    at<String *(__fastcall *)(String *, void *, const char *)>(0x005f60b0),
    at<String *(__fastcall *)(String *, void *, const StringView *)>(0x005f6040),
    at<void(__fastcall *)(String *)>(0x005f61e0),
    at<s32(__fastcall *)(const String *, void *, const char *, u32)>(0x005f7b90),
    at<StringView *(__fastcall *)(const String *, void *, StringView *, u32)>(0x005f7d60),
    at<StringView *(__fastcall *)(const String *, void *, StringView *, u32)>(0x005f7dd0),
    at<void *const *>(0x006fda18),
    at<void *const *>(0x006fd9e8),
    at<RefObject *(__fastcall *)(StreamBase *)>(0x004217e0),
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<BufferedSource *(__fastcall *)(BufferedSource *)>(0x00638ef0),
    at<s32 *(__fastcall *)(BufferedSource *, void *, s32 *, u32)>(0x00638f20),
    at<void(__fastcall *)(StreamBase *, void *, RefObject *)>(0x005f7240),
};
const StreamOpenEnv *g_env = &kGameEnv;

const u32 kPathSize = 0x104;  // MAX_PATH
const u32 kInvalidAttributes = 0xffffffff;
const u32 kDirectory = 0x10;
const u32 kEntryFlags = 0x80000001;

// The handle's own OpenEntry (vtable +0xc).
void openEntry(Handle *h, s32 *out, StreamBase *s, const char *name) {
    reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32, u32, u32)>(h->vtable[3])(
        h, nullptr, out, reinterpret_cast<u32>(s), reinterpret_cast<u32>(name), kEntryFlags, 0);
}

// The path split at its last separator: the folder opened, then the name in it.
void openByPath(StreamBase *s, s32 *out, Handle *folder, const char *path) {
    String full;
    g_env->stringCtor(&full, nullptr, path);
    const char slash = '/', backslash = '\\';
    const s32 lastSlash = g_env->reverseFind(&full, nullptr, &slash, 0xffffffff);
    const s32 lastBackslash = g_env->reverseFind(&full, nullptr, &backslash, 0xffffffff);
    const s32 cut = lastBackslash > lastSlash ? lastBackslash : lastSlash;
    if (cut < 1) {
        *out = -1;
    } else {
        StringView view;
        String part;
        s32 result;
        g_env->stringCopy(&part, nullptr, g_env->left(&full, nullptr, &view, static_cast<u32>(cut)));
        const bool failed = *g_env->openFolder(folder, nullptr, &result, reinterpret_cast<u32>(part.data)) < 0;
        g_env->stringDtor(&part);
        if (failed) {
            *out = -1;
        } else {
            g_env->stringCopy(&part, nullptr, g_env->from(&full, nullptr, &view, static_cast<u32>(cut + 1)));
            g_env->openEntry(folder, nullptr, out, reinterpret_cast<u32>(s), reinterpret_cast<u32>(part.data),
                             kEntryFlags, 0);
            g_env->stringDtor(&part);
        }
    }
    g_env->stringDtor(&full);
}

}  // namespace

void StreamOpen_SetEnv(const StreamOpenEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

s32 *__fastcall StreamBase_OpenIn(StreamBase *s, void *, s32 *out, const char *name, const char *container) {
    if (s->source) {
        *out = -1;
        return out;
    }
    Handle wad, folder;
    g_env->handleCtor(&wad);
    wad.vtable = g_env->wadHandleVtable;
    g_env->handleCtor(&folder);
    folder.vtable = g_env->folderHandleVtable;
    char path[kPathSize];
    s32 result;
    if (container && *container) {
        g_env->resolve(&result, path, kPathSize, container);
        const u32 attributes = (*g_env->getAttributes)(path);
        if (attributes == kInvalidAttributes) {
            *out = -1;
        } else {
            Handle *h;
            s32 *opened;
            if (attributes & kDirectory) {
                h = &folder;
                opened = g_env->openFolder(&folder, nullptr, &result, reinterpret_cast<u32>(path));
            } else {
                h = &wad;
                opened = g_env->openWad(&wad, nullptr, &result, reinterpret_cast<u32>(container), 0);  // as given (sic)
            }
            if (*opened < 0)
                *out = -1;
            else
                openEntry(h, out, s, name);
        }
    } else {
        g_env->resolve(&result, path, kPathSize, name);
        openByPath(s, out, &folder, path);
    }
    g_env->handleDtor(&folder);
    g_env->handleDtor(&wad);
    return out;
}

s32 *__fastcall StreamBase_OpenBuffered(StreamBase *s, void *, s32 *out, StreamBase *other, u32 capacity) {
    RefObject *const source = g_env->getSource(other);
    void *const block = g_env->alloc(sizeof(BufferedSource));
    BufferedSource *const buffer = block ? g_env->bufferCtor(static_cast<BufferedSource *>(block)) : nullptr;
    if (buffer) {
        s32 result;
        if (*g_env->bufferOpen(buffer, nullptr, &result, capacity) < 0) {
            reinterpret_cast<void(__fastcall *)(BufferedSource *, void *, u32)>(buffer->base.vtable[0])(buffer,
                                                                                                       nullptr, 1);
            *out = -1;
            return out;
        }
    }
    g_env->attach(s, nullptr, source);
    g_env->attach(s, nullptr, reinterpret_cast<RefObject *>(buffer));  // its base; null: nothing
    *out = 0;
    return out;
}

}  // extern "C"
