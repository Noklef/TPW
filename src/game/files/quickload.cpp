#include "quickload.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/globalmem.h"
#include "../core/movs.h"
#include "vfs.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 indexes() { return global<u32>(0x006fe228); }
u32 &loaded() { return global<u32>(0x007a445c); }

QuickloadIndex *table(u32 second) {
    return second ? global<QuickloadIndex *>(0x007a3e5c) : global<QuickloadIndex *>(0x007a45dc);
}

// strcpy as inlined (rep movsd, then rep movsb, the NUL included), then the extension cut off.
void stem(char *to, const char *from) {
    u32 n = 0;
    while (from[n]) ++n;
    Rep_Movs(to, from, n + 1);
    char *dot = Crt().strrchr(to, '.');
    if (dot && dot[1] != '\\') *dot = 0;
}

}  // namespace

extern "C" {

void __cdecl Quickload_Init() {
    global<u32>(0x007a3d3c) = 1;
    global<void *>(0x007a45dc) = GlobalMem_Alloc(indexes() * 12, 1);
    global<void *>(0x007a3e5c) = GlobalMem_Alloc(indexes() * 12, 1);
}

void __cdecl Quickload_Free() {
    GlobalMem_Free(global<void *>(0x007a45dc));
    global<void *>(0x007a45dc) = nullptr;
    GlobalMem_Free(global<void *>(0x007a3e5c));
    global<void *>(0x007a3e5c) = nullptr;
    loaded() = 0;
    global<u32>(0x007a3d3c) = 0;
}

s32 __cdecl Quickload_Has(const char *name, u32 second, u32 index) {
    if (!loaded()) return -1;
    const QuickloadIndex &q = table(second)[index];
    for (u32 i = 0; i < q.count; ++i)
        if (!Crt().stricmp(name, q.names[i])) return 1;
    return 0;
}

s32 __cdecl Quickload_Find(const char *name, u32 second) {
    if (!loaded()) return -1;
    for (s32 e = 0; e < (s32)indexes(); ++e) {
        const QuickloadIndex &q = table(second)[e];
        for (u32 i = 0; i < q.count; ++i)
            if (!Crt().stricmp(name, q.names[i])) return e;
    }
    return -1;
}

u32 __cdecl Quickload_HasStem(const char *path, u32 index, u32 second) {
    char want[0x104], name[0x104];
    stem(want, path);
    if (!loaded()) return 0;
    const QuickloadIndex &q = table(second)[index];
    for (u32 i = 0; i < q.count; ++i) {
        stem(name, q.names[i]);
        if (!Crt().stricmp(want, name)) return 1;
    }
    return 0;
}

char *__cdecl Quickload_Locate(const char *name, u32 flags, s32 *index) {
    if (!name) return nullptr;
    if (!loaded()) flags &= ~2u;
    s32 found = 0;
    if (flags & 2) {
        const char *const slash = Crt().strrchr(name, '\\');
        found = Quickload_Find(slash ? slash + 1 : name, flags & 8);
        if (index) *index = found;
        if (found == -1) {
            if (!(flags & 4)) return nullptr;
            flags &= ~2u;
        }
    } else if (index) {
        *index = -1;
    }
    const u32 listed = flags & 2;
    const s32 slot = listed ? found : 0;
    u32 n = 0;
    while (name[n]) ++n;
    char *const directory = global<char **>(0x0079cb4c)[slot];
    char *const buffer = reinterpret_cast<char *>(static_cast<uintptr_t>(0x0079c9d0));
    Rep_Movs(buffer, name, n + 1);
    if (!listed) {
        if (!VFS_FileExists(buffer)) {
            if (index) *index = -1;
            return nullptr;
        }
        if (index) *index = slot;
    }
    return (flags & 1) ? buffer : directory;
}
}
