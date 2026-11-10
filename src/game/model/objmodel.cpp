#include "objmodel.h"

#include <cstdint>
#include <cstring>

#include "../core/addrcalls.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vobj2.h"
#include "../gfx/loadscreen.h"
#include "../gfx/texcache.h"
#include "../gfx/texset.h"
#include "../gfx/texture.h"
#include "../core/vwrap.h"
#include "../math/x87.h"
#include "../render/engineerrors.h"
#include "../render/meshlist.h"
#include "../render/mode.h"
#include "../terrain/procedural.h"
#include "../terrain/texcache.h"
#include "../gfx/texcache.h"
#include "../render/renderer.h"
#include "../render/pointsprites.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
u8 *ptr(const void *p, u32 offset) { return reinterpret_cast<u8 *>((uintptr_t)at<u32>(p, offset)); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

AddrCalls g_treeCalls;
AddrCalls g_drawCalls;

// Whether a mesh of the tree goes to the clip test: its flags, the render flags, its band.
bool tree_mesh_wanted(const u8 *mesh) {
    const u32 flags = at<u32>(mesh, 0);
    if (flags & 0x8210)
        return false;
    if (!(flags & 0xc0000) && !(global<u32>(0x008bd508) & 0xc0000000))
        return false;
    if (!(flags & 0x2000))
        return true;
    if (!(flags & 0x4000000))
        return g_treeCalls.Th(0x0056ee90, global<u32>(0x00879d60), at<u32>(mesh, 0x90)) != 0;
    return gTerrainViewBox_8bd4e8[0] <= at<s8>(mesh, 0x92) && gTerrainViewBox_8bd4e8[1] <= at<s8>(mesh, 0x90) && gTerrainViewBox_8bd4e8[2] >= at<s8>(mesh, 0x93) && gTerrainViewBox_8bd4e8[3] >= at<s8>(mesh, 0x91);
}

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
    u8 &guard = gPointsprites_fb33e0;
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
void ObjModel_SetTreeCalls(const AddrCall *table, u32 count) { g_treeCalls.Set(table, count); }
void ObjModel_SetDrawCalls(const AddrCall *table, u32 count) { g_drawCalls.Set(table, count); }

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

u32 __fastcall ObjModel_LightMesh(u8 *object, void *, u8 *mesh, const u32 *matrix, const u32 *lights, u32 flags) {
    const u32 kind = at<u32>(mesh, 0);
    if (at<u16>(mesh, 0x5e) == 0 || (kind & 0x1000000))
        return 1;
    u32 *colours = reinterpret_cast<u32 *>(ptr(mesh, 0x9c));
    const u32 frame = global<u32>(0x008bd50c);
    if (*colours == frame && !(global<u32>(0x008bd508) & 0x40000000) && !(kind & 0xc0000))
        return 1;
    *colours++ = frame;
    if (!(at<u32>(mesh, 0) & 0x1000000) && !(global<u32>(0x008bcbc8) & 0x800)) {   // (the flag read again: the stamp may be the mesh's +0)
        global<u32>(0x00879dec) = 0;
        ListNodes_Drain(ptr(global<u8 *>(0x0087a6d0), 0x30));
        if (!(flags & 0x3000000))
            ListNodes_Drain(ptr(global<u8 *>(0x0087a6d0), 0x2c));
        if (!(global<u32>(0x008bcbc8) & 0x1000))
            Meshes_Ready(gMeshlist_87a248, nullptr, lights);
        if (global<u32>(0x008bcbc8) & 0x400) {
            // fld cam+0x34; fld cam+0x30; fsubr mesh+0x40; fld cam+0x38; fxch st2; fsubr mesh+0x44; ...; fsubr mesh+0x48
            const u8 *const camera = global<u8 *>(0x0087afe8);
            const float x = (float)(at<float>(mesh, 0x40) - X87Reg(at<float>(camera, 0x30)));
            const float y = (float)(at<float>(mesh, 0x44) - X87Reg(at<float>(camera, 0x34)));
            const float z = (float)(at<float>(mesh, 0x48) - X87Reg(at<float>(camera, 0x38)));
            u32 xb, yb, zb;
            std::memcpy(&xb, &x, 4);
            std::memcpy(&yb, &y, 4);
            std::memcpy(&zb, &z, 4);
            Meshes_SetDirection(gMeshlist_87a248, nullptr, xb, yb, zb);
        }
        Meshes_SetView(gMeshlist_87a248, nullptr, matrix, (u32)(uintptr_t)(mesh + 0x78));
    }
    if ((at<u32>(ptr(object, 8), 0x30) & 0x800000) && global<s32>(0x008bd524) == 4) {
        const u32 mask = at<u32>(gMeshlist_87a248, 0x2c);
        at<u32>(gMeshlist_87a248, 0x2c) = 0xffff0000;
        Meshes_LightMesh(gMeshlist_87a248, nullptr, colours, mesh);
        at<u32>(gMeshlist_87a248, 0x2c) = mask;
        return 1;
    }
    Meshes_LightMesh(gMeshlist_87a248, nullptr, colours, mesh);
    return 1;
}

// The terrain cell (x, y) of the map at [[0x879d60]] (width +0x18, cells +0x2c), read again each time; its index wraps as the original's.
static u32 terrain_cell(s32 x, s32 y) {
    const u8 *const map = **reinterpret_cast<u8 *const *const *>((uintptr_t)0x00879d60);
    const u32 index = static_cast<u32>(at<s32>(map, 0x18)) * static_cast<u32>(y) + static_cast<u32>(x);
    return *reinterpret_cast<const u32 *>((uintptr_t)(at<u32>(map, 0x2c) + index * 4));
}

static bool blended_cell(u32 cell) { return (cell & 0x80) && (cell & 0x200); }

u32 __fastcall ObjModel_BlendTerrainTiles(u8 *object, void *, s32 x0, s32 y0, s32 x1, s32 y1) {
    at<s32>(object, 0x12c) = 0;
    for (s32 y = y0; y <= y1; ++y)
        for (s32 x = x0; x <= x1; ++x)
            if (blended_cell(terrain_cell(x, y)))
                ++at<s32>(object, 0x12c);
    const s32 pairCount = at<s32>(object, 0x12c);
    u32 *const pairs = static_cast<u32 *>(GameHeap_Alloc(static_cast<u32>(pairCount) * 8));
    if (pairs)
        for (s32 i = 0; i < pairCount; ++i)
            pairs[2 * i] = pairs[2 * i + 1] = 0;
    const s32 count = at<s32>(object, 0x12c);
    at<u32 *>(object, 0x124) = pairs;
    u8 *sets = nullptr;
    if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(static_cast<u32>(count) * 0x10 + 4))) {
        sets = block + 4;
        at<s32>(block, 0) = count;
        CRT_VectorCtorIterator(sets, 0x10, count, reinterpret_cast<void(__fastcall *)(void *)>(&TexSet_Ctor),
                               reinterpret_cast<void *>((uintptr_t)0x00575bf0));
    }
    at<u8 *>(object, 0x128) = sets;
    u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
    if (!at<u32>(object, 0x124) && !sets) {
        Object_ReleaseSavedPairs(object);
        EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
        return 0;
    }
    u32 result = 1;
    s32 done = 0;
    for (s32 y = y0; y <= y1; ++y)
        for (s32 x = x0; x <= x1; ++x) {
            const u32 cell = terrain_cell(x, y);
            if (!blended_cell(cell))
                continue;
            u32 *const pairSlot = reinterpret_cast<u32 *>(ptr(ptr(object, 8), 0x50) + (cell >> 16) * 8 + 4);
            u32 *const slot = TexSet_TextureSlot(reinterpret_cast<const u8 *>((uintptr_t)*pairSlot), nullptr, 0);
            u8 *texture = nullptr;
            if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(100))) {
                at<s32>(block, 0) = 1;
                texture = block + 4;
                CRT_VectorVbaseCtorIterator(texture, 0x60, 1, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                            reinterpret_cast<void *>((uintptr_t)0x00590a70));
            }
            if (!texture) {   // (the links made so far stay: the pairs that would put them back are freed, sic)
                EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
                for (s32 i = done - 1; i >= 0; --i)
                    TexSet_Release(ptr(object, 0x128) + i * 0x10);
                GameHeap_Free(ptr(object, 0x124));
                if (u8 *const made = ptr(object, 0x128)) {   // (their dtor 0x575bf0 is a jump to TexSet_Release)
                    CRT_VectorDtorIterator(made, 0x10, at<s32>(made - 4, 0), reinterpret_cast<void(__fastcall *)(void *)>(&TexSet_Release));
                    GameHeap_Free(made - 4);
                }
                return 0;
            }
            u8 *const old = reinterpret_cast<u8 *>((uintptr_t)*slot);
            TerrainOverlay overlay;
            overlay.pixels = Texture_LockFlipped(old);
            overlay.width = at<s16>(old, 0x10);
            overlay.height = at<s16>(old, 0x12);
            overlay.bytes_per_pixel = gObjModel_BlendTerrainTiles_701a18[global<u32>(0x008bd554)];
            overlay.flags = cell & 0xffff;
            result &= TerrainTexCache_BlendTile(ptr(global<u8 *>(0x00879d60), 0x230), nullptr, texture, x, y, &overlay);
            TexCache_UnlinkItem(reinterpret_cast<u8 * &>(gTexcache_8c4b44), nullptr, reinterpret_cast<u8 *>((uintptr_t)*slot));
            TexSet_InitShared(ptr(object, 0x128) + done * 0x10, nullptr, (u32)(uintptr_t)texture);
            u8 *const set = ptr(object, 0x128) + done * 0x10;
            u32 *const pair = at<u32 *>(object, 0x124) + 2 * done;
            ++done;
            pair[1] = (u32)(uintptr_t)pairSlot;
            pair[0] = *pairSlot;
            *pairSlot = (u32)(uintptr_t)set;
        }
    if (result)
        at<u32>(object, 4) |= 0x20000000;
    return result;
}

u32 __fastcall ObjModel_DrawMeshTree(u8 *object, void *, u8 *mesh, const u32 *first, const u32 *second) {
    u32 a[16] = {0x3f800000, 0, 0, 0, 0, 0x3f800000, 0, 0, 0, 0, 0x3f800000, 0, 0, 0, 0, 0x3f800000};
    u32 b[16] = {0x3f800000, 0, 0, 0, 0, 0x3f800000, 0, 0, 0, 0, 0x3f800000, 0, 0, 0, 0, 0x3f800000};
    using Transform = void(__cdecl *)(u8 *, u8 *, const u32 *, u32 *);
    u8 *const lights = object + 0x98;
    for (; mesh; mesh = ptr(mesh, 8)) {
        u8 *const xf1 = global<u8 *>(0x0087b1d8);
        reinterpret_cast<Transform>((uintptr_t)at<u32>(ptr(xf1, 0), 0x18))(xf1, mesh + 0x10, first, a);
        u8 *const xf2 = global<u8 *>(0x0087b1d8);
        reinterpret_cast<Transform>((uintptr_t)at<u32>(ptr(xf2, 0), 0x18))(xf2, mesh + 0x10, second, b);
        if (tree_mesh_wanted(mesh)) {
            const u32 codes = g_treeCalls.Th(0x00561a30, global<u32>(0x00879a20), mesh + 0x78, a);
            if (codes & 0x40) {
                if (!g_treeCalls.Th(0x0057aa10, (u32)(uintptr_t)object, mesh, b, lights, codes | gRenderer_8bb624))
                    return 0;
                if (!g_treeCalls.Th(0x00581cd0, global<u32>(0x008bcbb8), a, b, lights, mesh, at<u32>(object, 0x14c) | codes | gRenderer_8bb624))
                    return 0;
                g_treeCalls.Th(0x0057a420, (u32)(uintptr_t)object, mesh);
            }
        }
        if (!(at<u8>(mesh, 0) & 0x20) && at<u32>(mesh, 0xc) && !ObjModel_DrawMeshTree(object, nullptr, ptr(mesh, 0xc), a, b))
            return 0;
    }
    return 1;
}

u32 __fastcall ObjModel_DrawOrQueue(u8 *part) {
    u8 *const object = part - 0x154;
    const u32 me = (u32)(uintptr_t)object;
    const u32 stampA = global<u32>(0x008bcc40), stampB = global<u32>(0x008bcc44);
    if (at<u32>(object, 0x90) == stampA && at<u32>(object, 0x94) == stampB) return 1;
    if ((at<u32>(ptr(object, 8), 0x30) & 0x200000) && !(at<u8>(object, 4) & 0x10)) {
        const s32 count = gObjModel_DrawOrQueue_8bb620;
        if (count >= 0x100) return 1;
        global<u32>(0x008bb220 + count * 4) = me;
        gObjModel_DrawOrQueue_8bb620 = count + 1;
        return 1;
    }
    at<u32>(object, 4) &= ~0x10u;
    const u32 prepare = at<u32>(object, 0x140);
    if (prepare && at<u32>(object, 0x90) != stampA) reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)prepare)(ptr(object, 8));
    at<u32>(object, 0x90) = stampA;
    at<u32>(object, 0x94) = stampB;
    at<u32>(object, 0x14c) = 0;
    if (global<u32>(0x008bd524) == 4) {
        at<u32>(object, 0x14c) = (global<u32>(0x008bcbc8) & 0x800) << 0x11;
        if (at<u32>(ptr(object, 8), 0x30) & 0x400000) at<u32>(object, 0x14c) |= 0x4000000;
        if (at<u32>(ptr(object, 8), 0x30) & 0x800000) at<u32>(object, 0x14c) |= 0x8000000;
    }
    if (at<u8>(object, 4) & 4) return 1;
    if (at<u32>(ptr(object, 8), 0x30) & 0x200) {
        gRenderer_8bb624 |= 0x2000000;
        return g_drawCalls.Th(0x0057a530, me, at<u32>(ptr(object, 8), 0x78), object + 0x10, object + 0x50);
    }
    gRenderer_8bb624 &= ~0x2000000u;
    if (global<u32>(0x008bcbc8) & 0x200000) {
        const u32 flags = at<u32>(ptr(object, 8), 0x30);
        if (!(flags & 2) && (flags & 0x800)) {
            if (!g_drawCalls.Th(0x0057b8a0, me)) return 0;   // Object_UpdateShadow
            const u8 *const shadow = at<const u8 *>(object, 0x13c);
            if (shadow) {
                g_drawCalls.Th(0x0058c9f0, gTexcache_8c4b44, at<u32>(shadow, 0x18), 0x3f000000u);   // TexCache_TouchItem (0.5f)
                g_drawCalls.Th(0x0057c270, 0x008bb638u, me);   // MeshQueue2_Add
            }
        }
    }
    u32 matrix[16];
    std::memcpy(matrix, global<const u32 *>(0x0087b088), sizeof(matrix));
    if (at<u8>(object, 4) & 1) {
        u32 *const result = static_matrix();
        u8 *const xf = global<u8 *>(0x0087b1d8);
        reinterpret_cast<void(__cdecl *)(u8 *, u8 *, const u32 *, u32 *)>((uintptr_t)at<u32>(ptr(xf, 0), 0x18))(xf, object + 0x10, matrix, result);
        std::memcpy(matrix, result, sizeof(matrix));
    }
    return g_drawCalls.Th(0x0057a530, me, at<u32>(ptr(object, 8), 0x78), matrix, 0x0087b090u);
}
}
