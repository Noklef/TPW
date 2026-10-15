#include "train.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

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
}
