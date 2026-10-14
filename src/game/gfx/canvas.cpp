// The blending canvas: see canvas.h.
#include "canvas.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const CanvasEnv kGameEnv = {
    at<SurfaceInfo *(__fastcall *)(SurfaceInfo *)>(0x005fa680),
    {at<CanvasDriver *>(0x00fa7e3c), at<CanvasDriver *>(0x00fa7e38), at<CanvasDriver *>(0x00fa7e34),
     at<CanvasDriver *>(0x00fa7e30)},
    {at<void *const *>(0x00703650), at<void *const *>(0x00703680), at<void *const *>(0x007036b0),
     at<void *const *>(0x007036e0)},
};
const CanvasEnv *g_env = &kGameEnv;

// The first five words: the masks and the depth.
bool sameFormat(const SurfaceInfo &a, const SurfaceInfo &b) {
    return a.redMask == b.redMask && a.greenMask == b.greenMask && a.blueMask == b.blueMask &&
           a.alphaMask == b.alphaMask && a.bitCount == b.bitCount && (a.paletted == 0) == (b.paletted == 0);
}

}  // namespace

void Canvas_SetEnv(const CanvasEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

Canvas *__fastcall Canvas_Ctor(Canvas *c) {
    g_env->infoCtor(&c->target);
    g_env->infoCtor(&c->format);
    c->clip.left = c->clip.top = c->clip.right = c->clip.bottom = 0;
    c->target.bits = nullptr;
    c->driver = nullptr;
    c->flags = 0;
    c->changed = 0;
    return c;
}

void __fastcall Canvas_SetSurface(Canvas *c, void *, const SurfaceInfo *info) {
    if (!info) return;
    if (!sameFormat(c->target, *info)) c->changed = 1;
    c->target = *info;
    c->format = c->target;
    c->clip.left = 0;
    c->clip.top = 0;
    c->clip.right = static_cast<s32>(info->width);
    c->clip.bottom = static_cast<s32>(info->height);
    CanvasDriver *driver = nullptr;
    if (!c->format.paletted) {
        switch (c->format.bitCount) {
            case 8: driver = g_env->drivers[0]; break;
            case 16: driver = g_env->drivers[1]; break;
            case 24: driver = g_env->drivers[2]; break;
            case 32: driver = g_env->drivers[3]; break;
        }
    }
    c->driver = driver;
}

Rect *__fastcall Canvas_Clip(Canvas *c) { return &c->clip; }

void __cdecl CanvasDriver8_Init() { g_env->drivers[0]->table = g_env->tables[0]; }
void __cdecl CanvasDriver16_Init() { g_env->drivers[1]->table = g_env->tables[1]; }
void __cdecl CanvasDriver24_Init() { g_env->drivers[2]->table = g_env->tables[2]; }
void __cdecl CanvasDriver32_Init() { g_env->drivers[3]->table = g_env->tables[3]; }

}  // extern "C"
