#include "train.h"

#include <cmath>
#include <cstdint>

#include "../core/globalmem.h"
#include "../math/x87.h"
#include "../model/gridfit.h"
#include "../model/parts.h"
#include "../motion/animator.h"
#include "helpers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(uintptr_t address) { return *reinterpret_cast<T *>(address); }

const TrainEnv kGameEnv = {&Animator_Update};
const TrainEnv *g_trainEnv = &kGameEnv;

u8 *RideAt(u32 index) { return reinterpret_cast<u8 *const *>(0x007a4610)[index]; }

}  // namespace

extern "C" {

void __cdecl Train_SetCouplings(u32, u8 *train) {
    if (*reinterpret_cast<const u32 *>(0x00790afc)) return;
    for (u32 i = 0; i < at<u32>(train, 0x30); ++i) {
        u8 *car = at<u8 *>(train, 0x34 + 4 * i);
        const u32 count = at<u32>(train, 0x30);
        u32 flags = at<u32>(car, 8);
        flags = count > 1 && i < count - 1 ? flags | 0x100 : flags & ~0x100u;
        at<u32>(car, 8) = flags;
        Car_PickCoupling(car);
    }
}

void __cdecl Car_PickCoupling(u8 *car) {
    const u32 flags = at<u32>(car, 8);
    const u8 *type = at<const u8 *>(at<const u8 *>(car, 0x100), 0x18);
    const bool rear = !(flags & 0x2000) && (flags & 0x100) && (at<u8>(type, 0) & 8);
    at<u8 *>(car, 0x90) = car + (rear ? 0x84 : 0x78);
    at<u32>(car, 0x74) = at<u32>(at<const u8 *>(car, 0x100), 0x18) + (rear ? 0x44 : 0x30);
}

double __cdecl Car_TrainLength(const u8 *car) {
    if (!car) return 0.0;
    const u8 *train = at<const u8 *>(car, 0x100);
    const u8 *type = at<const u8 *>(train, 0x18);
    const float shortest = at<float>(type, 0xbc), longest = at<float>(type, 0xc0);
    X87Reg sum = 0.0;
    for (u32 i = 0, n = at<u32>(car, 4); i < n; ++i) {
        const u8 *other = at<const u8 *>(train, 0x34 + 4 * i);
        if (!other) continue;
        sum = sum + (X87Reg(longest) - shortest) * at<float>(other, 0xe8);
        sum = sum + shortest;
        sum = sum + at<float>(type, 0xc4);
    }
    return ((sum + (X87Reg(longest) - shortest) * at<float>(car, 0xe8)) + shortest).v;
}

void __cdecl Car_SetChannelRates(u32, u8 *car) {
    const u8 *const train = at<const u8 *>(car, 0x100);
    if ((at<u8>(train, 4) & 1) || (at<u32>(car, 8) & 0x2000)) return;
    constexpr float kMinusOne = -1.0f, kMinusHalf = -0.5f;   // 0x6fde9c, 0x6fdea0 (subtracted)
    const u32 ride = at<u32>(car, 0x70);
    const u8 *const ranges = at<const u8 *>(train, 0x18) + 0xd0;
    const auto set = [ride, ranges](u32 channel, u32 range, X87Reg rate) {
        if (!(rate.v >= at<float>(ranges, range))) rate = at<float>(ranges, range);   // (fcom; test ah, 1: a NaN too)
        if (rate.v > at<float>(ranges, range + 4)) rate = at<float>(ranges, range + 4);
        u8 *const block = at<u8 *>(reinterpret_cast<u8 *const *>(0x007a4610)[ride], 0x10) + channel;
        at<float>(block, 0x20) = (float)(X87Reg(at<float>(block, 0x1c)) * rate);
    };
    float &t = at<float>(car, 0xe8);
    if (t > 1.0f)   // 0x6fde70
        t = 1.0f;
    else if (!(t >= 0.0f))   // 0x6fde74
        t = 0.0f;
    double whole;
    const double phase = std::modf((double)at<float>(car, 0xf8), &whole);   // CRT modf (0x67b280)
    at<float>(car, 0xf8) = !(phase >= 0.0) ? (float)(X87Reg((float)phase) - kMinusOne) : (float)phase;
    set(0, 0, X87Reg(t) * at<float>(ranges, 8) + at<float>(ranges, 0xc));
    X87Reg pitch = at<float>(car, 0xec);
    if (at<u32>(car, 8) & 0x10000000) pitch = std::modf((pitch - kMinusHalf).v, &whole);
    set(0x38, 0x10, pitch * at<float>(ranges, 0x18) + at<float>(ranges, 0x1c));
    set(0xe0, 0x40, X87Reg(at<float>(car, 0xf8)) * at<float>(ranges, 0x48) + at<float>(ranges, 0x4c));
    set(0x70, 0x20, X87Reg(at<float>(car, 0xf0)) * at<float>(ranges, 0x28) + at<float>(ranges, 0x2c) - kMinusHalf);
    set(0xa8, 0x30, X87Reg(at<float>(car, 0xf4)) * at<float>(ranges, 0x38) + at<float>(ranges, 0x3c) - kMinusHalf);
}

void __cdecl Car_FitHeight(const u8 *owner, u8 *car, const u8 *type, float height, u32 free) {
    X87Reg v = height;
    if (const u32 slot = at<u32>(car, 4)) {
        const u8 *const before = at<const u8 *>(at<const u8 *>(car, 0x100), 0x30 + slot * 4);
        const X87Reg below = before ? X87Reg(Car_TrainLength(before)) : X87Reg(0.0f);   // (inlined: the same sums)
        v = X87_SubR(below + at<float>(type, 0xc4), X87Reg(height));   // fsubr
    }
    if (!free) {
        if (!(v.v > at<float>(type, 0x68))) v = at<float>(type, 0x68);
        if (v.v >= at<float>(type, 0x6c)) v = at<float>(type, 0x6c);   // (a NaN top keeps v)
    }
    X87Reg t = (v - at<float>(type, 0xbc)) / (X87Reg(at<float>(type, 0xc0)) - at<float>(type, 0xbc));
    if (!(t.v > 0.0)) t = 0.0f;
    if (!(t.v < 1.0)) t = 1.0f;
    at<float>(car, 0xe8) = (float)t;
    Car_SetChannelRates(at<u32>(owner, 4), car);
}

u32 __cdecl Train_ShrinkToTop(const u8 *owner, const u8 *type, const u8 *train) {
    const u32 count = at<u32>(train, 0x30);
    if (count <= 1) return 0;
    u32 shrunk = 0;
    const u8 *const last = at<const u8 *>(train, 0x30 + count * 4);
    for (s32 i = (s32)(count - 1); i >= 0; --i) {
        u8 *const car = at<u8 *>(train, 0x34 + 4 * (u32)i);
        const X87Reg length = last ? X87Reg(Car_TrainLength(last)) : X87Reg(0.0f);   // (inlined: the same sums)
        if (!(length.v > at<float>(type, 0x6c))) break;
        X87Reg t = (X87Reg(at<float>(type, 0x68)) - at<float>(type, 0xbc)) / (X87Reg(at<float>(type, 0xc0)) - at<float>(type, 0xbc));
        if (!(t.v > 0.0)) t = 0.0f;
        if (!(t.v < 1.0)) t = 1.0f;
        at<float>(car, 0xe8) = (float)t;
        Car_SetChannelRates(at<u32>(owner, 4), car);
        shrunk = 1;
    }
    return shrunk;
}

void __cdecl Train_RestoreSaved(const u8 *owner, u8 *train) {
    for (u32 o = 0; o < 0x48; o += 4) at<u32>(train, o) = global<u32>(0x00790ff0 + o);   // (rep movsd)
    u32 i = 0;
    for (; i < global<u32>(0x00791020); ++i) {
        const u8 *const saved = reinterpret_cast<const u8 *>((uintptr_t)(0x00791d00 + 0x104 * i));
        u8 *const car = at<u8 *>(train, 0x34 + 4 * i);
        at<u32>(car, 4) = at<u32>(saved, 4);
        at<u32>(car, 8) = (at<u32>(car, 8) & 0x600000) | at<u32>(saved, 8);
        constexpr u32 kRuns[][2] = {{0xc, 0x70}, {0x90, 0xb0}, {0xd4, 0x104}};
        for (const auto &run : kRuns)
            for (u32 o = run[0]; o < run[1]; o += 4) at<u32>(car, o) = at<u32>(saved, o);
        at<u32>(car, 0x50) = global<u32>(0x00791038 + 4 * i);
        Car_SetChannelRates(at<u32>(owner, 4), car);
    }
    for (; i < 5; ++i) at<u32>(train, 0x34 + 4 * i) = 0;
    global<u32>(0x00791cd8) = 0;
}

void __cdecl Train_RemoveCar(const u8 *owner, const u8 *car) {
    const auto ride_value = [](const u8 *c) -> float & {   // (the car's ride +8, +4, +0x78, +0x44)
        const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[at<u32>(c, 0x70)];
        return at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 4), 0x78), 0x44);
    };
    u8 *const train = at<u8 *>(car, 0x100);
    const u32 n = --at<u32>(train, 0x30);
    u32 i = at<u32>(car, 4);
    if (i >= n) {
        at<u32>(train, 0x34 + at<u32>(train, 0x30) * 4) = 0;
        return;
    }
    do {
        if (i == at<u32>(car, 4)) {   // (read again each time)
            const float kept = ride_value(at<const u8 *>(train, 0x34 + 4 * i));
            float below = 0.0f;
            if (i)
                below = (float)(X87Reg(Car_TrainLength(at<const u8 *>(train, 0x30 + 4 * i))) + at<float>(at<const u8 *>(train, 0x18), 0xc4));
            const double length = Car_TrainLength(at<const u8 *>(train, 0x38 + 4 * i));
            ride_value(at<const u8 *>(train, 0x38 + 4 * i)) = kept;
            const double t = CarType_Unlerp((float)(X87Reg(length) - below), at<const u8 *>(train, 0x18));
            u8 *const next = at<u8 *>(train, 0x38 + 4 * i);
            at<float>(next, 0xe8) = (float)t;
            if (!(t >= 0.0))   // (fcomp 0x6fe1b4 of the unrounded value: below or unordered)
                at<float>(next, 0xe8) = 0.0f;
            if (at<float>(next, 0xe8) > 1.0f)   // (0x6fe1a8)
                at<float>(next, 0xe8) = 1.0f;
            Car_SetChannelRates(at<u32>(owner, 4), at<u8 *>(train, 0x38 + 4 * i));
        }
        u8 *const next = at<u8 *>(train, 0x38 + 4 * i);
        at<u8 *>(train, 0x34 + 4 * i) = next;
        --at<u32>(next, 4);
        ++i;
    } while (i < at<u32>(train, 0x30));
    at<u32>(train, 0x34 + at<u32>(train, 0x30) * 4) = 0;
}

u8 *__cdecl Ride_Train(u32 index, u32 *cars) {
    const u8 *ride = reinterpret_cast<const u8 *const *>(0x007a4610)[index];
    if (!(at<u32>(ride, 4) & 0x80000)) {
        *cars = 0;
        return nullptr;
    }
    u8 *train = at<u8 *>(at<const u8 *>(ride, 0xd4), 0x100);
    u32 n = at<u32>(train, 0x30);
    if (at<u8>(train, 4) & 0x20) --n;
    *cars = n;
    return train;
}

u32 __cdecl Train_CarValue(const u8 *train, u32 n) {
    if (!(at<u8>(train, 4) & 0x20)) return at<u32>(at<const u8 *>(train, 0x34 + 4 * n), 0x70);
    u32 seen = 0;
    for (u32 i = 0, count = at<u32>(train, 0x30); i < count; ++i) {
        const u8 *car = at<const u8 *>(train, 0x34 + 4 * i);
        if (at<u32>(car, 8) & 0x2000) continue;
        if (seen == n) return at<u32>(car, 0x70);
        ++seen;
    }
    return 0;
}

u32 *__cdecl CoasterEvent_Make(u32 a, u32 b, u32 c, u32 d) {
    u32 *pair = reinterpret_cast<u32 *>(0x00791050), *event = reinterpret_cast<u32 *>(0x00791ce8);
    pair[0] = a;
    pair[1] = b;
    event[0] = c;
    event[1] = d;
    event[2] = 0;
    event[3] = 0x00791050;
    return event;
}

double __cdecl CarType_Unlerp(float length, const u8 *type) {
    const float shortest = at<float>(type, 0xbc);
    return ((X87Reg(length) - shortest) / (X87Reg(at<float>(type, 0xc0)) - shortest)).v;
}

double __cdecl CarType_Scale(float length, const u8 *type) {
    return X87_DivR(X87Reg(at<float>(type, 0xc0)) - at<float>(type, 0xbc), length).v;
}

double __cdecl CarType_Lerp(float t, const u8 *type) {
    const float shortest = at<float>(type, 0xbc);
    return ((X87Reg(at<float>(type, 0xc0)) - shortest) * t + shortest).v;
}

void __cdecl Flags_SetChanged(u32 *flags, u32 also) {
    *flags |= 1;
    if (also) *flags |= 0x1000;
}

void __cdecl Flags_SetMoved(u32 *flags, u32 high) { *flags |= high ? 0x80000010 : 0x10; }

void __cdecl Object_GetPosition(const u8 *object, u32 *out) {
    out[0] = at<u32>(object, 0x30);
    out[1] = at<u32>(object, 0x34);
    out[2] = at<u32>(object, 0x38);
}

double __cdecl Car_TravelCost(const u8 *ride, u32 withBase, const u8 *car, const float *x, const float *z,
                              const float *fromX, const float *fromZ) {
    const u8 *owner = car ? at<const u8 *>(car, 0x100) : nullptr;
    const u8 *type = owner ? at<const u8 *>(owner, 0x18) : nullptr;
    if (!type) return 0.0;
    const float x0 = fromX ? *fromX : at<float>(at<const u8 *>(car, 0x68), 0x4c);
    const float z0 = fromZ ? *fromZ : at<float>(at<const u8 *>(car, 0x68), 0x54);
    const u8 *next = at<const u8 *>(owner, 0x2c);
    const u8 *other = next ? at<const u8 *>(next, 0x18) : nullptr;
    const u8 *fallback = at<const u8 *>(ride, 4) + 0xa8;
    const X87Reg speed = X87Reg(CarType_Lerp(at<float>(car, 0xe8), type)) * 0.1f;
    const X87Reg dx = X87Reg(x ? *x : at<float>(car, 0x4c)) - x0;
    const X87Reg dz = X87Reg(x ? *z : at<float>(car, 0x54)) - z0;
    const X87Reg sum = dx * dx + dz * (float)dz.v;
    const X87Reg dist = X87Reg(sum.v != sum.v ? sum.v : X87_Round(std::sqrt(sum.v))) * 0.1f;
    const X87Reg base = withBase ? at<float>(type, 0x5c) : 0.0f;
    const u8 *s = type;
    if (at<u32>(type, 0) & 0xc000) {
        if (!other || !(at<u32>(other, 0) & 0xc000)) s = fallback;
        if ((at<u32>(s, 0) & 0x4000) && !(at<u8>(at<const u8 *>(car, 0x68), 8) & 0x80)) s = fallback;
    }
    return ((X87Reg(at<float>(s, 0x60)) * dist + base) + X87Reg(at<float>(s, 0x64)) * speed).v;
}

void __cdecl Car_SetMoveCostChange(const u8 *car, const float *cost, const float *extra, const float *x, const float *z, u32 a, u32 b) {
    const u8 *const ride = global<const u8 *>(0x00790fec);
    if (!ride || !global<u32>(0x0079c640) || global<u32>(0x00785a2c)) return;
    s32 before = (s32)(long long)*cost;   // __ftol
    if (extra) before += (s32)(long long)*extra;
    s32 after = Float_RoundUp((float)Car_TravelCost(ride, 0, car, x, z, nullptr, nullptr));
    if (extra)
        after += Float_RoundUp((float)Car_TravelCost(global<const u8 *>(0x00790fec), 0, at<const u8 *>(car, 0x64), nullptr, nullptr, x, z));
    if (global<u32>(0x0079c6d8) == a && global<u32>(0x0079c6a0) == b) {
        global<s32>(0x0079c69c) = 0;
        return;
    }
    global<s32>(0x0079c69c) = after - before;
}

void __cdecl Car_SetupCoupling(u8 *car, u32 *coupling, const u8 *spec, const u8 *type) {
    coupling[0] = at<u32>(spec, 0xc);
    u8 *const first = static_cast<u8 *>(GlobalMem_Alloc(coupling[0] * 0x58, 1));
    coupling[1] = (u32)(uintptr_t)first;
    u8 *const second = static_cast<u8 *>(GlobalMem_Alloc(coupling[0] * 0x58, 1));
    coupling[2] = (u32)(uintptr_t)second;
    const u8 *const holder = reinterpret_cast<u8 *const *>(0x007ae6a8)[at<u32>(type, 0x14)];
    for (u32 i = 0; i < coupling[0]; ++i) {
        const u32 part = Model_FindPartByKey4(holder, 0x400000, at<u32>(spec, 0x10) + i);
        at<u32>(first + i * 0x58, 4) = part;
        at<u32>(second + i * 0x58, 4) = part;
        at<u8 *>(first + i * 0x58, 0x10) = car;
        at<u8 *>(second + i * 0x58, 0x10) = car;
    }
}

void __cdecl Car_FreeCoupling(const u8 *car, u32 *coupling) {
    if (!(at<u8>(at<const u8 *>(car, 0x100), 4) & 1)) GlobalMem_Free(reinterpret_cast<void *>(coupling[1]));
    GlobalMem_Free(reinterpret_cast<void *>(coupling[2]));
    coupling[1] = 0;
    coupling[2] = 0;
}
}

namespace {

// A car's train length stored as a float (0 for no car). 0x433300 has Car_TrainLength inline,
// adding the car's own length to the sum the other way round (fadd st0, st1, not faddp): only
// a NaN pair tells the two apart, and a NaN length only ever reaches comparisons there.
float CarLength(const u8 *car) { return (float)Car_TrainLength(car); }

}  // namespace

extern "C" {

u32 __cdecl Pylon_FindBraceLevel(float wanted, const u8 *train, u32 *level, float *height, u32) {
    const u8 *type = at<const u8 *>(train, 0x18);
    const float base = at<float>(type, 0x68), top = at<float>(type, 0x6c), gap = at<float>(type, 0xc4);
    u32 wantBits;
    std::memcpy(&wantBits, &wanted, 4);
    if (!(top > wanted)) {  // (fld; fstp: a signalling NaN comes out quiet)
        u32 topBits;
        std::memcpy(&topBits, &top, 4);
        wantBits = X87_FloatThrough(topBits);
    }
    float want;
    std::memcpy(&want, &wantBits, 4);
    const u32 count = at<u32>(train, 0x30);
    if (!(global<u32>(0x0079c658) & 0x100) && count && (at<u8>(at<const u8 *>(train, 0x34), 8) & 0x40)) {
        const float length = CarLength(at<const u8 *>(train, 0x30 + 4 * count));
        if (!(((X87Reg(length) + base) + gap).v >= top)) {  // below the top, or unordered
            *level = count;
            const X87Reg spare = at<u32>(train, 0x30) ? X87Reg(gap) : X87Reg(0.0f);
            const X87Reg start = X87Reg(length) + base;
            const float stored = (float)start;
            const X87Reg h = (start + spare).v > want ? spare + stored : X87Reg(want);
            *height = (float)h;
            return 1;
        }
    }
    u32 found = 0, foundLevel = 0;
    float prev = 0.0f;
    for (u32 i = 0; i < at<u32>(train, 0x30); ++i) {
        const u8 *car = at<const u8 *>(train, 0x34 + 4 * i);
        const X87Reg k = i ? 1.0 : 0.0;
        const float cur = CarLength(car);
        const X87Reg need = (k - X87Reg(-1.0f)) * gap - X87Reg(base) * -2.0f;
        const X87Reg room = (X87Reg(cur) - prev) - X87Reg(-0.01f);
        if (!(need.v > room.v)) {
            const X87Reg limit = ((X87Reg(cur) - gap) - base) - X87Reg(-0.01f);
            if (!(want >= limit.v)) {  // below, or unordered: wanted as it is (an integer copy)
                *level = at<u32>(car, 4);  // sic: the car's +4, and a fault for an empty slot
                std::memcpy(height, &wantBits, 4);
                return 1;
            }
            foundLevel = i;
            found = 1;
            *height = (float)((k * gap + prev) + base);
        }
        prev = cur;
    }
    const u32 n = at<u32>(train, 0x30);
    if (!(((X87Reg(prev) + base) + (n ? X87Reg(gap) : X87Reg(0.0f))).v >= top)) {
        *level = n;
        const X87Reg spare = at<u32>(train, 0x30) ? X87Reg(gap) : X87Reg(0.0f);
        const X87Reg start = X87Reg(prev) + base;
        const float stored = (float)start;
        X87Reg h = (start + spare).v > want ? spare + stored : X87Reg(want);
        *height = (float)h;
        if (!(top >= h.v)) h = top;
        *height = (float)h;
        return 1;
    }
    if (found) {
        *level = foundLevel;
        return 1;
    }
    *level = n;
    *height = (float)((X87Reg(gap) + prev) + base);
    return 1;
}

void __cdecl Train_PlaceCars(const u8 *type, u8 *train, u32 force, u32 keep) {
    if (global<u32>(0x00790afc) && !force) return;
    for (u32 i = 0; i < at<u32>(train, 0x30); ++i) {
        u8 *const car = at<u8 *>(train, 0x34 + 4 * i);
        if (!keep) {
            at<u32>(car, 8) |= 0x200000;
            if (u8 *const piece = at<u8 *>(car, 0x68)) at<u32>(piece, 8) |= 0x200000;
        }
        if (at<u32>(car, 8) & 0x2000) continue;
        const u8 *const ride = RideAt(at<u32>(car, 0x70));   // EDI
        if (i != 0) {
            const u8 *const prev = at<const u8 *>(train, 0x30 + 4 * i);
            float base;
            if (at<u32>(prev, 8) & 0x2000) {
                base = (float)(X87Reg(at<float>(prev, 0x50)) + at<float>(at<const u8 *>(train, 0x18), 0xc4));
            } else {
                const u8 *const model = at<const u8 *>(RideAt(at<u32>(prev, 0x70)), 8);
                const u8 *const top = at<const u8 *>(at<const u8 *>(at<const u8 *>(model, 0x28), 4), at<u32>(type, 0x18) * 20 + 4);
                base = top ? X87_Through(at<float>(top, 0x34)) : 0.0f;   // (0x6fde74)
            }
            at<float>(at<u8 *>(at<u8 *>(at<u8 *>(ride, 8), 4), 0x78), 0x44) = base;
            g_trainEnv->animate(RideAt(at<u32>(car, 0x70)), reinterpret_cast<const u8 *>(1), 0);
            if (!(at<u8>(type, 0) & 0x10)) {
                if (u32 *const flags = at<u32 *>(at<u8 *>(ride, 8), 0x2c)) *flags |= 0x80000010;
            }
        } else {
            at<float>(at<u8 *>(at<u8 *>(at<u8 *>(ride, 8), 4), 0x78), 0x44) = X87_Through(at<float>(train, 0x20));
            g_trainEnv->animate(RideAt(at<u32>(car, 0x70)), reinterpret_cast<const u8 *>(1), 0);
            if (u32 *const flags = at<u32 *>(at<u8 *>(ride, 8), 0x2c)) {
                *flags &= ~0x10u;
                Model_FitToGrid(at<const u8 *>(ride, 8), global<const u8 *>(0x007a0854), at<u32>(at<const u8 *>(car, 0x100), 0x20), 0);
            }
        }
        // The car's ride model's frame at the type's index k (+0x1c, +0x20, +0x24, +0x28) past the model's +0x46: the root's frames
        // (+0x70, 0xa0 each) below its +0x44, past them the others (+0x74, 0x58 each).
        const auto frame = [&](u32 k) {
            const u8 *const root = at<const u8 *>(at<const u8 *>(RideAt(at<u32>(car, 0x70)), 8), 4);
            const u32 index = at<u32>(type, k) + at<u16>(root, 0x46);
            const u32 count = at<u16>(root, 0x44);
            return index < count ? reinterpret_cast<u32 *>(at<u8 *>(root, 0x70) + index * 0xa0)
                                 : reinterpret_cast<u32 *>(at<u8 *>(root, 0x74) + (index - count) * 0x58);
        };
        if (i == at<u32>(train, 0x30) - 1) {
            if (at<u32>(type, 0) & 0x400) Flags_SetMoved(frame(0x1c), 1);
            if (at<u32>(type, 0) & 0x800) *frame(0x20) &= ~0x10u;
        } else {
            const u8 *const next = at<const u8 *>(train, 0x38 + 4 * i);
            if ((at<u32>(type, 0) & 0x400) && !(at<u32>(next, 8) & 0x2000)) *frame(0x1c) &= ~0x10u;
            if (at<u32>(type, 0) & 0x800) Flags_SetMoved(frame(0x20), 1);
        }
        if (i == 0) {
            if (at<u32>(type, 0) & 0x1000) Flags_SetMoved(frame(0x24), 1);
            if (at<u32>(type, 0) & 0x2000) *frame(0x28) &= ~0x10u;
        } else {
            if (at<u32>(type, 0) & 0x1000) *frame(0x24) &= ~0x10u;
            if (at<u32>(type, 0) & 0x2000) *frame(0x28) |= 0x80000010;
        }
    }
}
}

void Train_SetEnv(const TrainEnv *e) { g_trainEnv = e ? e : &kGameEnv; }
const TrainEnv *Train_GetEnv() { return g_trainEnv; }
