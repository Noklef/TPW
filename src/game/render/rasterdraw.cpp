#include "rasterdraw.h"

#include <cstdint>
#include <cstring>
#include <mmintrin.h>

#include "../math/x87.h"
#include "fixedverts.h"
#include "rasteriser.h"
#include "rasterformat.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kSpanRoutine = (u32)&gRasteriser_fbe044, kColour = (u32)&gRasterformat_fbe008;

// __ftol's low half.
s32 Ftol(X87Reg v) { return (s32)(u32)(long long)v.v; }
u32 Bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

// The object of the virtual base (the device's table at self - 8), and a slot of an object's.
u32 VirtualBase(const u8 *self) { return (u32)(uintptr_t)(self - 8) + at<u32>(at<const void *>(self - 8, 0), 4); }
template <typename... A>
u32 Slot(u32 object, u32 offset, A... args) {
    return reinterpret_cast<u32(__fastcall *)(u32, u32, A...)>((uintptr_t)at<u32>((void *)(uintptr_t)at<u32>((void *)(uintptr_t)object, 0), offset))(object, 0, args...);
}

// The triangles of a primitive (the division by 3 as the original's multiply), each by a routine.
void Triangles(const u8 *primitive, u32 routine) {
    const u16 *index = at<const u16 *>(primitive, 0x2c);
    for (s32 n = at<s32>(primitive, 0x24) / 3; n != 0; --n, index += 3) {
        const u32 vertices = at<u32>(primitive, 0x28);
        g_calls.Cd(routine, vertices + index[0] * 0x18u, vertices + index[1] * 0x18u, vertices + index[2] * 0x18u);
    }
}

// An index list's triangles, (count + 2) / 3 of them, each by a routine. Returns 0. (sic: a count past 0xfffffffd wraps to none and the loop runs on.)
u32 IndexedTriangles(const u8 *vertices, const u16 *indices, u32 count, u32 routine) {
    if (!count) return 0;
    u32 n = (count + 2) / 3;
    do {
        g_calls.Cd(routine, (u32)(uintptr_t)(vertices + indices[0] * 0x18u), (u32)(uintptr_t)(vertices + indices[1] * 0x18u), (u32)(uintptr_t)(vertices + indices[2] * 0x18u));
        indices += 3;
    } while (--n);
    return 0;
}

}  // namespace

void RasterDraw_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall RasterDraw_Primitive(u8 *self, void *, const u8 *primitive) {
    if (at<u32>(primitive, 4) & 0x4000) return 1;
    Slot(VirtualBase(self), 8, at<u32>(primitive, 4));
    Slot(VirtualBase(self), 0xc, 0u, (global<u8>(0x008bd550) & 1) ? 0u : at<u32>(primitive, 8));
    global<u32>(kColour) = at<u32>(primitive, 0x34);
    const u32 flags = at<u32>(primitive, 4);
    Triangles(primitive, (flags & 0x10000000) && !(flags & 0x100) ? 0x006f02e0u : 0x006ef7f0u);
    if ((at<u32>(primitive, 4) & 0x1000) && !(global<u32>(0x008bd508) & 0x4000000)) _mm_empty();
    return 1;
}

u32 __stdcall RasterDraw_Outlined(const u8 *primitive) {
    global<u32>(kSpanRoutine) = global<u32>(0x006f19cc);
    Triangles(primitive, 0x006ef7f0u);
    g_calls.Cd(0x006792f0, at<u32>(primitive, 0x18), 3u, at<u32>(primitive, 0x28), at<u32>(primitive, 0x20), at<u32>(primitive, 0x2c), at<u32>(primitive, 0x24), 0u);
    _mm_empty();
    return 1;
}

u32 __cdecl RasterDraw_TestQuad() {
    u32 vertices[24];
    const u32 corners[4][4] = {{0xa0, 0xa0, 0x4000, 0xfffefefe}, {0x2760, 0x10e0, 0x4000, 0xff0000fe}, {0x1ea0, 0x1d60, 0x4000, 0xff00fe00}, {0x1e0, 0x1900, 0x4000, 0xfffe0000}};
    for (u32 i = 0; i < 4; ++i) std::memcpy(vertices + i * 6, corners[i], 0x10);
    u16 indices[6] = {0, 1, 2, 0, 2, 3};
    global<u32>(kSpanRoutine) = global<u32>(0x006f1a10);
    const u32 answer = g_calls.Cd(0x006792f0, 4u, 3u, (u32)(uintptr_t)vertices, 4u, (u32)(uintptr_t)indices, 6u, 0u);
    _mm_empty();
    return answer;
}

u32 __fastcall RasterDraw_Rect(u8 *self, void *, float x0, float y0, float x1, float y1, u32 colour, const u8 *texture) {
    const float scale = global<float>(0x007013b0);
    const s32 left = Ftol(X87Reg(x0) * scale), top = Ftol(X87Reg(y0) * scale), right = Ftol(X87Reg(x1) * scale), bottom = Ftol(X87Reg(y1) * scale);
    u32 v[24];
    u16 indices[6] = {0, 1, 2, 0, 2, 3};
    const s32 corners[4][2] = {{left, top}, {right, top}, {right, bottom}, {left, bottom}};
    for (u32 i = 0; i < 4; ++i) {
        v[i * 6] = (u32)corners[i][0];
        v[i * 6 + 1] = (u32)corners[i][1];
        v[i * 6 + 2] = 1000;
        v[i * 6 + 3] = colour;
    }
    if (!texture) {
        global<u32>(kSpanRoutine) = global<u32>(0x006f19cc);
    } else {
        global<u32>(kSpanRoutine) = global<u32>(0x006f19b0);
        const u32 data = Slot((u32)(uintptr_t)texture, 8);
        Slot(VirtualBase(self), 0xc, 0u, data);
        const u32 width = at<u32>(texture, 8) << 4, height = at<u32>(texture, 0xc) << 4;
        v[4] = v[5] = 0;
        v[10] = width;
        v[11] = 0;
        v[16] = width;
        v[17] = height;
        v[22] = 0;
        v[23] = height;
    }
    g_calls.Cd(0x00679350, 4u, 3u, (u32)(uintptr_t)v, 4u, (u32)(uintptr_t)indices, 6u, 0u);
    _mm_empty();
    return 1;
}

u32 __cdecl RasterDraw_TriangleList(u32, u32, const u8 *vertices, u32, const u16 *indices, u32 count, u32) { return IndexedTriangles(vertices, indices, count, 0x006ef7f0u); }

u32 __cdecl RasterDraw_TriangleListNoZ(u32, u32, const u8 *vertices, u32, const u16 *indices, u32 count, u32) { return IndexedTriangles(vertices, indices, count, 0x006f0a50u); }

u32 __stdcall RasterDraw_MappedTriangle(float scale, u32 colour, u32 specular, float u0, float v0, float u1, float v1) {
    const float half = global<float>(0x007013c4), z = 50.0f, w = 0.02f;
    const X87Reg reach = X87Reg(scale) - global<float>(0x007013b4);
    const float side = (float)reach;
    const float u = (float)(X87Reg(u0) * half), v = (float)(X87Reg(v0) * half);
    const float farU = (float)(((X87Reg(u1) - u0) / scale + u1) * half), farV = (float)(((X87Reg(v1) - v0) / scale + v1) * half);
    u32 corners[24] = {0, 0, Bits(z), Bits(w), colour, specular, Bits(u), Bits(v), Bits(side), 0, Bits(z), Bits(w), colour, specular, Bits(farU), Bits(v), 0, Bits(side), Bits(z), Bits(w), colour, specular, Bits(u), Bits(farV)};
    global<u32>(kColour) = 0x7f7f;
    FixedVerts_PackRounded(reinterpret_cast<u8 *>(corners), 3, 0x20);
    g_calls.Cd(0x006f0a50, (u32)(uintptr_t)corners, (u32)(uintptr_t)(corners + 6), (u32)(uintptr_t)(corners + 12));
    _mm_empty();
    return 1;
}
}
