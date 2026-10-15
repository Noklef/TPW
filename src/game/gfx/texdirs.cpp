#include "texdirs.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/globalmem.h"

namespace {

void *&chain() { return *reinterpret_cast<void **>(0x007b0628); }
const char **&table() { return *reinterpret_cast<const char ***>(0x007b062c); }
u32 &count() { return *reinterpret_cast<u32 *>(0x007b0630); }
s32 *indices() { return reinterpret_cast<s32 *>(0x0074d980); }
u32 &current() { return *reinterpret_cast<u32 *>(0x007b0620); }

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
}
