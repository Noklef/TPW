#include "overlayload.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../model/holder.h"
#include "../ui/windowreg.h"
#include "overlay.h"
#include "texdirs.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
const u8 *fixed(u32 address) { return reinterpret_cast<const u8 *>((uintptr_t)address); }
u8 *field(u8 *p, u32 at) { return *reinterpret_cast<u8 **>(p + at); }

const OverlayLoadEnv kGameEnv = {&Holder_LoadMd2, &TextureSet_Load, &Overlay_CtorResized, &Overlay_CtorResizedHeight, &Overlay_Ctor, &GuiWindows_Register};
const OverlayLoadEnv *g_env = &kGameEnv;

// The directory and the name in `path`, the holder of that file with its textures, the overlay's memory (*holder: 0, nothing more: nullptr).
u8 *loaded(String *path, const WString *directory, const char *name, u8 **holder) {
    String_AssignWide(path, nullptr, directory);
    String_AppendStr(path, nullptr, name);
    *holder = g_env->load(path->data, 0, 0, 1, 0);
    if (!*holder) return nullptr;
    g_env->textures(field(*holder, 4), 0, 1);
    return static_cast<u8 *>(GameHeap_Alloc(0x84));
}

// The name less the length [0x7b96bc] (the extension's) of chars.
StringView *withoutSuffix(const String *name, StringView *view) { return String_Left(name, nullptr, view, name->length - global<u32>((u32)(uintptr_t)&gString_7b96b8 + 0x4)); }
}  // namespace

void OverlayLoad_SetEnv(const OverlayLoadEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__cdecl OverlayLoad_Button(OverlayFile file, u32, const String *directory) {
    String shorter;
    String_CtorStr(&shorter, nullptr, file.name);
    StringView view;
    String_Assign(&shorter, nullptr, withoutSuffix(&shorter, &view));
    const u8 *const name = reinterpret_cast<const u8 *>(&shorter);
    bool known = Names_Equal(name, fixed(0x007b9ab8)) || Names_Equal(name, fixed((u32)(uintptr_t)&gString_7b9a80));
    if (!known) {
        String cbut;
        String_CtorStr(&cbut, nullptr, reinterpret_cast<const char *>(0x0074fdf0));
        known = Names_Equal(name, reinterpret_cast<const u8 *>(&cbut)) != 0;
        String_Dtor(&cbut);
    }
    if (!known) {
        String_Dtor(&shorter);
        return nullptr;
    }
    u8 *result = nullptr;
    String path;
    String_Ctor(&path);
    u8 *holder;
    u8 *const memory = loaded(&path, reinterpret_cast<const WString *>(directory), file.name, &holder);
    if (holder) {
        u8 *const overlay = memory ? g_env->resized(memory, nullptr, holder) : nullptr;
        *reinterpret_cast<u16 *>(overlay + 0x12) = 1;
        g_env->registerWindow(overlay, shorter.data);
        result = overlay;
    }
    String_Dtor(&path);
    String_Dtor(&shorter);
    return result;
}

u8 *__cdecl OverlayLoad_Slider(OverlayFile file, u32, const String *directory) {
    String name;
    String_CtorStr(&name, nullptr, file.name);
    StringView view;
    String shorter;
    String_CtorCopy(&shorter, nullptr, withoutSuffix(&name, &view));
    const u32 equal = Names_Equal(reinterpret_cast<const u8 *>(&shorter), fixed((u32)(uintptr_t)&gString_7b93a8));
    String_Dtor(&shorter);
    if (!equal) {
        String_Dtor(&name);
        return nullptr;
    }
    u8 *result = nullptr;
    String path;
    String_Ctor(&path);
    u8 *holder;
    u8 *const memory = loaded(&path, reinterpret_cast<const WString *>(directory), file.name, &holder);
    if (holder) {
        u8 *const overlay = memory ? g_env->resizedHeight(memory, nullptr, holder) : nullptr;
        *reinterpret_cast<u16 *>(overlay + 0x12) = 1;
        g_env->registerWindow(overlay, reinterpret_cast<const char *>(0x0074fe00));
        result = overlay;
    }
    String_Dtor(&path);
    String_Dtor(&name);
    return result;
}

u8 *__cdecl OverlayLoad_Plain(OverlayFile file, u32, String directory) {
    u8 *result = nullptr;
    String path;
    String_Ctor(&path);
    const char *const name = file.name;
    const char *const shadow = reinterpret_cast<const char *>(0x0074fe14);
    if (*name != '!') {
        String_AssignWide(&path, nullptr, reinterpret_cast<const WString *>(&directory));
        String_AppendStr(&path, nullptr, name);
        const char *a = name, *b = shadow;
        while (*a == *b && *a) ++a, ++b;
        if (*a != *b) {
            u8 *const holder = g_env->load(path.data, 0, 0, 1, 0);
            if (holder) {
                g_env->textures(field(holder, 4), 0, 1);
                u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x84));
                u8 *const overlay = memory ? g_env->plain(memory, nullptr, holder) : nullptr;
                g_env->registerWindow(overlay, reinterpret_cast<const char *>(field(field(field(holder, 4), 0x78), 0x54)));
                result = overlay;
            }
        }
    }
    String_Dtor(&path);
    String_Dtor(&directory);
    return result;
}
}
