#include "tvstatic.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "display.h"
#include "palette.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

Surface *__cdecl TvStatic_CreateSurface() {
    Surface *s = static_cast<Surface *>(GameHeap_Alloc(0x50));
    if (s) {
        u8 *key = reinterpret_cast<u8 *>(&s->keyRgb);
        for (u32 i = 0; i < 5; ++i)
            key[i] = 0;
        s->attached = 0;
        s->surface = nullptr;
        SurfaceInfo_Ctor(&s->info);
    }
    u32 desc[0x1b];  // (only the fields below are set, as in the original)
    desc[0] = 0x6c;
    desc[1] = 7;
    desc[2] = 0xf0;
    desc[3] = 0x140;
    desc[0x1a] = 0x840;
    void *surface = s;  // (the original's out slot held the Surface)
    u8 *directDraw = *global<u8 **>(0x00879a24);
    s32 result;
    if (comcall<s32>(directDraw, 0x18, desc, &surface, nullptr) != 0)
        result = -1;
    else
        Surface_Attach(&result, surface, s);
    if (result < 0) {
        GameHeap_Free(s);
        return nullptr;
    }
    return s;
}

void __cdecl TvStatic_Init() { global<Surface *>(0x00874fd0) = TvStatic_CreateSurface(); }

void __cdecl TvStatic_Free() {
    void *surface = Surface_GetSurface(global<Surface *>(0x00874fd0), 1);
    comcall<u32>(surface, 8);
    GameHeap_Free(global<Surface *>(0x00874fd0));
}

void __cdecl TvStatic_Draw(void *screen) {
    const u8 *palette = reinterpret_cast<const u8 *>((uintptr_t)0x00fa71f8);
    DrawColour black, white, grey;
    black.rgb = 0xff000000;
    black.index = Palette_Nearest(palette, 0, 0, 0);
    white.rgb = 0xffffffff;
    white.index = Palette_Nearest(palette, 0xff, 0xff, 0xff);
    grey.rgb = 0xff969696;
    grey.index = Palette_Nearest(palette, 0x96, 0x96, 0x96);
    Surface *target = reinterpret_cast<Surface *>((uintptr_t)0x00874fd8);
    s32 out;
    Surface_Attach(&out, screen, target);
    Surface_Lock(&out, global<Surface *>(0x00874fd0), nullptr);
    Display_SetSurface(global<Surface *>(0x00874fd0));
    for (u32 x = 0; x < 0x140; ++x)
        for (u32 y = 0; y < 0xf0; ++y) {
            if (Crt().rand() & 3)
                Display_Pixel(x, y, black);
            else if (Crt().rand() & 1)
                Display_Pixel(x, y, white);
            else
                Display_Pixel(x, y, grey);
        }
    Surface_Unlock(global<Surface *>(0x00874fd0));
    const Rect to = {0, 0, global<s32>(0x007a1968), global<s32>(0x007a1930)};
    const Rect from = {0, 0, 0x140, 0xf0};
    Surface_Blt(&out, target, &to, global<Surface *>(0x00874fd0), &from, 0);
}
}
