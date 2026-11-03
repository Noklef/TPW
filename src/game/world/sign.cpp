#include "sign.h"

#include <cmath>
#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../model/parts.h"
#include "../model/ridemodel.h"
#include "../motion/animator.h"
#include "rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const SignEnv kPortEnv = {&Ride_Create, &Model_FindSignEntries, &Holder_SetScale, &Ride_MarkModel, &Holder_FlagModel, &Holder_WakeWithMatrix, &Animator_Play};
const SignEnv *g_env = &kPortEnv;

const float kHalf = 0.5f;            // 0x6fe7d0
const float kTwoThirds = 0.6666667f;   // 0x6fe7d4
const double kAngleWrap = 6.2831854820251465;   // 0x6fe7d8

u8 *Instance(u32 ride) { return global<u8 *>(0x007a4610 + ride * 4); }

// The faces of the model's nodes that use the entry take the other.
void replace(const u8 *model, u32 from, u32 to) {
    const u8 *node = at<const u8 *>(model, 0x70);
    for (u32 n = at<u16>(model, 0x44); n != 0; --n, node += 0xa0) {
        u32 *face = at<u32 *>(node, 0x6c);
        for (u32 k = at<u16>(node, 0x5a); k != 0; --k, face += 4) {
            if (*face == from) *face = to;
        }
    }
}

void entries(u8 *model, u32 type) {
    u8 *const holder = global<u8 *>(0x007ae6a8 + type * 4);
    if (!at<u32>(holder, 0xd0)) return;
    u32 sign1 = 0, sign2 = 0, own1 = 0, own2 = 0;
    g_env->findEntries(at<u8 *>(at<u8 *>(holder, 8), 4), &sign1, &sign2);
    g_env->findEntries(model, &own1, &own2);
    if (!sign1 || !sign2 || !own1 || !own2) return;
    global<u32>(0x007afd8c) = type;
    global<u32>(0x007afd84) = sign1;
    global<u32>(0x007afd7c) = own1;
    global<u32>(0x007afd80) = own2;
    global<u32>(0x007afd88) = sign2;
    replace(model, own1, sign1);
    replace(model, own2, sign2);
}

// The box's measures into the record; the scale.
float measure(const u8 *box, float b, float c) {
    const float *const low = &at<float>(box, 0x18), *const high = &at<float>(box, 0x24);
    float floor[3], centre[3];
    at<u32>(floor, 0) = at<u32>(low, 0);
    floor[1] = 0.0f;
    at<u32>(floor, 8) = at<u32>(low, 8);
    centre[0] = (float)((X87Reg(high[0]) + low[0]) * kHalf);
    at<u32>(centre, 4) = 0;
    centre[2] = (float)((X87Reg(high[2]) + low[2]) * kHalf);
    float &width = global<float>(0x007afd64), &depth = global<float>(0x007afd68), &far = global<float>(0x007afd70), &away = global<float>(0x007afd74);
    width = (float)(X87Reg(high[0]) - low[0]);
    const X87Reg dz = X87Reg(high[2]) - low[2];
    depth = (float)dz;
    const X87Reg hw = X87Reg(width) * kHalf, hz = dz * kHalf;
    const float halfDepth = (float)hz;
    const float diagonal = (float)X87_Sqrt(hw * hw + hz * halfDepth);
    const float fromCentre = (float)Vec3_Distance(centre, high);
    away = fromCentre;
    far = (float)Vec3_Distance(floor, high);

    const X87Reg byC = X87Reg(c) / far;
    float scale = (float)(X87Reg(b) * kTwoThirds / diagonal);
    if (!(byC.v >= (double)scale)) scale = (float)byC;
    const X87Reg t = X87Reg(diagonal) / far, u = X87Reg(fromCentre) / far;
    global<float>(0x007afd6c) = (float)(u / (u + t));
    return scale;
}

}  // namespace

void Sign_SetEnv(const SignEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

u8 *__cdecl Sign_Create(u32 value, float a, u32 height, float b, float c, u32 type) {
    const u32 made = g_env->createRide(type, 0xc01, nullptr, 0, 0, 0, 0, 0, -1, 0);
    u8 *const instance = Instance(made);
    global<u32>(0x007afd0c) = made;
    entries(at<u8 *>(at<u8 *>(instance, 8), 4), type);

    global<float>(0x007afd58) = X87_Through(a);
    global<float>(0x007afd5c) = X87_Through(b);
    global<float>(0x007afd60) = X87_Through(c);
    global<u32>(0x007afd54) = value;
    const float scale = measure(at<const u8 *>(instance, 0xcc), b, c);

    float axes[3];
    for (float &axis : axes) axis = (float)(X87Reg(scale) * 1.0f);
    g_env->setScale(at<const u8 *>(instance, 8), axes);
    float &width = global<float>(0x007afd64), &depth = global<float>(0x007afd68), &far = global<float>(0x007afd70), &away = global<float>(0x007afd74);
    width = (float)(X87Reg(scale) * width * kHalf);
    depth = (float)(X87Reg(scale) * depth * kHalf);
    far = (float)(X87Reg(scale) * far);
    away = (float)(X87Reg(scale) * away);

    float matrix[16];
    Matrix_Identity(matrix);
    at<u32>(matrix, 0x38) = height;
    const u8 *const root = at<const u8 *>(at<const u8 *>(at<const u8 *>(instance, 8), 4), 0x78) + 0x10;
    for (u32 i = 0; i < 16; ++i) global<u32>(0x007afd10 + i * 4) = at<u32>(root, i * 4);

    g_env->markModel(&global<u32>(0x007afd0c));
    const u32 ride = global<u32>(0x007afd0c);
    g_env->flagModel(reinterpret_cast<const u8 *const *>(Instance(ride) + 8));
    at<float *>(Instance(ride), 0xc8) = matrix;   // (sic: dangling)
    g_env->wake(at<u8 *>(Instance(ride), 8), reinterpret_cast<const u32 *>(matrix), 1);
    if (global<u32>(0x007afd0c) != 0) g_env->play(Instance(global<u32>(0x007afd0c)), 5, 0, 1, 1.0f, 0);
    global<u32>(0x007afd08) = 1;
    global<float>(0x007afd50) = (float)X87Reg(std::fmod((double)global<float>(0x007afd50), kAngleWrap));
    return reinterpret_cast<u8 *>(0x007afd08);
}
}
