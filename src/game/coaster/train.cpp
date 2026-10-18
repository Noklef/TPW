#include "train.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../math/x87.h"
#include "../model/parts.h"
#include "helpers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(uintptr_t address) { return *reinterpret_cast<T *>(address); }

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
