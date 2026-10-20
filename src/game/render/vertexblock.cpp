#include "vertexblock.h"

#include <cstdint>

#include <cstring>

#include "../core/globals.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

// the sine table's entry for (the float at 0x874fc0 + phase) * its scale (stored as a float, then fistp)
float ripple(float phase) {
    const SinTable sines = SinTable_Game();
    return SinTable_At(sines, (float)((X87Reg(*reinterpret_cast<const float *>(0x00874fc0)) + phase) * sines.scale));
}

}  // namespace

extern "C" {

void __fastcall VertexBlock_Add(u8 *block, void *, float x, float y, float z, float u, float v, u32 wave, const float *normal) {
    if (wave) {
        y = (float)(X87Reg(Terrain_HeightAt(x, z)) + y);
        const float phase = (float)(X87Reg(z) + x);
        y = (float)(X87Reg(ripple(phase)) + y);
        const float lift = ripple(phase);  // (looked up again)
        float *const n = at<float *>(block, 0x68) + at<u32>(block, 0x9c) * 3;
        n[0] = 0.0f;
        n[1] = (float)((X87Reg(lift) - -1.0f) * 0.5f);  // 0x700e28, 0x700de4
        n[2] = 0.0f;
    } else if (normal) {
        const float nz = X87_Through(normal[2]), ny = X87_Through(normal[1]), nx = X87_Through(normal[0]);
        float *const n = at<float *>(block, 0x68) + at<u32>(block, 0x9c) * 3;
        n[0] = nx;
        n[1] = ny;
        n[2] = nz;
    } else {
        float *const n = at<float *>(block, 0x68) + at<u32>(block, 0x9c) * 3;
        n[0] = 0.0f;
        n[1] = 1.0f;
        n[2] = 0.0f;
    }
    u32 count = at<u32>(block, 0x9c);
    float *const p = at<float *>(block, 0x64) + (count >> 2) * 12 + (count & 3);
    p[0] = X87_Through(x);
    p[4] = X87_Through(y);
    p[8] = X87_Through(z);
    count = at<u32>(block, 0x9c);
    float *const t = at<float *>(block, 0x6c) + (count >> 2) * 8 + (count & 3);
    t[0] = X87_Through(u);
    t[4] = X87_Through(v);
    at<u32>(block, 0x9c) = at<u32>(block, 0x9c) + 1;
}

u32 __fastcall VertexBlock_AddQuad(u8 *block, void *, const u32 *positions, u32, const u32 *uvs, const u8 *normals) {
    // the floats go over as their bits, as the original pushes them (no x87 load to quieten a signalling NaN)
    using AddBits = void(__fastcall *)(u8 *, void *, u32, u32, u32, u32, u32, u32, const u8 *);
    const AddBits add = reinterpret_cast<AddBits>(&VertexBlock_Add);
    const u32 base = at<u32>(block, 0x9c);
    for (u32 i = 0; i < 4; i++)
        add(block, nullptr, positions[3 * i], positions[3 * i + 1], positions[3 * i + 2], uvs[3 * i], uvs[3 * i + 1], 0, normals + 0xc * i);
    u8 *list = at<u8 *>(block, 0x94);
    at<u16>(list, 2) = (u16)(base + 2);
    at<u16>(at<u8 *>(block, 0x94), 4) = (u16)(base + 1);
    at<u16>(at<u8 *>(block, 0x94), 6) = (u16)base;
    list = at<u8 *>(block, 0x94) + 8;
    u32 triangles = at<u32>(block, 0x98) + 1;
    at<u8 *>(block, 0x94) = list;
    at<u32>(block, 0x98) = triangles;
    at<u16>(list, 2) = (u16)(base + 1);
    at<u16>(at<u8 *>(block, 0x94), 4) = (u16)(base + 2);
    at<u16>(at<u8 *>(block, 0x94), 6) = (u16)(base + 3);
    list = at<u8 *>(block, 0x94) + 8;
    triangles = at<u32>(block, 0x98) + 1;
    at<u8 *>(block, 0x94) = list;
    at<u32>(block, 0x98) = triangles;
    return triangles;
}

u32 __fastcall VertexBlock_AddFace(u8 *block, void *, u32 x, u32 y, u32 z, u32 face, u32 turn, s32 type, u32, u32 big, u32, u32 fixed) {
    using AddBits = void(__fastcall *)(u8 *, void *, u32, u32, u32, u32, u32, u32, const u8 *);
    const AddBits add = reinterpret_cast<AddBits>(&VertexBlock_Add);
    const auto value = [](u32 bits) {
        float f;
        std::memcpy(&f, &bits, 4);
        return f;
    };
    const auto bits = [](float f) {
        u32 b;
        std::memcpy(&b, &f, 4);
        return b;
    };
    const float step = big ? 20.0f : 10.0f;
    const u32 base = at<u32>(block, 0x9c);
    if (!fixed && !*reinterpret_cast<const u32 *>(0x00874fc4) && type < 13) {
        if (Global81ae2c_Get_52f840() == 4 || Global81ae2c_Get_52f840() == 0xe || Global81ae2c_Get_52f840() == 0x13) {
            const X87Reg across = X87Reg((double)*reinterpret_cast<const s32 *>(0x008186b8)) * X87Reg(10.0f);
            const X87Reg down = X87Reg((double)*reinterpret_cast<const s32 *>(0x008186b4)) * X87Reg(10.0f);
            x = bits((float)(X87_SubR(across, X87Reg(value(x))) + X87Reg(*reinterpret_cast<const float *>(0x00820afc))));
            z = bits((float)(X87_SubR(down, X87Reg(value(z))) + X87Reg(*reinterpret_cast<const float *>(0x00820af8))));
        }
    }
    const u32 wave = type != -1 && type < 15 ? 1 : 0;
    const u32 first = type < 15 && type >= 8 ? 5 : 0;
    u32 corner[4];
    switch ((u8)turn) {
    case 4: corner[0] = first + 2; corner[1] = first; corner[2] = first + 3; corner[3] = first + 1; break;
    case 0x10: corner[0] = first + 3; corner[1] = first + 2; corner[2] = first + 1; corner[3] = first; break;
    case 0x40: corner[0] = first + 1; corner[1] = first + 3; corner[2] = first; corner[3] = first + 2; break;
    default: corner[0] = first; corner[1] = first + 1; corner[2] = first + 2; corner[3] = first + 3; break;
    }
    const auto vertex = [&](u32 px, u32 py, u32 pz, u32 k) {
        const u32 *const uv = reinterpret_cast<const u32 *>(0x00871ee8 + 0xc * corner[k]);
        add(block, nullptr, px, py, pz, uv[0], uv[1], wave, nullptr);
    };
    const auto past = [&](u32 p) { return bits((float)(X87Reg(step) + X87Reg(value(p)))); };   // fld step; fadd p
    const auto below = [&]() { return bits((float)(X87Reg(value(y)) - X87Reg(-10.0f))); };     // fld y; fsub -10
    switch ((u8)face) {
    case 0: {
        vertex(x, y, z, 0);
        const u32 xs = past(x);
        vertex(xs, y, z, 1);
        const u32 zs = past(z);
        vertex(x, y, zs, 2);
        vertex(xs, y, zs, 3);
        break;
    }
    case 1: {
        vertex(x, y, z, 0);
        const u32 xs = past(x);
        vertex(xs, y, z, 1);
        const u32 yk = below();
        vertex(x, yk, z, 2);
        vertex(xs, yk, z, 3);
        break;
    }
    case 4: {
        const u32 xs = past(x);
        vertex(xs, y, z, 0);
        const u32 zs = past(z);
        vertex(xs, y, zs, 1);
        const u32 yk = below();
        vertex(xs, yk, z, 2);
        vertex(xs, yk, zs, 3);
        break;
    }
    case 0x10: {
        const u32 zs = past(z);
        const u32 xs = past(x);
        vertex(xs, y, zs, 0);
        vertex(x, y, zs, 1);
        const u32 yk = below();
        vertex(xs, yk, zs, 2);
        vertex(x, yk, zs, 3);
        break;
    }
    case 0x40: {
        const u32 zs = past(z);
        vertex(x, y, zs, 0);
        vertex(x, y, z, 1);
        const u32 yk = below();
        vertex(x, yk, zs, 2);
        vertex(x, yk, z, 3);
        break;
    }
    default:
        break;
    }
    u8 *list = at<u8 *>(block, 0x94);
    at<u16>(list, 2) = (u16)(base + 1);
    at<u16>(at<u8 *>(block, 0x94), 4) = (u16)base;
    at<u16>(at<u8 *>(block, 0x94), 6) = (u16)(base + 3);
    list = at<u8 *>(block, 0x94) + 8;
    u32 triangles = at<u32>(block, 0x98) + 1;
    at<u8 *>(block, 0x94) = list;
    at<u32>(block, 0x98) = triangles;
    at<u16>(list, 2) = (u16)base;
    at<u16>(at<u8 *>(block, 0x94), 4) = (u16)(base + 2);
    at<u16>(at<u8 *>(block, 0x94), 6) = (u16)(base + 3);
    list = at<u8 *>(block, 0x94) + 8;
    triangles = at<u32>(block, 0x98) + 1;
    at<u8 *>(block, 0x94) = list;
    at<u32>(block, 0x98) = triangles;
    return triangles;
}
}
