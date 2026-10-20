#include "objmodel.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vobj2.h"
#include "../gfx/loadscreen.h"
#include "../gfx/texset.h"
#include "../render/engineerrors.h"
#include "../render/mode.h"
#include "../terrain/procedural.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
u8 *ptr(const void *p, u32 offset) { return reinterpret_cast<u8 *>((uintptr_t)at<u32>(p, offset)); }

bool same(const char *a, const char *b) {
    for (; *a == *b; ++a, ++b)
        if (!*a)
            return true;
    return false;
}

const ObjModelEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0)};
const ObjModelEnv *g_env = &kGameEnv;

// The engine's static matrix (0xfb3400): the identity, built the first time under the guard 0xfb33e0 & 1.
u32 *static_matrix() {
    u8 &guard = *reinterpret_cast<u8 *>((uintptr_t)0x00fb33e0);
    u32 *const m = reinterpret_cast<u32 *>((uintptr_t)0x00fb3400);
    const u8 bits = guard;
    if (!(bits & 1)) {
        for (u32 i = 0; i < 16; ++i)
            m[i] = 0;
        guard = bits | 1;
        m[15] = m[10] = m[5] = m[0] = 0x3f800000;
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>(0x005d1750));
    }
    return m;
}

struct MatrixFrame {   // (the game's frame: the flip at +0x10, the copy at +0x50)
    u32 flip[16];
    u32 copy[16];
};

void transform_into(MatrixFrame &frame, u32 *into) {
    u32 *const out = static_matrix();
    using Slot = void(__cdecl *)(void *, u32 *, u32 *, u32 *);
    void *const transformer = *reinterpret_cast<void **>((uintptr_t)0x0087b1d8);
    reinterpret_cast<Slot>((*static_cast<void *const *const *>(transformer))[6])(transformer, frame.copy, frame.flip, out);
    for (u32 i = 0; i < 16; ++i)
        into[i] = out[i];
}

}  // namespace

void ObjModel_SetEnv(const ObjModelEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall ObjModel_Unload(u8 *object) {
    if (ptr(object, 8)) {
        if (*reinterpret_cast<const u8 *>(0x008bd550) & 0x20) {
            u8 *mesh = ptr(ptr(object, 8), 0x70);
            for (s32 i = 0; i < (s32)at<u16>(ptr(object, 8), 0x44); ++i, mesh += 0xa0) {
                if (at<u32>(mesh, 0) & 0x200) continue;
                s32 j = (s32)at<u16>(mesh, 0x5a) - 1;
                if (j >= 0) {
                    const u8 *face = ptr(mesh, 0x6c) + 0x10 * j;
                    for (; j >= 0; --j, face -= 0x10) {
                        const u8 *const p = ptr(face, 0);
                        if (p && (at<u16>(p, 0) & 0x1000)) break;
                    }
                }
                if (j >= 0) TrackedGlobal_Free(at<u32>(mesh, 0x74));
            }
        }
        Object_ReleaseSavedPairs130(object);
        Object_ReleaseSavedPairs(object);
        Object_FreeBlock13c(object);
        for (s32 i = 0; i < (s32)at<u16>(ptr(object, 8), 0x36); ++i) {
            u8 *const set = ptr(ptr(ptr(object, 8), 0x50), 8 * i + 4);
            if (set && TexSet_Release(set)) {
                GameHeap_Free(ptr(ptr(ptr(object, 8), 0x50), 8 * i + 4));
                at<u32>(ptr(ptr(object, 8), 0x50), 8 * i + 4) = 0;
            }
        }
        TrackedGlobal_Free(at<u32>(object, 0x144));
        const u8 *model = ptr(object, 8);
        at<u32>(object, 0x148) = 0;
        at<u32>(object, 0x144) = 0;
        for (s32 i = 0; i < (s32)at<u16>(model, 0x44); ++i) {
            at<u32>(ptr(model, 0x70), 0xa0 * i + 0x9c) = 0;
            model = ptr(object, 8);
        }
        if ((at<u8>(ptr(object, 8), 0x30) & 2) && *reinterpret_cast<u8 *const *>(0x00879d60))
            TerrainTex_Unload(*reinterpret_cast<u8 *const *>(0x00879d60));
    }
    const u32 flags = at<u32>(object, 4);
    at<u32>(object, 8) = 0;
    at<u32>(object, 4) = flags & ~0x10000000u;
    return 1;
}

u32 __fastcall ObjModel_Load(u8 *object, void *, u8 *model) {
    u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
    at<u8 *>(object, 8) = model;
    at<u32>(object, 4) = 0;
    if (at<u8>(model, 0x30) & 2) {
        u8 *const procedural = *reinterpret_cast<u8 *const *>(0x00879d60);
        if (!procedural || !at<u32>(model, 0x6c)) {
            EngineErrors_Push(errors, nullptr, 3, 0xb0754006);
            return 0;
        }
        TerrainTex_Setup(procedural, nullptr, model);
    }
    if (model && model + 0x18 && same(reinterpret_cast<const char *>(model + 0x18), reinterpret_cast<const char *>((uintptr_t)0x00763e54))) {
        for (s32 i = 0; i < (s32)at<u16>(model, 0x3c); ++i)   // ("BlueprintMesh": its parts flagged 2)
            at<u8>(ptr(model, 0x64), 0x10 * i + 0xc) |= 2;
    }
    u32 sharp = 0;
    if ((at<u32>(model, 0x30) & 0x200) && !(*reinterpret_cast<const u8 *>(0x008bcbc8) & 2)) {
        if ((*reinterpret_cast<const u32 *>(0x008bcbd0) == 0x200 && Flag54dcc0_Test()) ||
            (*reinterpret_cast<const u32 *>(0x008bcbd0) == 0x280 && !Flag54dcc0_Test()))
            sharp = 1;
    }
    u32 &render = *reinterpret_cast<u32 *>(0x008bd550);
    if (render & 0x100)
        render |= 0x40000000;
    for (s32 i = 0; i < (s32)at<u16>(ptr(object, 8), 0x36); ++i) {
        if (!at<u32>(ptr(ptr(object, 8), 0x50), 8 * i + 4)) {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x10));
            u8 *const set = block ? TexSet_Ctor(block) : nullptr;
            at<u8 *>(ptr(ptr(object, 8), 0x50), 8 * i + 4) = set;
        }
        const u8 *const loaded = ptr(object, 8);
        u8 *const pair = ptr(loaded, 0x50) + 8 * i;
        u8 *const set = ptr(pair, 4);
        if (!set)
            continue;
        TexSet_Init(set, nullptr, pair, ptr(loaded, 0x54) + 0x10 * i);
        if (!sharp)
            continue;
        for (s32 j = 0; j < (s32)at<u16>(ptr(ptr(object, 8), 0x54), 0x10 * i + 0xa); ++j) {
            u8 *const texture = reinterpret_cast<u8 *>((uintptr_t)TexSet_Texture(set, nullptr, j));
            if (texture)
                at<u32>(texture, 8) |= 0x4100000;
        }
    }
    if (render & 0x40000000) {
        TexSet_LoadDeferred();
        render &= ~0x40000000u;
    }
    const u8 *loaded = ptr(object, 8);
    const u32 block = TrackedGlobal_Alloc(((u32)at<u16>(loaded, 0x44) + (u32)at<u16>(loaded, 0x3a) * 4) * 4);
    at<u32>(object, 0x144) = block;
    if (!block) {
        EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
        ObjModel_Unload(object);
        return 0;
    }
    at<u32>(object, 0x148) = block;
    for (s32 i = 0; i < (s32)at<u16>(loaded, 0x44); ++i) {
        u8 *const mesh = ptr(loaded, 0x70) + 0xa0 * i;
        if (at<u16>(mesh, 0x5e)) {
            at<u32>(mesh, 0x9c) = at<u32>(object, 0x148);
            *reinterpret_cast<u32 *>((uintptr_t)at<u32>(object, 0x148)) = 0x7fffffff;
        }
        loaded = ptr(object, 8);
        at<u32>(object, 0x148) += (u32)at<u16>(ptr(loaded, 0x70), 0xa0 * i + 0x5e) * 4 + 4;
    }
    if (*reinterpret_cast<const u8 *>(0x008bd508) & 1)
        LoadScreen_Tick(reinterpret_cast<u8 *>((uintptr_t)0x008c4748));
    at<u32>(object, 4) |= 0x10000000;
    return 1;
}

void __fastcall ObjModel_SetMatrices(u8 *object, void *, u8 which, const u32 *matrix) {
    MatrixFrame frame;
    for (u32 i = 0; i < 16; ++i)
        frame.flip[i] = 0;
    frame.flip[15] = 0x3f800000;
    frame.flip[10] = 0x3f800000;
    frame.flip[0] = 0x3f800000;
    frame.flip[5] = 0xbf800000;
    for (u32 i = 0; i < 16; ++i)
        frame.copy[i] = matrix[i];
    if (which & 1) {
        transform_into(frame, &at<u32>(object, 0x10));
        at<u32>(object, 4) |= 1;
    }
    if (which & 2) {
        transform_into(frame, &at<u32>(object, 0x50));
        at<u32>(object, 4) |= 2;
    }
}
}
