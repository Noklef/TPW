#include "lobbymodel.h"

#include <cstdint>
#include <cstring>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../motion/animator.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *entry(const u8 *model) { return reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(model, 4)]; }

float *matrix(const u8 *model) {
    return reinterpret_cast<float *>(at<u8 *>(at<u8 *>(at<u8 *>(entry(model), 8), 4), 0x78) + 0x10);
}

}  // namespace

extern "C" {

void __fastcall LobbyModel_SetPosition(const u8 *model, void *, const float *v) {
    u32 bits[3];
    for (int i = 2; i >= 0; --i) {
        std::memcpy(&bits[i], &v[i], 4);
        bits[i] = X87_FloatThrough(bits[i]);
    }
    float *const m = matrix(model);
    std::memcpy(&m[0xc], bits, 12);
}

void __fastcall LobbyModel_TurnY(const u8 *model, void *, float angle) {
    Matrix_RotateEulerFast(matrix(model), 2, &angle);
}

void __fastcall LobbyModel_SetScale(const u8 *model, void *, float x, float y, float z) {
    const float scale[3] = {x, y, z};
    Holder_SetScale(at<u8 *>(entry(model), 8), scale);
}

u32 __fastcall LobbyModel_Play(const u8 *model, void *, u32 entry_, u32 now) {
    if (at<u32>(model, 4) == 0) return 0;
    return Animator_Play(entry(model), 5, entry_, now ? 1 : 0, 1.0f, 0);
}

void __fastcall LobbyModel_AlignTo(const u8 *model, void *, const float *v) {
    const u8 *const ride = entry(model);
    const auto root = [ride] { return at<u8 *>(at<u8 *>(at<u8 *>(ride, 8), 4), 0x78); };
    Matrix_Identity(reinterpret_cast<float *>(root() + 0x10));
    u32 bits[3];
    std::memcpy(bits, v, 12);  // read after the identity (v may be in the matrix)
    float p[3];
    std::memcpy(p, bits, 12);
    const float zero = 0.0f;  // (the argument's slot, cleared)
    const X87Reg c = 0.0f;  // 0x702a38
    // the first row: (z, 0, -x) over its length
    const X87Reg z0 = X87Reg(p[2]) - zero, nx = X87Reg(zero) - p[0];
    const float cf = (float)c, zf = (float)z0, nxf = (float)nx;
    const X87Reg sum = (z0 * z0.v + c * cf) + X87Reg(nxf) * nxf;
    const float length = (float)X87_Sqrt(sum);
    const X87Reg b = X87Reg(cf) / length, d = X87Reg(nxf) / length, a = X87Reg(zf) / length;
    // the second: that row crossed with v, over its length
    const float df = (float)d;
    const X87Reg e1 = d * p[1] - b * p[2];
    const X87Reg e2 = a * p[2] - X87Reg(df) * p[0];
    const X87Reg e3 = b * p[0] - a * p[1];
    const float e1f = (float)e1, e2f = (float)e2, e3f = (float)e3;
    const X87Reg sum2 = (e1 * e1.v + e2 * e2f) + e3 * e3f;
    const float length2 = (float)X87_Sqrt(sum2);
    at<float>(root(), 0x10) = (float)a;
    at<float>(root(), 0x14) = (float)b;
    at<float>(root(), 0x18) = df;
    at<float>(root(), 0x20) = (float)(X87Reg(e1f) / length2);
    at<float>(root(), 0x24) = (float)(X87Reg(e2f) / length2);
    at<float>(root(), 0x28) = (float)(X87Reg(e3f) / length2);
    // the third: v as it is
    at<u32>(root(), 0x30) = bits[0];
    at<u32>(root(), 0x34) = bits[1];
    at<u32>(root(), 0x38) = bits[2];
}

u32 __fastcall LobbyModel_TimeLeft(const u8 *model) {
    if (at<u32>(model, 4) == 0) return 0;
    return Layer_GetPlayback(entry(model), nullptr, nullptr, nullptr, 0);
}
}
