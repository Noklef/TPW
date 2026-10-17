// Tests on a directory entry: reimplementation of 0x5c53c0.
#include "direntry.h"

#include <cstdint>

#include "../core/string.h"

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

}  // extern "C"
