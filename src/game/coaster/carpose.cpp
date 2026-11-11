#include "carpose.h"

#include <cmath>
#include <cstdint>
#include <emmintrin.h>

#include "../math/matrix.h"
#include "../math/vector.h"
#include "../math/x87.h"
#include "../motion/animator.h"
#include "cars.h"
#include "handles.h"
#include "orient.h"
#include "train.h"
#include "../math/sintable.h"
#include "coastersupdate.h"
#include "coastersload.h"
#include "../world/rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const CoasterPoseEnv kPorts = {&TrackPiece_Handles, &Car_SetChannelRates, &Animator_Update, &TrackPiece_NodeHandles};
const CoasterPoseEnv *g_env = &kPorts;

const float kMinusFive = -5.0f;      // 0x6fdea4
const float kToTurns = 0.159154937f;  // 0x6fdea8, 1 / 2 pi
const float kMinusHalf = -0.5f;      // 0x6fdea0
const float kMinusOne = -1.0f;       // 0x6fde9c
const float kHalf = 0.5f;            // 0x6fdeac
const float kMinusHalfPi = -1.57079637f;  // 0x6fdeb0
const float kTwo = 2.0f;             // 0x74cad0

// fld, fchs, fstp.
float Negated(float x) {
    union {
        float f;
        u32 bits;
    } u;
    u.f = x;
    u.bits = X87_FloatThrough(u.bits) ^ 0x80000000u;
    return u.f;
}

s32 Fistp(float x) { return _mm_cvtss_si32(_mm_set_ss(x)); }

void CopyWords(void *to, const void *from) {
    at<u32>(to, 0) = at<u32>(from, 0);
    at<u32>(to, 4) = at<u32>(from, 4);
    at<u32>(to, 8) = at<u32>(from, 8);
}

bool On(const u8 *piece) { return !(at<u8>(at<u8 *>(piece, 0x100), 4) & 1) && !(at<u32>(piece, 8) & 0x2000); }

void PoseGroupCar(u8 *car) {
    u8 *const kind = at<u8 *>(car, 0x100);
    const u8 *const type = at<const u8 *>(kind, 0x18);
    if (!On(car) || at<s32>(type, 0x2c) == -1)
        return;
    CopyWords(car + 0x4c, kind + 0x1c);
    const u32 seat = at<u32>(car, 4);
    at<float>(car, 0x4c) = (float)(X87Reg(at<float>(car, 0x4c)) - kMinusFive);
    at<float>(car, 0x54) = (float)(X87Reg(at<float>(car, 0x54)) - kMinusFive);
    const X87Reg height = (X87Reg(at<float>(type, 0xc0)) - at<float>(type, 0xbc)) * at<float>(car, 0xe8) + at<float>(type, 0xbc);
    if (!seat) {
        at<float>(car, 0x50) = (float)(height + at<float>(car, 0x50));
        return;
    }
    at<float>(car, 0x50) = (float)height;
    const X87Reg raised = height + at<float>(type, 0xc4);
    at<float>(car, 0x50) = (float)raised;
    at<float>(car, 0x50) = (float)(X87Reg(at<float>(at<u8 *>(kind, 0x30 + seat * 4), 0x50)) + raised);
}

// The node of an "on" piece: its bone's place, and with no flag 0x40 the roll.
void PoseOnNode(u8 *piece, const u8 *animator, u8 *&in, u8 *&out) {
    const u32 flags = at<u32>(piece, 8);
    u8 *const bone = at<u8 *>(at<u8 *>(at<u8 *>(animator, 8), 0x28), 4) + at<u32>(in, -0x3c) * 20;
    u8 *const matrix = at<u8 *>(bone, 4);
    if (flags & 0x10000000)
        Matrix_RotateYAngle(reinterpret_cast<float *>(matrix), -3.14159274f);
    CopyWords(in, matrix + 0x30);
    if (at<u8>(piece, 8) & 0x40) {
        at<float>(out, -4) = (float)(X87Reg(at<float>(matrix, 0x10)) * kTwo + at<float>(in, 0));
        at<float>(out, 0) = (float)(X87Reg(at<float>(matrix, 0x14)) * kTwo + at<float>(in, 4));
        at<float>(out, 4) = (float)(X87Reg(at<float>(matrix, 0x18)) * kTwo + at<float>(in, 8));
    } else {
        at<u32>(out, -4) = at<u32>(in, 0);
        at<float>(out, 0) = (float)(X87Reg(at<float>(in, 4)) - kMinusOne);
        at<u32>(out, 4) = at<u32>(in, 8);
        u32 side[3], up[3];
        CopyWords(side, matrix + 0x10);
        CopyWords(up, matrix + 0x20);
        float *const sideF = reinterpret_cast<float *>(side), *const upF = reinterpret_cast<float *>(up);
        Vec3_RotateY(sideF, Negated(at<float>(piece, 0xe0)));
        Vec3_RotateY(upF, Negated(at<float>(piece, 0xe0)));
        const double angle = Vec3_AngleX(upF);
        Vec3_RotateX(sideF, Negated((float)angle));
        at<float>(piece, 0xe4) = (float)Vec3_AngleZ(sideF);
    }
    in += 0x58;
    out += 0x58;
}

// The node of an "off" piece. sic: in and out stay where they are.
void PoseOffNode(u8 *piece, const u8 *in, u8 *out) {
    float offset[3] = {0.0f, 1.0f, 0.0f};
    if (at<u8>(piece, 8) & 0x40) {
        const float angle = at<float>(piece, 0xe0);
        offset[1] = 0.0f;
        offset[2] = -1.0f;
        const float a = (float)(X87Reg(angle) * gSinTableScale);
        const float b = (float)((X87Reg(angle) - kMinusHalfPi) * gSinTableScale);
        const s32 s = Fistp(a) & reinterpret_cast<s32 &>(gSinTableMask);
        const s32 c = Fistp(b) & reinterpret_cast<s32 &>(gSinTableMask);
        Vec3_RotateYCS(offset, gSinTable[c], gSinTable[s]);
    }
    at<float>(out, -4) = (float)(X87Reg(offset[0]) + at<float>(in, 0));
    at<float>(out, 0) = (float)(X87Reg(at<float>(in, 4)) + offset[1]);
    at<float>(out, 4) = (float)(X87Reg(at<float>(in, 8)) + offset[2]);
}

}  // namespace

void CoasterPose_SetEnv(const CoasterPoseEnv *env) { g_env = env ? env : &kPorts; }

extern "C" void __cdecl Coaster_PoseCars() {
    const u8 *const rules = at<const u8 *>(gCoastersupdate_792218, 4);
    for (u8 *group = at<u8 *>(gCoastersload_790fe4, 0x10); group; group = at<u8 *>(group, 0x28))
        for (u32 k = 0; k < at<u32>(group, 0x30); k++)
            PoseGroupCar(at<u8 *>(group, 0x34 + k * 4));
    for (u8 *piece = at<u8 *>(gCoastersload_790fe4, 0x18); piece; piece = at<u8 *>(piece, 0x64))
        g_env->handles(piece, rules, gCoastersload_790fe4);

    const bool speeds = at<u32>(rules, 0) & 0x10;
    const float minimum = at<float>(rules, 0x48);
    float speed = 0.0f;
    const u8 *animator = nullptr;
    for (u8 *piece = at<u8 *>(gCoastersload_790fe4, 0x18); piece; piece = at<u8 *>(piece, 0x64)) {
        const u8 *const next = at<const u8 *>(piece, 0x64);
        u32 direction[3];
        CopyWords(direction, piece + 0x34);
        TrackOrientation orientation;
        TrackOrientation_Init(reinterpret_cast<u32 &>(gCoastersload_790fe4), reinterpret_cast<const Vec3 *>(direction), at<u32>(piece, 0x6c), &orientation, 1, nullptr);
        at<float>(piece, 0xd4) = orientation.yaw;
        if (!(at<u8>(piece, 8) & 1))
            at<float>(piece, 0xe0) = orientation.yaw;
        if (!(at<u8>(piece, 8) & 2))
            at<float>(piece, 0xdc) = orientation.pitch;
        if (!(at<u8>(piece, 8) & 4))
            at<float>(piece, 0xd8) = 0.0f;
        if (speeds && !(at<u8>(piece, 8) & 4) && next) {
            at<float>(piece, 0xd8) = (float)CoasterCar_SwingRate(gCoastersupdate_792218, piece, speed);
            speed = (float)Coaster_SpeedAfterDrop(rules, speed, minimum, at<float>(piece, 0x50), at<float>(next, 0x50));
        }

        const X87Reg turn = X87Reg(at<float>(piece, 0xe0)) * kToTurns;
        at<float>(piece, 0xec) = (float)turn;
        double whole;
        const float part = (float)std::modf(turn.v, &whole);
        at<float>(piece, 0xec) = !(part >= 0.0f) ? (float)(X87Reg(part) - kMinusOne) : part;

        const X87Reg second = X87Reg(at<float>(piece, 0xdc)) * kToTurns;
        at<u32>(piece, 0xf4) = at<u32>(piece, 0xd8);
        at<float>(piece, 0xf0) = (float)second;
        at<float>(piece, 0xf4) = (float)(X87Reg(at<float>(piece, 0xf4)) * kToTurns);
        X87Reg wrapped = Float_Modf((float)(X87Reg(at<float>(piece, 0xf4)) - kMinusHalf), nullptr);
        if (!(wrapped.v >= 0.0))
            wrapped = wrapped - kMinusOne;
        at<float>(piece, 0xf4) = (float)(wrapped - kHalf);

        if (On(piece)) {
            g_env->channelRates((u32)(uintptr_t)rules, piece);
            animator = gRides_7a4610[at<u32>(piece, 0x70)];
            g_env->animate(const_cast<u8 *>(animator), reinterpret_cast<const u8 *>(1), 0);
        }

        const u8 *const list = at<const u8 *>(piece, 0x90);
        if (!at<u32>(list, 0))
            continue;
        u8 *in = at<u8 *>(list, 4) + 0x40;
        u8 *out = at<u8 *>(list, 8) + 0x44;
        for (u32 k = 0; k < at<u32>(at<const u8 *>(piece, 0x90), 0); k++) {
            if (On(piece))
                PoseOnNode(piece, animator, in, out);
            else
                PoseOffNode(piece, in, out);
        }
    }

    for (u8 *piece = at<u8 *>(gCoastersload_790fe4, 0x18); piece; piece = at<u8 *>(piece, 0x64))
        g_env->nodeHandles(piece, On(piece) ? gRides_7a4610[at<u32>(piece, 0x70)] : nullptr);
}
