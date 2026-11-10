#include "vertexfog.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"
#include "../frontend/lobbysky.h"
#include "d3dprobe.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &At(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// fld m32; fistp [0xfb34a4]: rounded to nearest even, 0x80000000 when out of range (as cvtss2si does), kept in the dword.
s32 Fistp(float f) {
    const s32 n = _mm_cvtss_si32(_mm_set_ss(f));
    At<s32>(0xfb34a4) = n;
    return n;
}

}  // namespace

extern "C" {

void __stdcall VertexFog_Colour(u8 *vertices, u32 count, s32 stride) {
    for (u8 *v = vertices; count != 0; --count, v += stride) {
        const float z = *reinterpret_cast<const float *>(v + 8);
        u32 &diffuse = *reinterpret_cast<u32 *>(v + 0x10), &specular = *reinterpret_cast<u32 *>(v + 0x14);
        if (!(z > reinterpret_cast<float &>(At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8c8)))) {
            specular = 0;
            continue;
        }
        if (z >= reinterpret_cast<float &>(At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8cc))) {
            diffuse &= 0xff000000;
            specular = At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8b4);
            continue;
        }
        const X87Reg t = (X87Reg(z) - reinterpret_cast<float &>(At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8c8))) * At<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x8d0);
        const float tf = (float)t;
        const s32 r = Fistp((float)(t * At<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x8bc)));
        const s32 g = Fistp((float)(X87Reg(tf) * At<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x8c0)));
        const s32 b = Fistp((float)(X87Reg(tf) * At<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x8c4)));
        specular = ((u32)b & 0xff) | ((u32)r & 0xff0000) | ((u32)g & 0xff00);
        const X87Reg keep = X87Reg(1.0f) - tf;  // 0x7016c0
        const u32 d = diffuse;
        const float dr = (float)(X87Reg((double)(d & 0xff0000)) * keep);
        const float dg = (float)(X87Reg((double)(d & 0xff00)) * keep);
        const float db = (float)(X87Reg((double)(d & 0xff)) * keep);
        const s32 nr = Fistp(dr), ng = Fistp(dg), nb = Fistp(db);
        diffuse = ((u32)nb & 0xff) | ((u32)nr & 0xff0000) | ((u32)ng & 0xff00) | (d & 0xff000000);
    }
}

void __stdcall VertexFog_Alpha(u8 *vertices, u32 count, s32 stride) {
    for (u8 *v = vertices; count != 0; --count, v += stride) {
        const float z = *reinterpret_cast<const float *>(v + 8);
        u32 &specular = *reinterpret_cast<u32 *>(v + 0x14);
        if (!(z > reinterpret_cast<float &>(At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8c8))))
            specular = 0xff000000;
        else if (z >= reinterpret_cast<float &>(At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8cc)))
            specular = 0;
        else
            specular = (u32)Fistp((float)(((X87Reg(reinterpret_cast<float &>(At<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8cc))) - z) * At<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x8d0)) * 255.0f)) << 24;  // 0x7016e4
    }
}
}
