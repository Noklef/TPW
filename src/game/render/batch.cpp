#include "batch.h"

#include <cstdint>
#include <xmmintrin.h>

#include "depthsort.h"
#include "drawbuffer.h"
#include "fixedverts.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "vertexfog.h"
#include "texformat.h"
#include "../gfx/texset.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const BatchEnv kPortEnv = {&DrawBuffer_Begin, &DrawBuffer_End};
const BatchEnv *g_env = &kPortEnv;

}  // namespace

void Batch_SetEnv(const BatchEnv *env) { g_env = env ? env : &kPortEnv; }

namespace {
AddrCalls g_treeCalls;
}

void Batch_SetTreeCalls(const AddrCall *table, u32 count) { g_treeCalls.Set(table, count); }

extern "C" {

u8 *__fastcall Model_BeginBatch(u8 *model, void *, u32 index, u32 *size) {
    const u8 *const settings = at<u8 *>(model, 4);
    const u32 mode = (at<u32>(settings, 0) & 0xc0000) ? 2 : 1;
    const u8 *const entry = at<u8 *>(settings, 0x6c) + (index << 4);
    const u8 *const material = at<u8 *>(entry, 0);
    const u8 *const set = material ? at<u8 *>(material, 4) : nullptr;
    const u32 i = material && (global<u8>(0x008bd550) & 0x40) ? at<u16>(material, 2) : 0;
    const u8 *const texture = set ? reinterpret_cast<const u8 *>((uintptr_t)TexSet_Texture(set, nullptr, (s32)i)) : nullptr;
    const bool shaded = texture && (at<u32>(texture, 8) & 0x400) && !at<u32>(texture, 0x3c);
    const u32 kind = (at<u32>(settings, 0) & 0x2000000) || shaded ? 0x2c4 : 0x1c4;
    const u32 extra = texture && (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
    const u32 flags = TexLoad_DriverFlags(model, nullptr, entry, texture);
    u8 *const batch = Batch_Begin(global<u8 *>(0x00879ab0), nullptr, mode, 4, kind, flags, extra);
    if (!batch)
        return nullptr;
    const u32 word = (at<u32>(model, 8) >> 24) & 2;
    at<u32>(batch, 0) = shaded ? word | 0x80 : word;
    at<u32>(batch, 0x34) = texture ? at<u32>(texture, 0x44) : 0xffffffff;
    *size = kind == 0x1c4 ? 0x20 : 0x28;
    return batch;
}

u8 *__fastcall Batch_Begin(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra) {
    if ((mode & 2) && (at<u32>(object, 0xc) & flags) && !(global<u32>(0x008bd508) & 0x4000000))
        mode |= 4;
    u8 *buffer = global<u8 *>(0x008bcbc4);
    u8 *batch = DrawBuffer_Begin(buffer, nullptr, mode);
    at<u8 *>(object, 0x20) = batch;
    if (!batch)
        return nullptr;
    at<u32>(batch, 4) = flags;
    at<u32>(at<u8 *>(object, 0x20), 0x18) = texture;
    at<u32>(at<u8 *>(object, 0x20), 0x1c) = kind;
    at<u32>(at<u8 *>(object, 0x20), 8) = extra;
    at<u32>(at<u8 *>(object, 0x20), 0x34) = 0xffffffff;
    u32 size = 0;
    switch ((s32)kind) {
    case 0xc4:
        size = 0x18;
        break;
    case 0x1c4:
        size = 0x20;
        break;
    case 0x2c4:
        size = 0x28;
        break;
    case 0x3c4:
        size = 0x30;
        break;
    }
    at<u32>(object, 0x1c) = size;
    at<u32>(object, 0x28) = DrawBuffer_VertexLimit(global<u8 *>(0x008bcbc4));
    at<u32>(object, 0x2c) = DrawBuffer_IndexLimit(global<u8 *>(0x008bcbc4));
    return at<u8 *>(object, 0x20);
}

namespace {

void fog(u8 *object, u32 render, u8 *vertices, u32 count) {
    if (render & 0x4000) {
        VertexFog_Colour(vertices, count, at<s32>(object, 0x1c));
        at<u32>(at<u8 *>(object, 0x20), 4) &= ~0x100000u;
        at<u32>(at<u8 *>(object, 0x20), 4) |= 0x80000;
    } else if (render & 0x8000) {
        VertexFog_Alpha(vertices, count, at<s32>(object, 0x1c));
    }
}

}  // namespace

void __fastcall Batch_FogVertices(u8 *object, void *, u8 *vertices, u32 count) {
    const u32 render = global<u32>(0x008bcbc8);
    if (render & 0x2000)
        fog(object, render, vertices, count);
}

void __fastcall Batch_Fog(u8 *object, void *) {
    if (global<u32>(0x008bd524) != 1)
        return;
    u8 *batch = at<u8 *>(object, 0x20);
    fog(object, global<u32>(0x008bcbc8), at<u8 *>(batch, 0x28), at<u32>(batch, 0x20));
}

void __fastcall Batch_TransformVertices(u8 *object, void *, u32 arg) {
    u8 *const batch = at<u8 *>(object, 0x20);
    const u32 count = at<u32>(batch, 0x20);
    if (!count) return;
    using Slot = float(__cdecl *)(void *, u8 *, u32, u32, u32);
    void *const transformer = global<void *>(0x0087b1d8);
    void *const *const vtable = *static_cast<void *const *const *>(transformer);
    const float answer = reinterpret_cast<Slot>(vtable[3])(transformer, at<u8 *>(batch, 0x28), at<u32>(object, 0x1c), count, arg);
    at<float>(batch, 0x30) = (float)(X87Reg(answer) + at<float>(batch, 0x30));
}

u32 __fastcall Batch_Flush(u8 *object, void *) {
    if (at<u32>(at<u8 *>(object, 0x20), 0) & 0x40)
        Batch_Fog(object, nullptr);
    const u32 flags = at<u32>(at<u8 *>(object, 0x20), 0);
    if (flags & 0x20)
        Batch_TransformVertices(object, nullptr, flags & 0xf);
    u8 *batch = at<u8 *>(object, 0x20);
    if ((at<u32>(batch, 0) & 0x400) && at<u32>(batch, 0x20)) {
        const s32 kind = global<s32>(0x008bd524);
        const float half = kind == 4 || (global<u32>(0x008bcbc8) & 0x8000002) ? 0.0f : 0.5f;
        u8 *vertex = at<u8 *>(batch, 0x28);
        for (u32 count = at<u32>(batch, 0x20); count != 0; --count) {
            for (u32 axis = 0; axis < 8; axis += 4) {
                const s32 n = _mm_cvtss_si32(_mm_set_ss(at<float>(vertex, axis)));
                global<s32>(0x00fb34a4) = n;
                at<float>(vertex, axis) = (float)(X87Reg((double)n) - half);
            }
            vertex += at<u32>(object, 0x1c);
        }
    }
    if (global<s32>(0x008bd524) == 4 && !(at<u32>(at<u8 *>(object, 0x20), 0) & 0x100)) {
        batch = at<u8 *>(object, 0x20);
        FixedVerts_PackRounded(at<u8 *>(batch, 0x28), at<u32>(batch, 0x20), at<s32>(object, 0x1c));
    }
    batch = at<u8 *>(object, 0x20);
    at<float>(batch, 0x30) = (float)(at<float>(batch, 0x30) / X87Reg((double)at<s32>(batch, 0x20)));
    batch = at<u8 *>(object, 0x20);
    const u32 wanted = at<u32>(batch, 4);
    if ((wanted & at<u32>(object, 0xc)) || (at<u32>(batch, 0) & 0x200)) {
        const u32 answer = vcall<u32>(object, 0x10, batch);
        if (!answer) {
            at<u32>(object, 0x20) = 0;
            return 0;
        }
        at<u32>(at<u8 *>(object, 0x20), 0) |= 0x10;
    } else if (wanted != 0x800 && (global<s32>(0x008bd524) == 1 || (global<s32>(0x008bd524) == 4 && !(wanted & 0x1000)))) {
        u8 *const block = g_env->begin(global<u8 *>(0x008bcbc4), nullptr, 6);
        const s32 triangles = at<s32>(batch, 0x24) / 3;
        const u8 *const vertices = at<u8 *>(batch, 0x28);
        const u16 *index = at<u16 *>(batch, 0x2c);
        if (global<s32>(0x008bd524) == 1) {
            float *const keys = reinterpret_cast<float *>(block + 0x28);
            for (s32 i = 0; i < triangles; ++i, index += 3) {
                reinterpret_cast<u32 *>(keys)[i] = at<u32>(vertices, index[0] * at<u32>(object, 0x1c) + 8);
                keys[i] = (float)(X87Reg(at<float>(vertices, index[1] * at<u32>(object, 0x1c) + 8)) + keys[i]);
                keys[i] = (float)(X87Reg(at<float>(vertices, index[2] * at<u32>(object, 0x1c) + 8)) + keys[i]);
            }
            DepthSort_TrianglesFloat(keys, at<u8 *>(batch, 0x2c), 0, triangles - 1);
        } else {
            u32 *const keys = reinterpret_cast<u32 *>(block + 0x28);
            for (s32 i = 0; i < triangles; ++i, index += 3)
                keys[i] = at<u32>(vertices, index[0] * 0x18u + 8) + at<u32>(vertices, index[1] * 0x18u + 8) + at<u32>(vertices, index[2] * 0x18u + 8);
            DepthSort_Triangles(keys, at<u8 *>(batch, 0x2c), 0, triangles - 1);
        }
    }
    at<u32>(object, 0x20) = 0;
    return 1;
}

u32 __fastcall Batch_Close(u8 *object, void *, float depth) {
    u8 *const batch = at<u8 *>(object, 0x20);
    if (!batch || !at<u32>(batch, 0x20) || !at<u32>(batch, 0x24))
        return 1;
    if (depth != -1.0f && depth == depth)   // (0x701258; equal or unordered: kept)
        at<u32>(batch, 0x30) = *reinterpret_cast<const u32 *>(&depth);
    const u32 stride = at<u32>(object, 0x1c);
    g_env->end(global<u8 *>(0x008bcbc4), nullptr, at<u32>(batch, 0x28) + stride * at<u32>(batch, 0x20), at<u32>(batch, 0x2c) + at<u32>(batch, 0x24) * 2);
    return Batch_Flush(object, nullptr);
}

u32 __fastcall Model_AddTriangles(u8 *model, void *, u8 *batch, const void *matrix, const u8 *vertices) {
    {
        using Slot = void(__cdecl *)(void *, const void *, const u8 *, u32, u32, u32, u32);
        void *const transformer = global<void *>(0x0087b1d8);
        void *const *const vtable = *static_cast<void *const *const *>(transformer);
        const u8 *const settings = at<u8 *>(model, 4);
        reinterpret_cast<Slot>(vtable[2])(transformer, matrix, vertices, at<u32>(settings, 0x60), 0x10, 0, at<u16>(settings, 0x58));
    }
    const u8 *settings = at<u8 *>(model, 4);
    const u8 *entry = at<u8 *>(settings, 0x6c);
    for (u32 g = 0; g < at<u16>(settings, 0x5a); ++g, entry += 0x10) {
        const u32 base = at<u32>(batch, 0x20);
        u8 *dst = at<u8 *>(batch, 0x28) + (base << 5);
        const u8 *src = vertices;
        for (u32 n = at<u16>(settings, 0x58); n; --n, src += 0x10, dst += 0x20) {
            at<float>(dst, 0) = (float)(X87Reg((double)gBatch_8bd4ac) * X87Reg(at<float>(src, 0)));
            at<float>(dst, 4) = (float)(X87Reg((double)gBatch_8bd4ac) * X87Reg(at<float>(src, 8)));
            at<u32>(dst, 8) = at<u32>(src, 4);
            at<u32>(dst, 0xc) = at<u32>(src, 0xc);
            at<u32>(dst, 0x10) = 0xffffffff;
        }
        at<u32>(batch, 0x20) += at<u16>(settings, 0x58);
        u16 *out = at<u16 *>(batch, 0x2c) + at<u32>(batch, 0x24);
        const u32 vbase = at<u16>(entry, 8);
        const u8 *face = at<u8 *>(at<u8 *>(model, 4), 0x70) + at<u16>(entry, 4) * 8;
        for (u32 k = at<u16>(entry, 6); k; --k, face += 8) {
            const u8 *const verts = at<u8 *>(batch, 0x28);
            const u8 *const remap = at<u8 *>(at<u8 *>(model, 4), 0x94);
            const u32 a = at<u16>(remap, (at<u16>(face, 2) - vbase) * 2);
            const u32 b = at<u16>(remap, (at<u16>(face, 4) - vbase) * 2);
            const u32 c = at<u16>(remap, (at<u16>(face, 6) - vbase) * 2);
            const u8 *const pa = verts + (a << 5), *const pb = verts + (b << 5), *const pc = verts + (c << 5);
            gBatch_8c63bc = X87_FloatThrough(at<u32>(pa, 0));
            gBatch_8c63c8 = at<u32>(pa, 4);
            gBatch_8c63b0 = at<u32>(pa, 0xc);
            gBatch_8c63c0 = at<u32>(pb, 0);
            gBatch_8c63cc = at<u32>(pb, 4);
            gBatch_8c63b4 = at<u32>(pb, 0xc);
            gBatch_8c63c4 = at<u32>(pc, 0);
            gBatch_8c63d0 = at<u32>(pc, 4);
            gBatch_8c63b8 = at<u32>(pc, 0xc);
            const float xa = reinterpret_cast<float &>(gBatch_8c63bc), ya = reinterpret_cast<float &>(gBatch_8c63c8), wa = reinterpret_cast<float &>(gBatch_8c63b0);
            const float xb = reinterpret_cast<float &>(gBatch_8c63c0), yb = reinterpret_cast<float &>(gBatch_8c63cc), wb = reinterpret_cast<float &>(gBatch_8c63b4);
            const float xc = reinterpret_cast<float &>(gBatch_8c63c4), yc = reinterpret_cast<float &>(gBatch_8c63d0), wc = reinterpret_cast<float &>(gBatch_8c63b8);
            const X87Reg f = (X87Reg(wa) * yb - X87Reg(wb) * ya) * xc - X87Reg(wc) * yb * xa;
            const X87Reg e = (X87Reg(wa) * yc - X87Reg(wc) * ya) * xb - X87Reg(wb) * yc * xa;
            if (f.v < e.v || f.v != f.v || e.v != e.v) {   // (below or unordered)
                out[0] = (u16)(a + base);
                out[1] = (u16)(b + base);
                out[2] = (u16)(c + base);
                out += 3;
                at<u32>(batch, 0x24) += 3;
            }
        }
        settings = at<u8 *>(model, 4);
    }
    return 1;
}
}

u32 __fastcall Model_AddTree(void *self, void *, u8 *model, const u32 *matrix, u8 *batch, const u8 *vertices) {
    u32 local[16];
    for (u32 i = 0; i < 16; ++i)
        local[i] = matrix[i];
    u8 *&part = *reinterpret_cast<u8 **>(model + 4);
    if (!part)
        return 1;
    do {
        u8 *const transformer = *reinterpret_cast<u8 **>((uintptr_t)0x0087b1d8);
        const u32 slot = (*reinterpret_cast<u32 **>(transformer))[6];
        reinterpret_cast<void(__cdecl *)(u8 *, u8 *, const u32 *, u32 *)>((uintptr_t)slot)(transformer, part + 0x10, matrix, local);
        if (!(*reinterpret_cast<u32 *>(part) & 0x8210) && !g_treeCalls.Th(0x00581a00, (u32)(uintptr_t)model, batch, local, vertices))
            return 0;
        u8 *const held = part;
        u8 *const child = *reinterpret_cast<u8 **>(held + 0xc);
        if (!(*held & 0x20) && child) {
            part = child;
            if (!Model_AddTree(self, nullptr, model, local, batch, vertices))
                return 0;
            part = held;
        }
        part = *reinterpret_cast<u8 **>(part + 8);
    } while (part);
    return 1;
}
