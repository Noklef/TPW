// A set of points drawn as triangles facing the camera: see pointsprites.h.
#include "pointsprites.h"

#include <cstdint>
#include <cstring>

#include "../math/mat4.h"
#include "../math/x87.h"

namespace {

AddrCalls g_calls;

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

const u32 kWhite = 0xffffffff;

// The static result of the matrix and the view.
const float *Combined(const float *matrix, const float *view) {
    float *const result = reinterpret_cast<float *>((uintptr_t)&gMat4_fb3400);
    u8 &guard = gPointsprites_fb33e0;
    if (!(guard & 1)) {
        memset(result, 0, 0x40);
        guard |= 1;
        result[15] = result[10] = result[5] = result[0] = 1.0f;
        g_calls.Cd(0x0067a7d0, 0x005d1750u);   // CRT_atexit(Stub_Ret)
    }
    const u8 *const transformer = global<const u8 *>(0x0087b1d8);
    reinterpret_cast<void(__cdecl *)(const void *, const float *, const float *, float *)>((uintptr_t)at<u32>(at<const void *>(transformer, 0), 0x18))(transformer, matrix, view, result);
    return result;
}

}  // namespace

void PointSprites_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall PointSprites_Draw(const u8 *self) {
    const u8 *const texture = at<const u8 *>(self, 8);
    const u32 key = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
    const float size = at<float>(self, 0x10);
    const float halfWidth = (float)(X87Reg(global<float>(0x008bcbcc)) * size);
    const float *const view = global<const float *>(0x0087b088);
    const float *const matrix = at<const float *>(self, 0x18);
    float combined[16];
    memcpy(combined, matrix ? Combined(matrix, view) : view, sizeof(combined));
    const u32 format = g_calls.Th(0x00590c40, (u32)(uintptr_t)texture);   // Texture_FormatBits
    const float u0 = X87_Through(at<float>(texture, 0x28)), v0 = X87_Through(at<float>(texture, 0x2c));
    const float u1 = (float)(X87Reg(at<float>(texture, 0x20)) + u0), v1 = (float)(X87Reg(at<float>(texture, 0x24)) + v0);
    u32 *const batch = reinterpret_cast<u32 *>((uintptr_t)g_calls.Th(0x00565240, global<u32>(0x00879ab0), 2u, 4u, 0x1c4u, format, key));   // Batch_Begin
    if (!batch) return 1;
    *batch = 0x20;
    const u8 *const positions = at<const u8 *>(self, 0xc);
    for (s32 i = 0; i < at<s32>(self, 0); ++i) {
        float point[4];
        Mat4_Transform(combined, nullptr, point, reinterpret_cast<const float *>(positions + i * 0xc), 0);
        if (!((double)point[2] > 0.0)) continue;
        const float z = X87_Through(point[2]), w = X87_Through(point[3]);
        const float bottom = (float)(X87Reg(point[1]) - size);
        float vertices[24];
        memcpy(&vertices[0], &point[0], 4);
        vertices[1] = (float)(X87Reg(size) + point[1]);
        memcpy(&vertices[2], &point[2], 8);
        vertices[6] = (float)((X87Reg(u0) + u1) * 0.5f);
        vertices[7] = v0;
        vertices[8] = (float)(X87Reg(halfWidth) + point[0]);
        vertices[9] = bottom;
        vertices[10] = z;
        vertices[11] = w;
        vertices[14] = u1;
        vertices[15] = v1;
        vertices[16] = (float)(X87Reg(point[0]) - halfWidth);
        vertices[17] = bottom;
        vertices[18] = z;
        vertices[19] = w;
        vertices[22] = u0;
        vertices[23] = v1;
        for (u32 k = 4; k < 24; k += 8) memcpy(&vertices[k], &kWhite, 4);
        if (!g_calls.Th(0x00565bb0, global<u32>(0x00879ab0), vertices, 3u, 0x20u, 0u, 0u)) return 0;   // DrawBatch_AddPolygon
    }
    return g_calls.Th(0x005658c0, global<u32>(0x00879ab0), 0u);   // Batch_Close
}
}
