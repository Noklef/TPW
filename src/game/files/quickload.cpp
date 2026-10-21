#include "quickload.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/globalmem.h"
#include "../core/movs.h"
#include "../core/vwrap.h"
#include "../gfx/texdirs.h"
#include "textscan.h"
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


// The located path and the directories built one after the other, as the original's stack has them (the directories' check of the
// byte before an empty folder reads the located path's last byte).
struct LoadPaths {
    char located[0x104];
    char directories[0x104];
};

u32 length(const char *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
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

s32 __cdecl Quickload_Exists(const char *name, u32 flags) {
    const char *const slash = Crt().strrchr(name, '\\');
    s32 found = 0;
    if (flags & 2) {
        found = Quickload_Has(slash ? slash + 1 : name, flags & 8, TexDirs_Current());
        if (!(flags & 4))
            return found;
    }
    if (found == 0)
        found = (s32)VFS_FileExists(name);
    return found;
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

u32 __cdecl Quickload_LoadRide(const char *name, const char *folder, const char *c) {
    if (loaded() && (s32)(indexes() - 1) >= 0) {
        u32 e = indexes() - 1;
        u32 left = indexes();
        do {
            for (u32 t = 0; t < 2; ++t) {
                QuickloadIndex *const q = (t == 0 ? global<QuickloadIndex *>(0x007a3e5c) : global<QuickloadIndex *>(0x007a45dc)) + e;
                GlobalMem_Free(q->names);
                q->names = nullptr;
                GlobalMem_Free(q->text);
                q->text = nullptr;
            }
            --e;
        } while (--left != 0);
    }
    loaded() = 0;
    LoadPaths paths;
    if (folder)
        Crt().sprintf(paths.located, reinterpret_cast<const char *>(0x007488ac), folder, name);   // "%s\%s"
    else
        Crt().sprintf(paths.located, name);   // (sic: the name is the format)
    s32 index;
    char *const directory = Quickload_Locate(paths.located, 0, &index);
    if (index == -1) return 0;   // (the original's "Could not find ride" goes to the empty Debug_Printf)
    if (global<u32>(0x007a3d3c)) {
        for (u32 pass = 0; pass < 2; ++pass) {
            char quick[0x104];
            const char *const base = global<char **>(0x0079cb4c)[index];
            QuickloadIndex *q;
            if (pass == 0) {
                Crt().sprintf(quick, reinterpret_cast<const char *>(0x0074d27c), base, folder);   // "%s\%s\qickload.txt"
                q = global<QuickloadIndex *>(0x007a45dc) + index;
            } else {
                Crt().sprintf(quick, reinterpret_cast<const char *>(0x0074d290), base, folder,
                              reinterpret_cast<const char *>(0x007af650));   // "%s\%s\%s\qickload.txt"
                q = global<QuickloadIndex *>(0x007a3e5c) + index;
            }
            u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(quick));
            char *data = nullptr;
            u32 size = 0;
            if (stream) {
                size = Object_CallSlot14(stream);
                data = static_cast<char *>(GlobalMem_Alloc(size, 1));
                if (data) Object_CallSlot8Product((u32)(uintptr_t)data, 1, size, stream);
                Object_FinishAndDelete(stream);
            }
            if (q) {
                q->count = 0;
                q->names = nullptr;
                q->text = nullptr;
            }
            if (!data) continue;
            // (the second pass's file found where the first's was not went to the empty Debug_Printf as a warning)
            loaded() = 1;
            u32 count = 0, text = 0;
            char *cursor = nullptr;
            for (u32 round = 0; round < 2; ++round) {
                TextScan scan = {data, 0, size};
                const u32 storing = round != 0;
                if (storing) {
                    q->text = static_cast<char *>(GlobalMem_Alloc(text, 1));
                    q->names = static_cast<char **>(GlobalMem_Alloc(count * 4, 1));
                    cursor = q->text;
                    q->count = count;
                    count = 0;
                }
                for (;;) {
                    if (TextScan_SkipBlanks(&scan)) break;
                    if (TextScan_IsBlank(&scan.next)) continue;
                    char token[0x100];
                    const u32 last = TextScan_Read(&scan, token, 0x100, 1, 1, 1, 1);
                    if (token[0]) {
                        const u32 n = length(token) + 1;
                        if (storing) {
                            Rep_Movs(cursor, token, n);
                            q->names[count] = cursor;
                            cursor += n;
                        } else {
                            text += n;
                        }
                        ++count;
                    }
                    if (last) break;
                }
            }
            GlobalMem_Free(data);
        }
    }
    TexDirs_SetCurrent((u32)index);
    const u32 n = length(folder);
    Rep_Movs(paths.directories, folder, n + 1);
    const char before = n ? paths.directories[n - 1] : paths.located[0x103];
    Crt().sprintf(paths.directories + n, reinterpret_cast<const char *>(before == '\\' ? 0x0074d1f0 : 0x0074d1ec),   // "%s" / "\%s"
                  reinterpret_cast<const char *>(0x007af650));
    if (!directory) return 0;
    TexDirs_Set(directory, folder, paths.directories, c);
    return 1;
}
}
