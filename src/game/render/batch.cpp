#include "batch.h"

#include <cstdint>

#include "drawbuffer.h"
#include "../math/x87.h"
#include "vertexfog.h"
#include "texformat.h"
#include "../gfx/texset.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

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
            at<float>(dst, 0) = (float)(X87Reg((double)global<s32>(0x008bd4ac)) * X87Reg(at<float>(src, 0)));
            at<float>(dst, 4) = (float)(X87Reg((double)global<s32>(0x008bd4ac)) * X87Reg(at<float>(src, 8)));
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
            global<u32>(0x008c63bc) = X87_FloatThrough(at<u32>(pa, 0));
            global<u32>(0x008c63c8) = at<u32>(pa, 4);
            global<u32>(0x008c63b0) = at<u32>(pa, 0xc);
            global<u32>(0x008c63c0) = at<u32>(pb, 0);
            global<u32>(0x008c63cc) = at<u32>(pb, 4);
            global<u32>(0x008c63b4) = at<u32>(pb, 0xc);
            global<u32>(0x008c63c4) = at<u32>(pc, 0);
            global<u32>(0x008c63d0) = at<u32>(pc, 4);
            global<u32>(0x008c63b8) = at<u32>(pc, 0xc);
            const float xa = global<float>(0x008c63bc), ya = global<float>(0x008c63c8), wa = global<float>(0x008c63b0);
            const float xb = global<float>(0x008c63c0), yb = global<float>(0x008c63cc), wb = global<float>(0x008c63b4);
            const float xc = global<float>(0x008c63c4), yc = global<float>(0x008c63d0), wc = global<float>(0x008c63b8);
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
