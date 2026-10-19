#include "viewquad.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"

namespace {

// fptan: the CRT's tan, which from |x| = 12288 runs fptan itself, with the hardware's reduction by its 66-bit pi (an exact reduction
// would differ in the float from |x| of about 2^33 on).
double tangent_of(double x) { return std::tan(x); }

}  // namespace

extern "C" {

void __fastcall ViewQuad_SetFov(float *quad, void *, float fov) {
    const X87Reg angle = X87Reg(fov) * 3.14159274f * 0.00277777785f;
    const X87Reg one = X87Reg(0.0f) - -1.0f;
    // fptan: from 2^63 on it pushes nothing, so the fxch after it meets an empty register (the indefinite) and the indefinite is what
    // comes out as the tangent (sic).
    double tangent = X87_Trig(angle.v, tangent_of);
    if (std::isfinite(angle.v) && std::fabs(angle.v) >= 9223372036854775808.0) {
        const unsigned long long indefinite = 0xfff8000000000000ull;
        std::memcpy(&tangent, &indefinite, 8);
    }
    const X87Reg h = X87Reg(tangent) * one * 0.5f;
    const float hf = static_cast<float>(h);
    const X87Reg w = h / *reinterpret_cast<const float *>((uintptr_t)0x008bcbcc);
    const float wf = static_cast<float>(w);
    quad[2] = quad[5] = quad[8] = quad[11] = 1.0f;
    quad[0] = -wf;
    quad[1] = hf;
    quad[3] = wf;
    quad[4] = hf;
    quad[6] = wf;
    quad[7] = -hf;
    quad[9] = -wf;
    quad[10] = -hf;
}
}
