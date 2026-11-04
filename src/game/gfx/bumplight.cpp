// A height map lit into a texture: see bumplight.h.
#include "bumplight.h"

#include <cstdint>

#include "../math/vector.h"
#include "../math/x87.h"
#include "array3d.h"
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

const BumpLightEnv kGameEnv = {&GamePow};
const BumpLightEnv *g_env = &kGameEnv;
const BumpStepsEnv kGameSteps = {&Array3D_Realloc, &Image_ErosionRamp, &Image_GaussianBlur, &BumpLight_Render, &Image_CropInto, &Image_Erode, &Image_Dilate, &Image_GaussianBlurLines, &Array3D_Free};
const BumpStepsEnv *g_steps = &kGameSteps;
AddrCalls g_calls;

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const float kOne = 1.0f, kMinusOne = -1.0f, kHalf = 0.5f, kOver255 = 0.00392156886f, k255 = 255.0f;   // 0x702f40, 0x702f58, 0x702f5c, 0x702f60, 0x702f68

}  // namespace

void BumpLight_SetEnv(const BumpLightEnv *env, const AddrCall *table, u32 count) {
    g_env = env ? env : &kGameEnv;
    g_calls.Set(table, count);
}

void BumpSteps_SetEnv(const BumpStepsEnv *env) { g_steps = env ? env : &kGameSteps; }

extern "C" {

void __fastcall BumpLight_Render(u8 *self, void *, const u8 *heights) {
    const u8 *mask = at<const u8 *>(self, 0x38);
    const u8 *height = at<const u8 *>(heights, 0xc);
    u8 *out = at<u8 *>(self, 0x68);
    const s32 texRows = at<s32>(self, 0x24);
    if (!texRows) return;
    const s32 texWidth = at<s32>(self, 0x3c);
    u32 block[4] = {(u32)texWidth, (u32)texRows, 4, 0};
    block[3] = g_calls.Cd(0x0067acc0, (u32)(texRows * texWidth * 4));   // the game's alloc
    g_calls.Th(0x005e80d0, (u32)(uintptr_t)(self + 0x3c), block);       // Image_ShrinkToImage
    const u8 *const texels = reinterpret_cast<const u8 *>((uintptr_t)block[3]);

    const X87Reg pitch = X87Reg(at<float>(self, 0x1c)) * -0.0174532924f;   // 0x702f50
    const X87Reg yaw = X87Reg(at<float>(self, 0x20)) * 0.0174532924f;      // 0x702f54
    const float pitchStored = (float)pitch;
    const float cosPitch = (float)X87Reg(X87_Cos(pitch.v));
    const X87Reg dirX = X87Reg(X87_Cos(yaw.v)) * cosPitch, dirY = X87Reg(-X87_Sin(pitchStored)), dirZ = X87Reg(-(X87Reg(X87_Sin(yaw.v)) * cosPitch).v);
    const X87Reg inverse = X87Reg(kOne) / X87_Sqrt(dirZ * dirZ + dirY * dirY + dirX * dirX);
    const float lightX = (float)(inverse * dirX), lightZ = (float)(inverse * dirZ), lightY = (float)(inverse * dirY);
    const float halfX = (float)(X87Reg(lightX) * kHalf), halfY = (float)((X87Reg(lightY) - kMinusOne) * kHalf);
    const X87Reg halfZReg = inverse * dirZ * kHalf;
    const float halfZ = (float)halfZReg;
    const X87Reg length = X87_Sqrt(halfZReg * halfZ + X87Reg(halfY) * halfY + X87Reg(halfX) * halfX);
    const X87Reg overLength = X87Reg(kOne) / length;
    const float overStored = (float)overLength;
    const float specularX = (float)(overLength * halfX), specularZ = (float)(X87Reg(overStored) * halfZ), specularY = (float)(X87Reg(overStored) * halfY);
    const float ambient = X87_Through(at<float>(self, 0xc)), diffuse = X87_Through(at<float>(self, 0x10)), specular = X87_Through(at<float>(self, 0x14));

    for (s32 row = 0; row < at<s32>(heights, 4); ++row) {
        for (s32 column = 0; column < at<s32>(heights, 0); ++column, ++mask, ++height, out += 4) {
            if (!*mask) {
                out[0] = out[1] = out[2] = out[3] = 0;
                continue;
            }
            const s32 here = height[0];
            const X87Reg slopeX = X87Reg((double)((s32)height[1] - here)) * at<float>(self, 4) * kOver255;
            const X87Reg slopeY = X87Reg((double)((s32)height[at<s32>(heights, 0)] - here)) * at<float>(self, 4) * kOver255;
            const float scale = (float)(X87Reg(kOne) / X87_Sqrt(slopeX * slopeX + slopeY * slopeY - kMinusOne));
            const float normalX = (float)(slopeX * scale), normalY = (float)(slopeY * scale);
            const float lit = (float)((X87Reg(normalX) * lightX + X87Reg(normalY) * lightZ) + X87Reg(scale) * lightY);
            const float shine = (float)((X87Reg(normalX) * specularX + X87Reg(normalY) * specularZ) + X87Reg(scale) * specularY);
            const s32 texRow = (row - at<s32>(self, 0x28)) % texRows;
            const u8 *const texel = texels + (texRow * texWidth + column % texWidth) * 4;
            const float colour[3] = {(float)(X87Reg((double)texel[1]) * kOver255), (float)(X87Reg((double)texel[2]) * kOver255), (float)(X87Reg((double)texel[3]) * kOver255)};
            float sum[3];
            for (u32 k = 0; k < 3; ++k) sum[k] = (float)(X87Reg(colour[k]) * ambient);
            if ((double)lit > 0.0) {
                const X87Reg amount = X87Reg(lit) * diffuse;
                for (u32 k = 0; k < 3; ++k) sum[k] = (float)(X87Reg((float)(amount * colour[k])) + sum[k]);
            }
            if ((double)shine > 0.0) {
                const float amount = (float)(X87Reg(g_env->pow((double)shine, (double)at<float>(self, 0x18))) * specular);
                for (u32 k = 0; k < 3; ++k) sum[k] = (float)(X87Reg(amount) + sum[k]);
            }
            for (u32 k = 0; k < 3; ++k) sum[k] = (float)(X87Reg(sum[k]) * k255);
            Vec3_Clamp(sum, nullptr, 0.0f, 255.0f);
            out[0] = *mask;
            for (u32 k = 0; k < 3; ++k) out[1 + k] = (u8)(s32)sum[k];
        }
    }
    g_calls.Th(0x0055f1e0, (u32)(uintptr_t)block);   // BufferC_FreeAndClear
}

void __fastcall Bump_Build(u8 *self) {
    if (!at<u32>(self, 0x38) || !at<u32>(self, 0x48)) return;
    u8 *const heights = self + 0x2c, *const out = self + 0x4c;
    g_steps->realloc(out, nullptr, at<u32>(self, 0x2c), at<u32>(self, 0x30), 1);
    g_steps->realloc(self + 0x5c, nullptr, at<u32>(self, 0x2c), at<u32>(self, 0x30), 4);
    g_steps->ramp(self, nullptr, reinterpret_cast<const u32 *>(heights), at<u32>(self, 0), reinterpret_cast<const u32 *>(out));
    const float sigma = at<float>(self, 8);
    if (sigma > 0) g_steps->blur(self, nullptr, reinterpret_cast<const u32 *>(out), reinterpret_cast<const u32 *>(heights), sigma);
    g_steps->render(self, nullptr, out);
}

u8 __fastcall Bump_FilterInto(u8 *self, void *, u32 *source, u8 *dest) {
    if (!source[3]) return 0;
    g_steps->realloc(dest, nullptr, source[0], source[1], 1);
    u8 result = g_steps->crop(dest, reinterpret_cast<u8 *>(source), at<s32>(self, 0xc), at<s32>(self, 0x10));
    if (result == 1) {
        const s32 radius = at<s32>(self, 4);
        if (radius < 0) result = static_cast<u8>(g_steps->erode(reinterpret_cast<u32 *>(dest), -radius));
        else if (radius > 0) result = static_cast<u8>(g_steps->dilate(reinterpret_cast<u32 *>(dest), radius));
    }
    if (result != 1) {
        g_steps->free(dest);
        return 0;
    }
    const double sigma = at<float>(self, 8);
    if (sigma > 0 || sigma < 0) result = g_steps->lines(reinterpret_cast<const u32 *>(dest), sigma);
    return result;
}
}
