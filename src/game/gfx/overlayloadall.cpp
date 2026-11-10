#include "overlayloadall.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/class6fd9d8.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../files/quickload.h"
#include "../core/staticobjects.h"

namespace {

const OverlayLoadAllEnv kGameEnv = {&Quickload_LoadRide, &NamedList_Find, reinterpret_cast<decltype(OverlayLoadAllEnv::first)>((uintptr_t)0x0041d570),
                                    reinterpret_cast<decltype(OverlayLoadAllEnv::next)>(&Class6fd9d8_Find)};
const OverlayLoadAllEnv *g_env = &kGameEnv;

const u8 *const kLanguage = reinterpret_cast<const u8 *>((uintptr_t)&gLanguage_78b530);
const String *const kPattern = reinterpret_cast<const String *>((uintptr_t)&gString_7b9760);

u32 findFolder(u32 key, u8 **folder) {
    return g_env->find(*reinterpret_cast<u8 ***>((uintptr_t)&gNamedObjects_786b68 + 0x34), nullptr, reinterpret_cast<const u8 *>((uintptr_t)key), folder);
}

// An empty entry: only its name's pointer and first character set.
void clear(OverlayFile &file) {
    char *const text = reinterpret_cast<char *>(&file) + 0x24;
    file.name = text;
    *text = 0;
}

}  // namespace

void OverlayLoadAll_SetEnv(const OverlayLoadAllEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __cdecl OverlayLoad_All() {
    g_env->loadRide(reinterpret_cast<const char *>((uintptr_t)0x0074fe28), reinterpret_cast<const char *>((uintptr_t)0x00747c7c), nullptr);
    u8 *folder = nullptr;
    if (findFolder(0x007b9b80, &folder) != 1) return 0;
    OverlayFile file;
    clear(file);
    if (g_env->first(folder, nullptr, kPattern, &file) != 1) return 0;
    const u32 unused = Get34_41ce80(folder);
    u32 count = 0;
    do {
        const String *const directory = reinterpret_cast<const String *>(folder + 0x18);
        if (OverlayLoad_Button(file, unused, directory) || OverlayLoad_Slider(file, unused, directory)) {
            ++count;
        } else {
            String copy;
            String_CtorCopy(&copy, nullptr, reinterpret_cast<const StringView *>(directory));
            if (OverlayLoad_Plain(file, unused, copy)) ++count;
        }
    } while (g_env->next(folder, nullptr, &file));
    return count;
}

u32 __cdecl OverlayLoad_Language() {
    u8 *folder = nullptr;
    if (findFolder((u32)(uintptr_t)&gString_7c2458, &folder) != 1) return 0;
    OverlayFile file;
    clear(file);
    String pattern, path;
    String_Ctor(&pattern);
    String_Ctor(&path);
    vcall<void>(&pattern, 0x14, Language_Buffer(kLanguage));
    String_EnsureTrailingBackslash(&pattern);
    vcall<void>(&pattern, 0x28, kPattern);
    if (g_env->first(folder, nullptr, &pattern, &file) == 1) {
        const u32 unused = Get34_41ce80(folder);
        bool first = true;
        do {
            if (first) {
                vcall<void>(&path, 0x18, folder + 0x18);
                String_EnsureTrailingBackslash(&path);
                vcall<void>(&path, 0x20, Language_Buffer(kLanguage));
                g_env->loadRide(file.name, path.data, reinterpret_cast<const char *>((uintptr_t)0x0074f89c));
                String_EnsureTrailingBackslash(&path);
                first = false;
            }
            String copy;
            String_CtorStr(&copy, nullptr, reinterpret_cast<const char *>((uintptr_t)Get8_51c050(reinterpret_cast<const u8 *>(&path))));
            u8 *const overlay = OverlayLoad_Plain(file, unused, copy);
            *reinterpret_cast<u32 *>(overlay + 0x80) = 1;  // (sic)
        } while (g_env->next(folder, nullptr, &file));
    }
    String_Dtor(&path);
    String_Dtor(&pattern);
    return 0;
}
