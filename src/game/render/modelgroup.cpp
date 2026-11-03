#include "modelgroup.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "../gfx/texset.h"
#include "../math/geometry.h"
#include "../math/x87.h"
#include "batch.h"
#include "clipcodes.h"
#include "clippoly.h"
#include "fixedverts.h"
#include "texcoords.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ModelGroupEnv kPortEnv = {&Model_TexCoords, &FixedVerts_Modulate, &Batch_FogVertices, &ClipPoly_TriangleFan};
const ModelGroupEnv *g_env = &kPortEnv;

u32 TextureId(const u8 *texture) { return at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0; }

const float &MinusOne() { return global<float>(0x00701d84); }
const float &Half() { return global<float>(0x00701d88); }

void Zeros(u8 *uv, u32 count, s32 stride) {
    for (; count; --count, uv += stride) {
        at<u32>(uv, 0) = 0;
        at<u32>(uv, 4) = 0;
    }
}

// The UVs from the settings' block of 32 bytes for each four vertices: u[4], then v[4].
void Plain(u8 *uv, const u8 *settings, const u8 *texture, u32 first, u32 count, s32 stride) {
    const float *const block = at<const float *>(settings, 0x68);
    for (u32 k = first; count; --count, ++k, uv += stride) {
        const float *const pair = block + (k >> 2) * 8 + (k & 3);
        at<float>(uv, 0) = (float)(X87Reg(pair[0]) * at<float>(texture, 0x20) + at<float>(texture, 0x28));
        at<float>(uv, 4) = (float)(X87Reg(pair[4]) * at<float>(texture, 0x24) + at<float>(texture, 0x2c));
    }
}

X87Reg SphereU(X87Reg dot, const u8 *texture) { return (dot - MinusOne()) * Half() * at<float>(texture, 0x20) + at<float>(texture, 0x28); }
X87Reg SphereV(X87Reg dot, const u8 *texture) { return (X87Reg(Half()) - dot * Half()) * at<float>(texture, 0x24) + at<float>(texture, 0x2c); }

double Dot(const float *row, const float *normal) { return Vec3_DotByValue(row[0], row[1], row[2], normal[0], normal[1], normal[2]); }

}  // namespace

extern "C" {

void ModelGroup_SetEnv(const ModelGroupEnv *env) { g_env = env ? env : &kPortEnv; }

u32 __fastcall ModelGroup_AddToBatch(u8 *model, void *, u8 *batch, u32 group, s32 stride) {
    const u8 *const settings = at<const u8 *>(model, 4);
    const u8 *const renderer = global<const u8 *>(0x00879ab0);
    const u8 *const open = at<const u8 *>(renderer, 0x20);
    if (at<u32>(renderer, 0x28) < (at<u16>(settings, 0x5e) + at<u32>(open, 0x20) + at<u32>(batch, 0x20)) * at<u32>(renderer, 0x1c) + at<u32>(open, 0x28) ||
        at<u32>(renderer, 0x2c) < at<u32>(open, 0x2c) + 2 * (at<u16>(settings, 0x5c) * 3u + at<u32>(open, 0x24) + at<u32>(batch, 0x24)))
        return 1;

    u8 *const base = reinterpret_cast<u8 *>((uintptr_t)((u32)stride * at<u32>(batch, 0x20) + at<u32>(batch, 0x28)));
    const u8 *const entry = at<const u8 *>(settings, 0x6c) + group * 16;
    const u8 *const material = at<const u8 *>(entry, 0);
    const u8 *const set = material ? at<const u8 *>(material, 4) : nullptr;
    const u32 number = material && (global<u8>(0x008bd550) & 0x40) ? at<u16>(material, 2) : 0;
    const u8 *const texture = set ? reinterpret_cast<const u8 *>((uintptr_t)TexSet_Texture(set, nullptr, (s32)number)) : nullptr;
    const u32 first = at<u16>(entry, 8), count = at<u16>(entry, 0xa) - first + 1;
    const u16 *const table = at<const u16 *>(settings, 0x94);

    u8 *uv = base + 0x18;
    if (!texture || !TextureId(texture)) {
        Zeros(uv, count, stride);
    } else if (at<u32>(texture, 8) & 0x8000000) {
        const float *const view = global<const float *>(0x0087afe8);
        float across[3], up[3];
        std::memcpy(across, view, 12);
        std::memcpy(up, view + 4, 12);
        const float *const normals = at<const float *>(settings, 0x64);
        for (u32 k = 0; k < count; ++k, uv += stride) {
            const float *const n = normals + table[first + k] * 3;
            const X87Reg dot = X87Reg(across[0]) * n[0] + X87Reg(across[1]) * n[1] + X87Reg(across[2]) * n[2];
            at<float>(uv, 0) = (float)SphereU(dot, texture);
            at<float>(uv, 4) = (float)SphereV(X87Reg(Dot(up, n)), texture);
        }
    } else {
        Plain(uv, settings, texture, first, count, stride);
    }

    if (at<u32>(settings, 0) & 0x2000000) {
        const u8 *const layer = at<const u8 *>(settings, 0x98);
        const u8 *const layerSet = layer ? at<const u8 *>(layer, 4) : nullptr;
        const u8 *const second = layerSet ? reinterpret_cast<const u8 *>((uintptr_t)TexSet_Texture(layerSet, nullptr, 0)) : nullptr;
        const u32 id = TextureId(second);  // (read with no check, sic: no texture faults)
        uv = base + 0x20;
        if (!id) {
            Zeros(uv, count, stride);
        } else if (at<u32>(second, 8) & 0x8000000) {
            const float *const view = global<const float *>(0x0087afe8);
            float across[3], up[3];
            std::memcpy(across, view, 12);
            std::memcpy(up, view + 4, 12);
            const float *const normals = at<const float *>(settings, 0x64);
            for (u32 k = 0; k < count; ++k, uv += stride) {
                const float *const n = normals + table[first + k] * 3;
                at<float>(uv, 0) = (float)SphereU(X87Reg(Dot(across, n)), second);
                at<float>(uv, 4) = (float)SphereV(X87Reg(Dot(up, n)), second);
            }
        } else {
            Plain(uv, settings, second, first, count, stride);
        }
        at<u32>(batch, 0xc) = id;
    } else if (texture && (at<u32>(texture, 8) & 0x400) && !at<u32>(texture, 0x3c)) {
        g_env->texCoords(base + 0x20, settings, (s32)group, texture, stride, 2);
        at<u32>(batch, 0xc) = at<u32>(texture, 0x68) & 0x1000 ? at<u32>(texture, 0x7c) : 0;
        if (at<u32>(texture, 0x68) & 0x800000)
            at<u32>(batch, 0x10) = at<u32>(texture, 0xc8) & 0x1000 ? at<u32>(texture, 0xdc) : 0;
    }

    const u8 *const verts = at<const u8 *>(model, 0xc);
    const u32 *const colours = at<const u32 *>(model, 0x10);
    const u32 mask = at<u32>(global<const u8 *>(0x0087a248), 0x2c);
    const bool coloured = !(at<u8>(entry, 0xc) & 1) && colours;
    u8 *out = base;
    for (u32 k = 0; k < count; ++k, out += stride) {
        const u8 *const from = verts + table[first + k] * 16u;
        at<u32>(out, 0) = at<u32>(from, 0);
        at<u32>(out, 4) = at<u32>(from, 4);
        at<u32>(out, 8) = at<u32>(from, 8);
        at<u32>(out, 0xc) = at<u32>(from, 0xc);
        at<u32>(out, 0x10) = coloured ? colours[first + k] & mask : mask;
    }

    if (texture && !TextureId(texture)) {
        const u32 colour = at<u32>(texture, 8) & 0x10000000 ? vcall<u32>(at<void *>(texture, 0x40), 0xc) : 0xffffffff;
        g_env->modulate(base, colour, count, stride);
    }

    u16 *indices = reinterpret_cast<u16 *>((uintptr_t)(at<u32>(batch, 0x2c) + at<u32>(batch, 0x24) * 2));
    const u8 *face = at<const u8 *>(settings, 0x70) + at<u16>(entry, 4) * 8u;
    const u32 faces = at<u16>(entry, 6);
    const u32 old = at<u32>(batch, 0x20);
    at<u32>(batch, 0x20) = old + count;
    const auto vertex = [batch, stride](u32 index) { return reinterpret_cast<const float *>((uintptr_t)(index * (u32)stride + at<u32>(batch, 0x28))); };

    if (at<u32>(model, 8) & 0x3f) {
        const u32 vertexLimit = at<u32>(renderer, 0x28) - (u32)stride * 10;
        const u32 indexLimit = at<u32>(renderer, 0x2c) - 0x30;
        u32 result;
        u8 codes[0x1ff0];
        ClipCodes_Compute(global<u32>(0x00879a20), 0, reinterpret_cast<u8 *>(&result), codes, base, count, (u32)stride, at<u32>(model, 8));
        if ((u8)result) {
            at<u32>(batch, 0x20) = old;
            return 1;
        }
        for (u32 n = faces; n; --n, face += 8) {
            const s32 a = (s32)at<u16>(face, 2) - (s32)first, b = (s32)at<u16>(face, 4) - (s32)first, c = (s32)at<u16>(face, 6) - (s32)first;
            if (!(codes[a] & codes[b] & codes[c])) {
                const u32 ia = (u32)a + old, ib = (u32)b + old, ic = (u32)c + old;
                if (Tri_WindingTest(vertex(ia), vertex(ib), vertex(ic))) {
                    *indices++ = (u16)ia;
                    *indices++ = (u16)ib;
                    *indices++ = (u16)ic;
                    const u32 any = (u32)codes[a] | codes[c] | codes[b];
                    if (any)
                        indices = g_env->fan((s32)any, indices, at<u8 *>(batch, 0x28), &at<u32>(batch, 0x20), stride);
                }
            }
            if (vertexLimit < at<u32>(batch, 0x20) * (u32)stride + at<u32>(batch, 0x28) || indexLimit < (u32)(uintptr_t)indices)
                break;
        }
        if ((u32)(uintptr_t)indices == at<u32>(batch, 0x2c)) {
            at<u32>(batch, 0x20) = old;
            return 1;
        }
    } else {
        for (u32 n = faces; n; --n, face += 8) {
            const u32 a = at<u16>(face, 2) - first + old, b = at<u16>(face, 4) - first + old, c = at<u16>(face, 6) - first + old;
            if (Tri_WindingTest(vertex(a), vertex(b), vertex(c))) {
                *indices++ = (u16)a;
                *indices++ = (u16)b;
                *indices++ = (u16)c;
            }
        }
    }
    at<s32>(batch, 0x24) = (s32)((u32)(uintptr_t)indices - at<u32>(batch, 0x2c)) >> 1;

    g_env->fog(global<u8 *>(0x00879ab0), nullptr, base, at<u32>(batch, 0x20) - old);
    const u32 flag = at<u32>(model, 8) >> 0x18 & 2;
    using Slot = float(__cdecl *)(void *, u8 *, s32, u32, u32);
    void *const transformer = global<void *>(0x0087b1d8);
    void *const *const vtable = *static_cast<void *const *const *>(transformer);
    const float answer = reinterpret_cast<Slot>(vtable[3])(transformer, base, stride, at<u32>(batch, 0x20) - old, flag);
    at<float>(batch, 0x30) = (float)(X87Reg(answer) + at<float>(batch, 0x30));
    if (at<u32>(batch, 4) & 0x800000) {
        if (flag == 2)
            at<u32>(batch, 0x30) = X87_FloatThrough(global<u32>(0x00701d8c));
        else
            at<float>(batch, 0x30) = (float)(X87Reg((double)(s32)(at<u32>(batch, 0x20) - old)) * global<float>(0x008bcc14) - global<float>(0x00701d90));
    }
    if (texture && (at<u32>(texture, 8) & 0x4000000))
        at<u32>(batch, 0) |= 0x400;
    return 1;
}
}  // extern "C"
