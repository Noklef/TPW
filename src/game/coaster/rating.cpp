#include "rating.h"

#include <cmath>
#include <cstring>

#include "../math/x87.h"

namespace {

CoasterRatingState &state() { return *reinterpret_cast<CoasterRatingState *>(0x007923a8); }

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }
// fld m32; fstp m32 (a signalling NaN comes out quiet), where the compiler would just copy.
float through(float x) {
    u32 b;
    std::memcpy(&b, &x, 4);
    b = X87_FloatThrough(b);
    std::memcpy(&x, &b, 4);
    return x;
}
template <typename T> const T &at(const void *p, u32 offset) {
    return *reinterpret_cast<const T *>(static_cast<const u8 *>(p) + offset);
}

const float kHalfPi = 1.57079637f;  // 0x6fe0b8

// The type's modifiers (BalanceTrack offset - 0x3818).
enum : u32 {
    kInitialSickness = 0x320,
    kInitialExcitement = 0x324,
    kGForceSickness = 0x32c,
    kUpsideDownSickness = 0x330,
    kSpeedExcitement = 0x334,
    kGForceExcitement = 0x338,
    kBankExcitement = 0x340,
    kXGForceUncomfortable = 0x344,
    kInvertedUncomfortable = 0x348,
    kNormalScreamStart = 0x34c,
    kTerrorScreamStart = 0x350,
    kNormalScreamSustain = 0x354,
    kTerrorScreamSustain = 0x358,
};

// x87 comparisons: an unordered result reads as "less" and "equal" both.
bool above(double a, double b) { return a > b; }
bool atMost(double a, double b) { return !(a > b); }

// The x87's arithmetic on its registers (24-bit significands, a wide exponent, as doubles;
// see x87.h), a being the destination: two NaNs give the one with the larger significand,
// a on a tie. Only a store to a float rounds (or overflows) again.
double add(double a, double b) { return X87_Add(a, b); }
double sub(double a, double b) { return a != a || b != b ? X87_Add(a, b) : X87_Add(a, -b); }
double mul(double a, double b) { return a != a && b != b ? X87_Add(a, b) : X87_Round(a * b); }

}  // namespace

extern "C" {

void __cdecl CoasterRating_Reset(const u8 *coaster) {
    const u8 *type = at<const u8 *>(coaster, 4);
    CoasterRatingState &g = state();
    g.sickness = at<float>(type, kInitialSickness);
    g.excitement = g.lastExcitement = through(at<float>(type, kInitialExcitement));
    g.lastForce[0] = g.lastForce[1] = g.lastForce[2] = 0;
    g.unknownA8 = 0;
    g.lastBank = 0;
    g.maxExcitement = g.maxSickness = 0;
    g.terrified = g.screaming = 0;
}

void __cdecl CoasterRating_Step(u8 *coaster, u32 node, u32, float speed, float bank, const float *force, float) {
    const u8 *type = at<const u8 *>(coaster, 4);
    u32 &flags = *reinterpret_cast<u32 *>(at<u8 *>(coaster, 0x9c) + node * 0x38);
    CoasterRatingState &g = state();
    auto m = [&](u32 offset) { return (double)at<float>(type, offset); };
    const double f0 = force[0], f1 = force[1], f2 = force[2];

    // The float casts are the original's stores to locals.
    const double change = add(add(std::fabs(sub(g.lastForce[1], f1)), std::fabs(sub(g.lastForce[2], f2))),
                              std::fabs(sub(g.lastForce[0], f0)));
    const float forceChange = (float)mul(change, m(kGForceSickness));
    const double over = sub(std::fabs((double)bank), kHalfPi);
    const float past = above(over, 0) ? (float)over : 0.0f;
    const float upside = past == 0 ? 0.0f : 1.0f;
    const double sickness = add(mul(add(upside, past), m(kUpsideDownSickness)), forceChange);

    const float bankChange = (float)std::fabs(sub(bank, g.lastBank));
    const double total = add(add(std::fabs(f2), std::fabs(f1)), std::fabs(f0));
    const double thrill = add(add(mul(total, m(kGForceExcitement)), mul(bankChange, m(kBankExcitement))),
                              mul(m(kSpeedExcitement), speed));
    const double sideways = mul(m(kXGForceUncomfortable), std::fabs(f0));
    const float inverted = (float)add(past == 0 ? 0.0 : 1.0, past);
    double uncomfortable = inverted;
    if (inverted != 0 && !(force[1] >= 0.1f)) {
        uncomfortable = mul(m(kInvertedUncomfortable), inverted);
        if (!(force[1] >= 0)) uncomfortable = mul(uncomfortable, 3.0);
    }
    double excitement = sub(thrill, add(sideways, uncomfortable));
    if (atMost(excitement, 0)) excitement = 0;

    g.sickness = (float)sub(sickness, mul(g.sickness, -0.5));
    const bool sicknessPeak = atMost(g.maxSickness, g.sickness);
    g.excitement = (float)sub(excitement, mul(g.excitement, -0.5));
    if (sicknessPeak) g.maxSickness = g.sickness;
    if (atMost(g.maxExcitement, g.excitement)) g.maxExcitement = g.excitement;

    flags &= ~7u;
    if (node > 2) {
        const double rise = sub(g.excitement, g.lastExcitement);
        if (above(rise, m(kNormalScreamStart))) {
            g.screaming = 1;
            flags |= 4;
        } else if (g.screaming && above(g.excitement, m(kNormalScreamSustain))) {
            flags |= 4;
        } else {
            g.screaming = 0;
        }
        if (above(rise, m(kTerrorScreamStart))) {
            g.terrified = 1;
            flags |= 2;
        } else if (g.terrified && above(g.excitement, m(kTerrorScreamSustain))) {
            flags |= 2;
        } else {
            g.terrified = 0;
        }
    }
    g.lastExcitement = g.excitement;
}

void __cdecl CoasterRating_Finish(u8 *coaster, float maxHeight, float topSpeed, float) {
    const CoasterRatingState &g = state();
    float sickness = (float)add(g.maxSickness, g.sickness);  // stored, then clamped
    double excitement = sub(mul(g.excitement, 0.5), mul(g.maxExcitement, -1.7f));
    if (!(100.0 >= excitement)) excitement = 100.0;
    if (excitement < 0) excitement = 0;
    if (!(100.0f >= sickness)) sickness = 100.0f;
    if (sickness < 0) sickness = 0;
    at<float>(coaster, 0x104) = (float)excitement;
    at<float>(coaster, 0x100) = sickness;
    at<u32>(coaster, 0x108) = 0;
    for (u8 *car = at<u8 *>(coaster, 0x54); car; car = at<u8 *>(car, 0x64))
        if ((at<u32>(car, 8) & 0x40) && (at<u32>(car, 8) & 0x80)) ++at<u32>(coaster, 0x108);
    const float product = (float)mul(at<float>(coaster, 0xac), at<float>(coaster, 0xa8));
    at<float>(coaster, 0x110) = through(maxHeight);
    at<float>(coaster, 0x10c) = product;
    at<float>(coaster, 0x114) = through(topSpeed);
}
}
