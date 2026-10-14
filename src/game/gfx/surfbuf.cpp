// A surface that owns its DirectDraw surface: see surfbuf.h.
#include "surfbuf.h"

#include <cstdint>

namespace {

const SurfaceBufferEnv kGameEnv = {
    0x00703998,
    reinterpret_cast<void(__cdecl *)(void *)>(static_cast<std::uintptr_t>(0x0067acb0)),
};
const SurfaceBufferEnv *g_env = &kGameEnv;

}  // namespace

void SurfaceBuffer_SetEnv(const SurfaceBufferEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

SurfaceBuffer *__fastcall SurfaceBuffer_Ctor(SurfaceBuffer *b) {
    b->surface.keyRgb = 0;
    b->surface.keyIndex = 0;
    b->surface.attached = 0;
    b->surface.surface = nullptr;
    SurfaceInfo_Ctor(&b->surface.info);
    b->created = 0;
    b->borrowed = 0;
    b->vtable = g_env->vtable;
    return b;
}

SurfaceBuffer *__fastcall SurfaceBuffer_DeletingDtor(SurfaceBuffer *b, void *, u32 flags) {
    SurfaceBuffer_Dtor(b);
    if (flags & 1) g_env->free(b);
    return b;
}

void __fastcall SurfaceBuffer_Dtor(SurfaceBuffer *b) {
    b->vtable = g_env->vtable;
    SurfaceBuffer_Release(b);
}

s32 *__fastcall SurfaceBuffer_Create(SurfaceBuffer *b, void *, s32 *out, const u32 *size, u32 caps) {
    SurfaceBuffer_Release(b);
    s32 made;
    if (*Surface_Create(&made, size[0], size[1], caps, &b->surface) < 0) {
        *out = -1;
        return out;
    }
    b->created = 1;
    *out = 0;
    return out;
}

void __fastcall SurfaceBuffer_Release(SurfaceBuffer *b) {
    if (!b->created || b->borrowed) return;
    s32 released;
    Surface_Release(&released, &b->surface);
    b->created = 0;
}

}  // extern "C"
