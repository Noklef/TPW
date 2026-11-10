// A model's sprite vertices turned to the camera: see aimsprites.h.
#include "aimsprites.h"

#include <emmintrin.h>

#include <cstdint>
#include <initializer_list>

#include "../core/crt.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../math/sintable.h"

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

const float kNegate = -1.0f;              // 0x6fe2b4
const float kTurn = -0.785398185f;        // 0x6fe2bc
const float kTurnCosine = 0.785398185f;   // 0x6fe2c0
const float kTilt = 0.52359879f;          // 0x6fe2c4
const float kTiltCosine = 2.09439516f;    // 0x6fe2c8
const float kMinusHalfPi = -1.57079637f;  // 0x6fe2a4
const float kOverTwoPi = 0.159154937f;    // 0x6fe2cc

// The table's entry for a scaled angle.
float Table(float scaled) {
    const s32 i = _mm_cvtss_si32(_mm_set_ss(scaled));
    return global<float>(0x007b0968 + 4 * (gSinTableMask & (u32)i));
}

float Scaled(float angle) { return Table((float)(X87Reg(gSinTableScale) * angle)); }

// A sine and cosine pair.
struct Turn {
    float sine, cosine;
};

// Of a fixed angle, the scale times each constant.
Turn FixedTurn(float sine, float cosine) { return {Scaled(sine), Scaled(cosine)}; }

// Of an angle: the cosine by its quarter turn on.
Turn AngleTurn(float angle) { return {Scaled(angle), Table((float)((X87Reg(angle) - kMinusHalfPi) * gSinTableScale))}; }

// About Y: x and z.
void TurnY(float *v, Turn t) {
    const float x = v[0];
    v[0] = (float)(X87Reg(t.cosine) * v[0] + X87Reg(t.sine) * v[2]);
    v[2] = (float)(X87Reg(t.cosine) * v[2] - X87Reg(x) * X87Reg(t.sine));
}

// About X: y and z.
void TurnX(float *v, Turn t) {
    const float y = v[1];
    v[1] = (float)(X87Reg(t.cosine) * v[1] - X87Reg(t.sine) * v[2]);
    v[2] = (float)(X87Reg(t.cosine) * v[2] + X87Reg(y) * X87Reg(t.sine));
}

bool IsNan(float x) {
    const double wide = (double)(float)x;
    return CRT_isnan(at<u32>(&wide, 0), at<u32>(&wide, 4)) != 0;
}

}  // namespace

extern "C" {

void AimSprites_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __cdecl AimSprites_Update(const u8 *instance) {
    if (!(at<u8>(at<u8 *>(at<u8 *>(instance, 0), 0x28), 0) & 4)) return;
    const u32 count = at<u16>(at<u8 *>(at<u8 *>(instance, 0), 4), 0x48);
    float up[3] = {0.0f, 0.0f, 0.0f};   // sic: kept from vertex to vertex (the first's is the stack's in the original)
    s32 tilt = 0, turn = 0;
    for (u32 i = 0; i < count; ++i) {
        const u8 *const holder = at<u8 *>(instance, 0);
        const u8 *const vertex = at<u8 *>(at<u8 *>(holder, 0x28), 4) + 0x14 * i;
        if (!(at<u8>(vertex, 0) & 2)) continue;
        const u8 *const model = at<u8 *>(holder, 4);
        const u32 index = at<u16>(model, 0x46) + i, first = at<u16>(model, 0x44);
        const u32 &face = index < first ? at<u32>(at<u8 *>(model, 0x70), 0xa0 * index) : at<u32>(at<u8 *>(model, 0x74), 0x58 * (index - first));
        float *const matrix = at<float *>(vertex, 4);
        if (face & 0x400) {
            for (u32 k : {0u, 1u, 2u, 8u, 9u, 10u})
                matrix[k] = (float)(X87Reg(matrix[k]) * kNegate);
        }
        const u32 slot = at<u32>(vertex, 0xc);
        if (face & 0x8000) {
            g_calls.Cd(0x00475590, slot, 0xdu, 0xffffffffu);   // SpriteScript_SetReg
            continue;
        }
        g_calls.Cd(0x00475480, slot, at<u32>(matrix, 0x30), at<u32>(matrix, 0x34), at<u32>(matrix, 0x38), 0u);   // SpriteScript_SetPosition
        float view[16];
        Matrix_Invert(view, matrix);
        Matrix_PreMul(global<const float *>(0x00790a98), view);
        const X87Reg length = X87_Sqrt(X87Reg(view[12]) * X87Reg(view[12]) + X87Reg(view[13]) * view[13] + X87Reg(view[14]) * view[14]);
        const float direction[3] = {(float)(X87Reg(view[12]) / length), (float)(X87Reg(view[13]) / length), (float)(X87Reg(view[14]) / length)};
        if (IsNan(direction[0])) {
            tilt = turn = 0;
        } else {
            const Turn eachTurn = FixedTurn(kTurn, kTurnCosine), eachTilt = FixedTurn(kTilt, kTiltCosine);
            float best = 0.0f;
            float candidate[3] = {0.0f, 1.0f, 0.0f};
            for (s32 o = 0; o < 7; ++o) {
                const float height = (float)(X87Reg(direction[1]) * candidate[1]);
                for (s32 c = 0; c < 8; ++c) {
                    const X87Reg dot = X87Reg(direction[0]) * candidate[0] + X87Reg(direction[2]) * candidate[2] + height;
                    if (dot.v > (double)best) {
                        best = (float)dot;
                        tilt = o;
                        turn = c;
                        for (u32 k = 0; k < 3; ++k) up[k] = candidate[k];
                    }
                    TurnY(candidate, eachTurn);
                }
                TurnX(candidate, eachTilt);
            }
        }
        if (tilt == 0 || tilt == 6) turn = 0;
        float side[3] = {0.0f, 0.0f, -1.0f};
        TurnX(side, AngleTurn((float)(X87Reg((double)tilt) * kTilt)));
        TurnY(side, AngleTurn((float)(X87Reg((double)turn) * kTurn)));
        const float cross[3] = {(float)(X87Reg(up[2]) * side[1] - X87Reg(up[1]) * side[2]), (float)(X87Reg(up[0]) * side[2] - X87Reg(up[2]) * side[0]), (float)(X87Reg(up[1]) * side[0] - X87Reg(up[0]) * side[1])};
        float axes[16], facing[16];
        Matrix_Identity(axes);
        Matrix_SetAxes(axes, cross, side, up);
        Matrix_Invert(facing, axes);
        Matrix_PreMul(view, facing);
        const float roll = (float)Matrix_Roll(facing);
        g_calls.Cd(0x00475590, slot, 0xdu, (u32)(turn + tilt * 8));   // SpriteScript_SetReg
        const float scaled = (float)(X87Reg(roll) * kOverTwoPi);
        g_calls.Cd(0x004755d0, slot, 9u, at<u32>(&scaled, 0));   // SpriteScript_SetRegF
        g_calls.Cd(0x004755d0, slot, 8u, gAimSprites_Update_74ced0);
    }
}
}
