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

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// v (three floats) over its length sqrt(x * x + y * y + z * z), in place.
void Normalise(float *v) {
    const X87Reg x = v[0], y = v[1], z = v[2];
    const X87Reg length = X87_Sqrt(x * x + y * y + z * z);
    v[0] = (float)(X87Reg(v[0]) / length);
    v[1] = (float)(X87Reg(v[1]) / length);
    v[2] = (float)(X87Reg(v[2]) / length);
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

void __fastcall LobbyFlyer_Step(u8 *flyer) {
    float *const pos = &at<float>(flyer, 0x30);
    float *const heading = &at<float>(flyer, 0x48);
    float *const wanted = &at<float>(flyer, 0x54);
    float d[3];
    for (u32 k = 0; k < 3; ++k)
        d[k] = (float)(X87Reg(at<float>(flyer, 0x3c + 4 * k)) - pos[k]);
    for (u32 k = 0; k < 3; ++k)
        wanted[k] = d[k];
    Normalise(wanted);
    const X87Reg turn = X87Reg(at<float>(global<u8 *>(0x00f82884), 8)) * global<float>(0x00702ae4);   // 0.1
    for (u32 k = 0; k < 3; ++k)
        heading[k] = (float)((X87Reg(wanted[k]) - heading[k]) * turn + heading[k]);
    Normalise(heading);
    const X87Reg step = X87Reg(at<float>(flyer, 0x28)) * at<float>(global<u8 *>(0x00f82884), 8);
    pos[0] = (float)(step * heading[0] + pos[0]);
    pos[1] = (float)(X87Reg(heading[1]) * step + pos[1]);
    pos[2] = (float)(X87Reg(heading[2]) * step + pos[2]);
    const X87Reg dx = X87Reg(at<float>(flyer, 0x3c)) - pos[0];
    const X87Reg dy = X87Reg(at<float>(flyer, 0x40)) - pos[1];
    const X87Reg dz = X87Reg(at<float>(flyer, 0x44)) - pos[2];
    const X87Reg reach = dx * dx + dy * dy + dz * dz;
    const float limit = at<float>(flyer, 0x2c);
    if (reach.v < limit || reach.v != reach.v || limit != limit) {   // (below or unordered)
        for (u32 k = 0; k < 3; ++k) {
            u8 *const scene = global<u8 *>(0x00f82884);
            const u32 next = at<u32>(scene, 0x34) * 0x19660du + 0x3c6ef35fu;
            at<u32>(scene, 0x34) = next;
            const u32 turned = next >> 13 | next << 19;
            at<u32>(scene, 0x34) = turned;
            const float extent = at<float>(flyer, 0x1c + 4 * k);
            const X87Reg r = X87Reg((double)(turned & 0x3fffffff)) * global<float>(0x00702adc) * extent;   // 2^-30
            at<float>(flyer, 0x3c + 4 * k) = (float)(r - X87Reg(extent) * global<float>(0x00702ae0) + at<float>(flyer, 0x10 + 4 * k));   // 0.5
        }
    }
    LobbyModel_AlignTo(at<u8 *>(flyer, 8), nullptr, heading);
    LobbyModel_SetPosition(at<u8 *>(flyer, 8), nullptr, pos);
}
}
