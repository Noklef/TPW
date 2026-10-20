#include "texdirs.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/globalmem.h"
#include "../files/quickload.h"
#include "../files/vfs.h"

namespace {

void *&chain() { return *reinterpret_cast<void **>(0x007b0628); }
const char **&table() { return *reinterpret_cast<const char ***>(0x007b062c); }
u32 &count() { return *reinterpret_cast<u32 *>(0x007b0630); }
s32 *indices() { return reinterpret_cast<s32 *>(0x0074d980); }
u32 &current() { return *reinterpret_cast<u32 *>(0x007b0620); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

// The extension from the last '.' replaced by ext, or ext appended when there is no '.'.
void set_extension(char *path, const char *ext) {
    char *const dot = Crt().strrchr(path, '.');
    std::strcpy(dot ? dot : path + std::strlen(path), ext);
}

}  // namespace

extern "C" {

const char *__cdecl TexDirs_First() { return indices()[0] != -1 ? table()[indices()[0]] : nullptr; }

u32 __cdecl TexDirs_Current() { return current(); }

void __cdecl TexDirs_SetCurrent(u32 value) { current() = value; }

void __cdecl TexDirs_Set(const char *base, const char *a, const char *b, const char *c) {
    const char *const names[3] = {a, b, c};
    const char *old[3];
    char paths[3][0x104];
    for (u32 k = 0; k < 3; ++k) old[k] = indices()[k] != -1 ? table()[indices()[k]] : nullptr;
    count() = 0;
    u32 size = 0;
    for (u32 k = 0; k < 3; ++k) {
        if (!names[k]) {
            indices()[k] = -1;
            continue;
        }
        const s32 length = static_cast<s32>(std::strlen(names[k]));
        Crt().sprintf(paths[k], names[k][length - 1] == '\\' ? "%s%s" : "%s%s\\", base, names[k]);
        if (!old[k] || std::strcmp(paths[k], old[k])) {
            old[k] = nullptr;
            size += std::strlen(paths[k]) + 1;
        }
        indices()[k] = static_cast<s32>(count()++);
    }
    char *to = nullptr;
    if (size) {
        void **const block = static_cast<void **>(GlobalMem_Alloc(size + count() * 4 + 4, 0));
        *block = chain();
        chain() = block;
        table() = reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(block + 1));
        to = reinterpret_cast<char *>(table() + count());
    }
    for (u32 k = 0; k < 3; ++k) {
        if (!names[k]) continue;
        if (old[k]) {
            table()[indices()[k]] = old[k];
        } else {
            table()[indices()[k]] = to;
            const u32 n = std::strlen(paths[k]) + 1;
            std::memcpy(to, paths[k], n);
            to += n;
        }
    }
}

void __cdecl TexDirs_Reset() {
    for (void *block = chain(); block;) {
        void *const next = *static_cast<void **>(block);
        GlobalMem_Free(block);
        block = next;
    }
    chain() = nullptr;
    table() = nullptr;
    count() = 0;
    current() = 0;
    for (u32 k = 0; k < 3; ++k) indices()[k] = -1;
}

void __cdecl TextureSet_Load(u8 *set, u32 probe, u32) {
    if (!set) return;
    at<const char **>(set, 0x4c) = table();
    at<u16>(set, 0x34) = static_cast<u16>(count());
    const char *const second = indices()[1] != -1 ? table()[indices()[1]] : nullptr;
    const char *const third = indices()[2] != -1 ? table()[indices()[2]] : nullptr;
    const u32 stems = current();
    u8 *entry = at<u8 *>(set, 0x54);
    for (s32 i = 0; i < static_cast<s32>(at<u16>(set, 0x36)); ++i, entry += 0x10) {
        const char *const name = at<const char *>(entry, 0xc);
        u32 found = 0;
        if (second) {
            if (*reinterpret_cast<u32 *>(0x007a445c) && !probe) {
                found = Quickload_HasStem(name, stems, 1);
            } else {
                char path[0x104];
                Crt().sprintf(path, "%s%s", second, name);
                set_extension(path, ".wct");
                found = VFS_FileExists(path);
                if (!found) {
                    set_extension(path, ".tga");
                    found = VFS_FileExists(path);
                }
            }
            if (found) at<const char *>(entry, 4) = at<const char **>(set, 0x4c)[indices()[1]];
        }
        if (third) {
            if (!found) at<const char *>(entry, 4) = at<const char **>(set, 0x4c)[indices()[2]];
            continue;
        }
        if (!found) at<const char *>(entry, 4) = at<const char **>(set, 0x4c)[0];
    }
}

void __cdecl TexDirs_SetForModel(u8 *info, u8 *model) {
    char path[0x104];
    const char *found = nullptr;
    for (s32 tries = 2;;) {
        const char *const name = at<const char *>(info, 8);
        const s32 n = static_cast<s32>(std::strlen(name));
        Crt().sprintf(path, name[n - 1] == '\\' ? "%s%s" : "%s\\%s", name, at<const char *>(at<u8 *>(info, 0x10), 4));
        if (tries == 1) set_extension(path, ".wct");
        found = Quickload_Locate(path, 0, nullptr);
        if (found || !--tries) break;
    }
    if (!found) found = path;
    TexDirs_Set(found, nullptr, at<const char *>(info, 8), at<const char *>(info, 0xc));
    TextureSet_Load(at<u8 *>(at<u8 *>(model, 0x24), 4), 1, 0);
}
}
