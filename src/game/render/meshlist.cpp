#include "meshlist.h"

#include <cstdint>

#include "../math/mat4.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}
