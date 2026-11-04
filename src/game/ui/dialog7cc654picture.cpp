// The picture in the online dialog: see dialog7cc654picture.h.
#include "dialog7cc654picture.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cc654PictureEnv kGameEnv = {game<void *(__fastcall *)(void *)>(0x005fa680),
                                         game<u8 *(__fastcall *)(u8 *, void *, u32)>(0x0065d7d0),
                                         game<u8 *(__fastcall *)(u8 *, void *, u32 *, u32 *)>(0x0065d6db),
                                         game<void(__cdecl *)(u8 *, void *, u32, u32)>(0x004925d0),
                                         game<void(__fastcall *)(void *, void *, void *, const char *, u32, u32)>(0x005fa8a0),
                                         game<void(__fastcall *)(void *, void *, void *, void *, const char *, u32, u32)>(0x005faa00),
                                         game<void(__fastcall *)(void *, void *, u32, u32, u32)>(0x005d3830),
                                         game<void(__fastcall *)(void *, void *, u8 *, u32, u32, u32)>(0x005d4260),
                                         game<void(__fastcall *)(u8 *)>(0x0065d745),
                                         game<void(__fastcall *)(void *)>(0x005d3860)};
const Dialog7cc654PictureEnv *g_env = &kGameEnv;

const char *fileName() {
    const char *const name = *reinterpret_cast<const char **>(global<u8 *>(0x007cc660) + 0x30);
    return name ? name : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
}

// Array3D's fields read here.
struct Array {
    u32 width, height, unused;
    u8 *pixels;
    u32 rest[3];
};

// A loader on the stack: its vtable, then its surface information and its palette.
struct Loader {
    u32 vtable;
    u8 info[0x42c];
};

// The information the loader fills: a slot, then a surface's.
struct Information {
    u32 slot;
    u8 info[0x28];
};

}  // namespace

void Dialog7cc654Picture_SetEnv(const Dialog7cc654PictureEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __cdecl Dialog7cc654_ShowPicture() {
    Loader loader;
    g_env->surfaceCtor(loader.info);
    loader.vtable = 0x00700140;
    Information information;
    g_env->surfaceCtor(information.info);
    u8 *const window = g_env->findChild(global<u8 *>(0x007cc654), nullptr, 0xee4);
    u32 width, height;
    u8 *const pixels = g_env->lock(window, nullptr, &width, &height);
    if (!pixels) return;
    g_env->fillInfo(pixels, information.info, width, height);
    g_env->infoFromFile(&loader, nullptr, &information, fileName(), 0, 0);
    g_env->loadFromFile(&loader, nullptr, &information, reinterpret_cast<void *>((uintptr_t)0x007cc628), fileName(), 0, 0);
    Array array;
    g_env->arrayInit(&array, nullptr, global<u32>(0x007cc644), global<u32>(0x007cc648), 4);
    const u8 *from = global<const u8 *>(0x007cc640);
    u8 *to = array.pixels;
    for (u32 y = 0; y < array.height; ++y) {
        for (u32 x = 0; x < array.width; ++x, from += 4, to += 4) {
            to[0] = 0xff;
            to[1] = from[2];
            to[2] = from[1];
            to[3] = from[0];
        }
    }
    const u32 mode = global<u32>(0x008bd554);
    if (mode == 2) g_env->shrink(&array, nullptr, pixels, width, height, 2);
    else if (mode == 3) g_env->shrink(&array, nullptr, pixels, width, height, 1);
    g_env->unlock(window);
    g_env->arrayFree(&array);
}
