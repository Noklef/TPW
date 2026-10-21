#include "meshlist.h"

#include <cstdint>
#include <cstring>

#include "../math/mat4.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

float of(u32 b) {
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

// The colour (+4-+0x10) of a light times the factor added into 0x879db0-0x879dbc.
void AddLight(const u8 *light, X87Reg factor) {
    for (u32 k = 0; k < 4; ++k)
        global<float>(0x00879db0 + 4 * k) = (float)(X87Reg(at<float>(light, 4 + 4 * k)) * factor + global<float>(0x00879db0 + 4 * k));
}

}  // namespace

extern "C" {

u32 __fastcall Meshes_Ready(u8 *object, void *, const u32 *list) {
    for (; *list; list++) {
        u8 *mesh = reinterpret_cast<u8 *>(*list);
        if (at<u32>(mesh, 0x50) & 0x20)
            at<u32>(object, 0x30) = (u32)(uintptr_t)mesh + 0x38;
        if (!(at<u32>(mesh, 0x50) & 0x10000000)) {
            at<u32>(mesh, 0x58) = *reinterpret_cast<u32 *>(0x00879dec);
            *reinterpret_cast<u32 *>(0x00879dec) = (u32)(uintptr_t)mesh;
        }
    }
    return 1;
}

void __fastcall Meshes_SetDirection(u8 *object, void *, u32 x, u32 y, u32 z) {
    const float fx = *reinterpret_cast<const float *>(&x), fy = *reinterpret_cast<const float *>(&y);
    const float fz = *reinterpret_cast<const float *>(&z);
    const X87Reg len = X87_Sqrt((X87Reg(fx) * fx + X87Reg(fy) * fy) + X87Reg(fz) * fz);
    if (!(len.v == 0.0f || len.v != len.v)) {  // 0x7016dc
        const X87Reg scale = X87_DivR(len, 1.0f);  // 0x7016c0
        at<float>(object, 0x34) = (float)(scale * fx);
        at<float>(object, 0x38) = (float)(scale * fy);
        at<float>(object, 0x3c) = (float)(scale * fz);
        return;
    }
    at<u32>(object, 0x34) = x;
    at<u32>(object, 0x38) = y;
    at<u32>(object, 0x3c) = z;
}

u32 __fastcall Meshes_SetView(u8 *object, void *, const u32 *matrix, u32) {
    u8 *mesh = *reinterpret_cast<u8 **>((uintptr_t)0x00879dec);
    *reinterpret_cast<const u32 **>((uintptr_t)0x00879dcc) = matrix;
    u32 *const view = reinterpret_cast<u32 *>((uintptr_t)0x00879df0);
    for (int i = 0; i < 16; ++i) view[i] = matrix[i];
    Mat4_Invert(view);
    const float *const m = reinterpret_cast<const float *>(view);
    for (; mesh; mesh = *reinterpret_cast<u8 **>(mesh + 0x58)) {
        if (mesh[0x50] & 2) {
            float *const d = reinterpret_cast<float *>(mesh + 0x38);
            Mat4_Transform(m, nullptr, d, reinterpret_cast<const float *>(mesh + 0x2c), 3);
            const X87Reg x = d[0], y = d[1], z = d[2];
            const float len = static_cast<float>((y * y + z * z) + x * x);
            if (!(len != len || len == 0.0f || len == 1.0f)) {
                const X87Reg inv = X87Reg(1.0f) / X87_Sqrt(X87Reg(len));
                d[0] = static_cast<float>(X87Reg(d[0]) * inv);
                d[1] = static_cast<float>(inv * X87Reg(d[1]));
                d[2] = static_cast<float>(inv * X87Reg(d[2]));
            }
        }
        if (mesh[0x50] & 4)
            Mat4_Transform(m, nullptr, reinterpret_cast<float *>(mesh + 0x20), reinterpret_cast<const float *>(mesh + 0x14), 2);
    }
    Mat4_Transform(m, nullptr, reinterpret_cast<float *>(object + 0x34), reinterpret_cast<const float *>(object + 0x34), 3);
    return 1;
}

void __fastcall Meshes_LightAt(u32 seed, void *, const float *normal, float x, float y, float z) {
    for (u32 k = 0; k < 0x10; k += 4)
        global<u32>(0x00879db0 + k) = global<u32>(0x00879dd8 + k);
    u32 spill = seed;   // (the stack slot of the ECX pushed for room: d below)
    for (const u8 *light = global<u8 *>(0x00879dec); light; light = at<u8 *>(light, 0x58)) {
        const u32 kind = at<u32>(light, 0x50) & 0xf;
        if (kind == 2) {
            const X87Reg a = X87Reg(at<float>(light, 0x38)) * normal[0];
            const X87Reg b = X87Reg(at<float>(light, 0x3c)) * normal[1];
            const X87Reg c = X87Reg(normal[2]) * at<float>(light, 0x40);
            const float f = (float)((X87Reg(-a.v) - b) - c);
            if (!(bits(f) & 0x80000000))
                AddLight(light, f);
        } else if (kind == 4) {
            const X87Reg dx = X87Reg(at<float>(light, 0x20)) - x;
            const X87Reg dy = X87Reg(at<float>(light, 0x24)) - y;
            const X87Reg dz = X87Reg(at<float>(light, 0x28)) - z;
            const X87Reg dxx = dx * dx;
            const float fy = (float)dy;
            const X87Reg dyy = dy * fy;
            spill = bits((float)dxx);
            const float fz = (float)dz;
            const X87Reg sum = dyy + of(spill);
            const X87Reg dzz = X87Reg(fz) * fz;
            spill = bits((float)sum);
            spill = bits((float)(dzz + of(spill)));
            if ((s32)spill < at<s32>(light, 0x48))
                AddLight(light, (X87Reg(at<float>(light, 0x48)) - of(spill)) * at<float>(light, 0x4c));
        } else if (kind == 6) {
            const X87Reg dy = X87Reg(at<float>(light, 0x24)) - y;
            const X87Reg dz = X87Reg(at<float>(light, 0x28)) - z;
            const X87Reg dx = X87Reg(at<float>(light, 0x20)) - x;
            const float f = (float)((dy * normal[1] + dz * normal[2]) + dx * normal[0]);
            if (!(bits(f) & 0x80000000))
                AddLight(light, (X87Reg(at<float>(light, 0x48)) - of(spill)) * at<float>(light, 0x4c) * f);
        }
    }
}
}
