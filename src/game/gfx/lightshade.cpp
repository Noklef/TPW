#include "lightshade.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../math/vector.h"
#include "../math/x87.h"
#include "imageops.h"

namespace {

__declspec(naked) double __cdecl GamePow(double, double) {
    __asm {
        fld qword ptr [esp + 4]
        fld qword ptr [esp + 12]
        mov eax, 0x0067c5d0
        call eax
        ret
    }
}

const LightShadeEnv kGameEnv = {&GamePow};
const LightShadeEnv *g_env = &kGameEnv;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

float constant(u32 address) { return *reinterpret_cast<const float *>((uintptr_t)address); }

X87Reg negated(X87Reg x) { return X87Reg(-x.v); }

}  // namespace

void LightShade_SetEnv(const LightShadeEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall LightShade_Apply(u8 *self, void *, const u8 *heights) {
    const float one = constant(0x00702f40), minusOne = constant(0x00702f58), half = constant(0x00702f5c), inverse255 = constant(0x00702f60);
    const u8 *height = at<const u8 *>(heights, 0xc);
    const u8 *mask = at<const u8 *>(self, 0x38);
    u8 *out = at<u8 *>(self, 0x68);
    const u32 rows = at<u32>(self, 0x24);
    if (!rows)
        return;
    u32 tile[4] = {at<u32>(self, 0x3c), rows, 4, 0};
    tile[3] = (u32)(uintptr_t)GameHeap_Alloc(((rows * tile[0]) << 2));
    Image_ShrinkToImage(self + 0x3c, nullptr, reinterpret_cast<u8 *>(tile));

    // The light's direction from its two angles.
    const X87Reg a = X87Reg(at<float>(self, 0x1c)) * constant(0x00702f50), b = X87Reg(at<float>(self, 0x20)) * constant(0x00702f54);
    const float aStored = (float)a;
    const X87Reg cosA = X87_Cos(a.v), sinB = X87_Sin(b.v), sinA = X87_Sin(X87Reg(aStored).v), cosB = X87_Cos(b.v);
    const float cosAStored = (float)cosA;
    const X87Reg z = negated(sinB * cosAStored), x0 = cosB * cosAStored, y0 = negated(sinA);
    const X87Reg sum = ((z * z) + (y0 * y0)) + (x0 * x0);
    const X87Reg scale = one / X87_Sqrt(sum);
    const X87Reg dirX = scale * x0, dirZ = scale * z, dirY = scale * y0;
    const float lightX = (float)dirX, lightY = (float)dirZ, lightZ = (float)dirY, lightXAgain = (float)dirX;

    // The half vector, normalised.
    const X87Reg halfA = dirZ * half, halfB = (X87Reg(lightZ) - minusOne) * half, halfC = X87Reg(lightXAgain) * half;
    float specY = (float)halfA;
    const X87Reg squareA = halfA * specY;
    float specZ = (float)halfB;
    const X87Reg squareB = halfB * specZ;
    float specX = (float)halfC;
    const X87Reg squareC = halfC * specX;
    const X87Reg length = X87_Sqrt((squareA + squareB) + squareC);
    const X87Reg inverse = one / length;
    const float inverseStored = (float)inverse;
    specX = (float)(inverse * specX);
    specY = (float)(X87Reg(inverseStored) * specY);
    specZ = (float)(X87Reg(inverseStored) * specZ);
    const float ambient = X87_Through(at<float>(self, 0xc)), diffuse = X87_Through(at<float>(self, 0x10)), specular = X87_Through(at<float>(self, 0x14));

    for (s32 y = 0; y < at<s32>(heights, 4); ++y) {
        for (s32 x = 0; x < at<s32>(heights, 0); ++x) {
            if (*mask) {
                const s32 base = height[0];
                const s32 dx = height[1] - base, dy = height[at<s32>(heights, 0)] - base;
                const float heightScale = at<float>(self, 4);
                const X87Reg u = (X87Reg((double)dx) * heightScale) * inverse255, v = (X87Reg((double)dy) * heightScale) * inverse255;
                const float normalZ = (float)(one / X87_Sqrt(((u * u) + (v * v)) - minusOne));
                const float normalX = (float)(X87Reg(normalZ) * u), normalY = (float)(X87Reg(normalZ) * v);

                const float lit = (float)(((X87Reg(normalX) * lightX) + (X87Reg(normalY) * lightY)) + (X87Reg(normalZ) * lightZ));
                const float shine = (float)(((X87Reg(normalX) * specX) + (X87Reg(normalY) * specY)) + (X87Reg(normalZ) * specZ));

                const s32 row = (s32)((u32)y - at<u32>(self, 0x28)) % (s32)tile[1];
                const s32 column = x % (s32)tile[0];
                const u8 *const texel = reinterpret_cast<const u8 *>((uintptr_t)tile[3]) + ((u32)row * tile[0] + (u32)column) * tile[2];
                const float red = (float)(X87Reg((double)texel[1]) * inverse255), green = (float)(X87Reg((double)texel[2]) * inverse255), blue = (float)(X87Reg((double)texel[3]) * inverse255);

                float colour[3] = {(float)(X87Reg(red) * ambient), (float)(X87Reg(green) * ambient), (float)(X87Reg(blue) * ambient)};
                if (lit > 0.0f) {
                    const X87Reg level = X87Reg(lit) * diffuse;
                    const float partRed = (float)(level * red), partGreen = (float)(level * green), partBlue = (float)(level * blue);
                    colour[0] = (float)(X87Reg(partRed) + colour[0]);
                    colour[1] = (float)(X87Reg(partGreen) + colour[1]);
                    colour[2] = (float)(X87Reg(partBlue) + colour[2]);
                }
                if (shine > 0.0f) {
                    const float highlight = (float)(X87Reg(g_env->pow(X87Reg(shine).v, X87Reg(at<float>(self, 0x18)).v)) * specular);
                    colour[0] = (float)(X87Reg(highlight) + colour[0]);
                    colour[1] = (float)(X87Reg(highlight) + colour[1]);
                    colour[2] = (float)(X87Reg(highlight) + colour[2]);
                }
                const float full = constant(0x00702f68);
                colour[0] = (float)(X87Reg(colour[0]) * full);
                colour[1] = (float)(X87Reg(colour[1]) * full);
                colour[2] = (float)(X87Reg(colour[2]) * full);
                Vec3_Clamp(colour, nullptr, 0.0f, 255.0f);
                out[0] = *mask;
                out[1] = (u8)(long long)X87Reg(colour[0]).v;  // __ftol, low byte
                out[2] = (u8)(long long)X87Reg(colour[1]).v;
                out[3] = (u8)(long long)X87Reg(colour[2]).v;
            } else {
                out[0] = 0;
                out[1] = 0;
                out[2] = 0;
                out[3] = 0;
            }
            out += 4;
            ++mask;
            ++height;
        }
    }
    BufferC_FreeAndClear(reinterpret_cast<u8 *>(tile));
}
