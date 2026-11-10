#include "signfonts.h"

#include <cstdint>
#include <cstring>

#include "../core/string.h"
#include "../core/vcall.h"
#include "vfs.h"
#include "../core/imports.h"

namespace {

const SignFontsEnv kGameEnv = {&VFS_ExtractFile};
const SignFontsEnv *g_env = &kGameEnv;

u8 *const kVector = reinterpret_cast<u8 *>((uintptr_t)0x00789c18);

// Slots of the string's vtable: assign a string, append a C string, append a string.
const u32 kAssign = 0x18, kAppendText = 0x20, kAppend = 0x28;

const String *Global(u32 address) { return reinterpret_cast<const String *>((uintptr_t)address); }

bool AddFont(String *from, String *to, const char *name) {
    vcall<void>(from, kAssign, Global(0x00f86558));
    vcall<void>(from, kAppend, Global(0x00f85a08));
    vcall<void>(from, kAppendText, name);
    vcall<void>(to, kAssign, Global(0x00f816e8));
    vcall<void>(to, kAppend, Global(0x00f85a08));
    vcall<void>(to, kAppend, Global(0x00f86750));
    vcall<void>(to, kAppend, Global(0x00f85a08));
    vcall<void>(to, kAppendText, name);
    if (!g_env->extract(from->data, to->data)) return false;
    using AddFontResource = s32(__stdcall *)(const char *);
    if (!(*reinterpret_cast<const AddFontResource *>((uintptr_t)IMPORT(Gdi32_AddFontResourceA)))(to->data)) return false;
    const u8 *const first = *reinterpret_cast<u8 *const *>(kVector + 4);
    u8 *const last = *reinterpret_cast<u8 **>(kVector + 8);
    for (const u8 *font = first; font != last; font += 0x14)
        if (String_Equals(reinterpret_cast<const String *>(font), nullptr, reinterpret_cast<const StringView *>(to))) return true;
    StringVector_InsertN(kVector, nullptr, last, 1, to);   // the game has the second inline
    return true;
}

}  // namespace

void SignFonts_SetEnv(const SignFontsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 __fastcall SignFonts_Add(const u8 *style, void *, u32) {
    String from, to;
    String_Ctor(&from);
    String_Ctor(&to);
    const char *const name = reinterpret_cast<const char *>(style + 0xc4);
    const char *const other = reinterpret_cast<const char *>(style + 0x254);
    const bool added = AddFont(&from, &to, name) && (!std::strcmp(name, other) || AddFont(&from, &to, other));
    String_Dtor(&to);
    String_Dtor(&from);
    return added;
}

}  // extern "C"
