#include "loading.h"

#include <cstdint>

#include "../core/string.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

struct FileEntry {
    const u8 *file;  // the name at +0x20
    u32 value;
    u8 flag;
    u8 pad[3];
};
static_assert(sizeof(FileEntry) == 12, "the table's entries are 12 bytes");

FileEntry *entries() { return reinterpret_cast<FileEntry *>((uintptr_t)0x00875970); }
s32 count() { return global<s32>(0x00875430); }

const char *name(const FileEntry &e) { return *reinterpret_cast<const char *const *>(e.file + 0x20); }

}  // namespace

extern "C" {

void __cdecl SpriteFiles_Sort() {
    u32 swaps;
    do {
        swaps = 0;
        for (s32 i = 1; i < count(); i++) {
            FileEntry &cur = entries()[i], &prev = entries()[i - 1];
            String a, b;
            String_CtorStr(&a, nullptr, name(cur));
            String_CtorStr(&b, nullptr, name(prev));
            const u32 after = Names_FirstAfter(reinterpret_cast<const u8 *>(&b), reinterpret_cast<const u8 *>(&a));
            String_Dtor(&b);
            String_Dtor(&a);
            if (after) {
                const FileEntry was = cur;
                cur.file = prev.file;
                cur.value = prev.value;
                cur.flag = prev.flag;
                prev.file = was.file;
                prev.value = was.value;
                prev.flag = was.flag;
                swaps++;
            }
        }
    } while (swaps);
}
}
