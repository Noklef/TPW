// Tests on a directory entry: reimplementation of 0x5c53c0 and 0x5c5230.
#include "direntry.h"

#include <cstdint>

#include "../core/string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __cdecl DirEntry_IsWad(const DirEntry *entry) {
    const String *const wad = reinterpret_cast<const String *>((uintptr_t)0x00f80808);
    String name;
    String_CtorStr(&name, nullptr, entry->name);
    StringView view;
    String tail;
    String_CtorCopy(&tail, nullptr, String_Right(&name, nullptr, &view, wad->length));
    const u32 equal = Names_Equal(reinterpret_cast<const u8 *>(&tail), reinterpret_cast<const u8 *>(wad));
    String_Dtor(&tail);
    String_Dtor(&name);
    return equal != 0 ? 1 : 0;
}

u32 __cdecl DirEntry_IsRealDir(const u8 *entry) {
    if (at<u8>(entry, 4) & 0x10) {
        if (*at<const char *>(entry, 0x20) != '.')
            return 1;
    }
    return 0;
}

}  // extern "C"
