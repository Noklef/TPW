#include "cars.h"

#include <cmath>
#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
const float *position(const u8 *car) { return &at<const float>(car, 0x4c); }

const float kPi = 3.14159274f;  // 0x6fe028, and its negation at 0x6fe02c

}  // namespace

extern "C" {

double __cdecl CoasterCar_SwingRate(const u8 *coaster, const u8 *car, float x) {
    const float angle = at<float>(car, 0x94);
    if (!(angle < 0.0f || angle > 0.0f)) return 0.0;  // fcomp 0.0; test ah,0x40
    if (at<u32>(car, 8) & 0x100) return 0.0;
    const float behind = (float)Vec3_Distance(position(at<const u8 *>(car, 0x68)), position(car));
    const X87Reg both = X87Reg(Vec3_Distance(position(at<const u8 *>(car, 0x64)), position(car))) + behind;
    const X87Reg swing =
        (X87Reg(angle < 0.0f ? -kPi : kPi) - angle) * x / at<float>(at<const u8 *>(coaster, 4), 0x10);
    return X87_DivR(both, swing).v;
}

void __cdecl Coaster_SpaceCars(u8 *coaster) {
    const float gap = (float)(X87Reg((double)at<u32>(at<const u8 *>(coaster, 4), 0x2ec)) * 0.001f);
    const float scale = at<float>(coaster, 0xac);
    const u32 count = at<u32>(coaster, 0xf4);
    u8 *const cars = at<u8 *>(coaster, 0x128);
    for (u32 i = 0; i < count; ++i) {
        u8 *car = cars + i * 0x80;
        const u32 k = at<u32>(car, 8) + at<u32>(car, 0xc);
        float &p = at<float>(car, 0x30);
        p = X87_Through(at<float>(at<u8 *>(car, 0x28), k * 0x60 - 0x20));
        if (!(p >= 0.0f)) p = (float)(X87Reg(at<float>(coaster, 0xa8)) + p);
    }
    u32 next = 1 % count;  // no cars: divide by zero, as the original
    const u8 *before = cars;
    at<u8 *>(coaster, 0x124) = cars;
    for (u32 left = count - 1; left; --left) {
        u8 *car = cars + next * 0x80;
        const u8 *nodes = at<const u8 *>(coaster, 0x9c);
        double whole;
        const double frac = std::modf((double)at<float>(before, 0x30), &whole);
        u32 node = (u32)(long long)whole;
        X87Reg last = 0.0;
        X87Reg sum = (X87Reg(1.0f) - frac) * scale / at<float>(nodes, node * 0x38 + 0x1c);
        while (!(sum.v >= gap)) {  // sic: forever on a NaN
            node = (node + 1) % at<u32>(coaster, 0xa4);
            last = sum;
            sum = sum + X87Reg(scale) / at<float>(nodes, node * 0x38 + 0x1c);
        }
        const X87Reg w = (sum - gap) / (sum - last);
        const float x = (float)(X87Reg((double)(int)(node - 1)) * (X87Reg(1.0f) - w) + X87Reg((double)(int)node) * w);
        const X87Reg length = at<float>(coaster, 0xa8);
        at<float>(car, 0x30) = (float)std::fmod((length + x).v, length.v);
        next = (next + 1) % count;
        before = car;
    }
}

void __cdecl CoasterCar_SetSegmentOffsets(const u8 *coaster, u8 *car) {
    const u8 *type = at<const u8 *>(car, 4);
    const auto segment = [car](s32 i) { return at<u8 *>(car, 0x28) + i * 0x60; };
    const auto length = [car, type](const u8 *seg) {
        const u32 i = at<u32>(seg, 8);
        return at<float>(type, i == 0 ? 0x1c : i == at<u32>(car, 0xc) - 1 ? 0x24 : 0x20);
    };
    const auto place = [coaster](u8 *seg, X87Reg offset) {
        at<float>(seg, 0x38) = (float)offset;
        at<float>(seg, 0x3c) = (float)(offset / at<float>(coaster, 0xac));
    };
    const s32 mid = at<s32>(car, 0x2c);
    at<u32>(segment(mid), 0x38) = 0;
    at<u32>(segment(mid), 0x3c) = 0;
    X87Reg sum = 0.0f;
    for (s32 i = (s32)((u32)mid - 1); i >= 0; --i) {
        u8 *seg = segment(i);
        sum = sum + length(seg);
        place(seg, sum);
    }
    sum = 0.0f;
    for (s32 i = (s32)((u32)mid + 1); i < at<s32>(car, 0xc); ++i) {
        u8 *seg = segment(i);
        sum = sum - length(seg);
        place(seg, sum);
    }
}

void __cdecl Coaster_SetSegmentWindow(u8 *coaster) {
    const auto value = [](const u8 *car) { return at<float>(at<const u8 *>(at<const u8 *>(car, 0x90), 4), 8); };
    const u8 *car = at<const u8 *>(coaster, 0x54);
    const float a = value(car), b = value(at<const u8 *>(at<const u8 *>(car, 0x64), 0x64));
    const float k = at<float>(at<const u8 *>(coaster, 4), 0x31c);
    const X87Reg mid = (X87Reg(b) + a) * 0.5f, near = X87Reg(k) * mid;
    at<float>(coaster, 0xf8) = (float)(near + (X87Reg(1.0f) - k) * a);
    at<float>(coaster, 0xfc) = (float)(near + (X87Reg(1.0f) - k) * b);
}

void __cdecl CoasterCar_FindSegmentsInWindow(const u8 *coaster, const u8 *car, s32 *first, s32 *last) {
    const float scale = at<float>(coaster, 0xac);
    const u8 *seg = at<const u8 *>(car, 0x28) + at<u32>(car, 8) * 0x60;
    const u8 *found = nullptr, *final = nullptr;
    for (u32 n = at<u32>(car, 0xc); n != 0; --n, seg += 0x60) {
        const u8 *type = at<const u8 *>(seg, 4);
        const X87Reg ahead = X87Reg(at<float>(type, 0x24)) / scale, behind = X87Reg(at<float>(type, 0x28)) / scale;
        const float at40 = at<float>(seg, 0x40);
        if (!((ahead + at40).v >= at<float>(coaster, 0xf8))) continue;
        if (X87_SubR(behind, at40).v > at<float>(coaster, 0xfc)) continue;
        if (!found) found = seg;
        final = seg;
    }
    const uintptr_t base = (uintptr_t)at<const u8 *>(car, 0x28);
    *first = (s32)((uintptr_t)found - base) / 0x60;
    *last = (s32)((uintptr_t)final - base) / 0x60;
}

void __cdecl CoasterCar_SetExtent(const u8 *coaster, u8 *car) {
    const float scale = at<float>(coaster, 0xac);
    const u8 *front = at<const u8 *>(car, 0x28) + at<u32>(car, 8) * 0x60;
    const u8 *back = front + at<u32>(car, 0xc) * 0x60 - 0x60;
    const auto offset = [scale, car](const u8 *seg) { return X87Reg(at<float>(seg, 0x38)) / scale + at<float>(car, 0x74); };
    const auto length = [scale](const u8 *seg, u32 at_) { return X87Reg(at<float>(at<const u8 *>(seg, 4), at_)) / scale; };
    at<float>(car, 0x20) = (float)(offset(front) + length(front, 0x24));
    at<float>(car, 0x24) = (float)(offset(back) - length(back, 0x28));
}

double __cdecl Coaster_SpeedAfterDrop(const u8 *params, float speed, float minimum, float fromY, float toY) {
    const X87Reg drop = (X87Reg(fromY) - toY) * at<float>(params, 0x24);
    const X87Reg next = drop * at<float>(params, toY > fromY ? 0x28 : 0x2c) + speed;
    return next.v > minimum ? next.v : minimum;
}

void __cdecl Coaster_SetupCars(u8 *coaster, u32 riders, u32 segments, u32 flags) {
    u8 *car = at<u8 *>(coaster, 0x128);
    for (u32 i = 0; i < at<u32>(at<const u8 *>(coaster, 4), 0x2e8); ++i, car += 0x80) {
        if ((at<u8>(car, 0) & 4) || (segments != 0 && segments != at<u32>(car, 0xc))) {
            if (segments) at<u32>(car, 0xc) = segments;
            const u32 count = at<u32>(car, 0xc);
            at<u32>(car, 8) = 0;
            at<u32>(car, 0x2c) = (count - 1) >> 1;
            if (count) {
                CoasterCar_SetSegmentOffsets(coaster, car);
                CoasterCar_SetExtent(coaster, car);
            }
        }
        u8 *seg = at<u8 *>(car, 0x28);
        if (!(flags & 1)) continue;
        if (!riders) riders = at<u32>(at<const u8 *>(car, 4), 8);
        at<u32>(coaster, 0xf0) = riders;
        const u8 *type = at<const u8 *>(car, 4);
        u32 seats = at<u32>(car, 0xc);
        if (at<u32>(type, 0xc) != at<u32>(type, 0x10)) --seats;
        if (at<u32>(type, 0x14) != at<u32>(type, 0x10)) --seats;
        u32 left = riders, seated = 0;
        for (u32 j = 0; j < at<u32>(car, 0xc); ++j, seg += 0x60) {
            if (at<u32>(seg, 4) != at<u32>(at<const u8 *>(car, 4), 0x10)) {
                at<u32>(seg, 0x44) = 0;
                continue;
            }
            const u32 share = left / (seats - seated++);
            left -= share;
            at<u32>(seg, 0x44) = share;
        }
    }
    at<u32>(coaster, 0xb8) = at<u32>(coaster, 0xf0);
}

void __cdecl Coaster_SetRiders(u32 index, s32 n) {
    u8 *coaster = reinterpret_cast<u8 **>(0x00790bd0)[index];
    if (n < 0) n = 0;
    const s32 most = at<s32>(at<const u8 *>(coaster, 4), 0x318);
    if (most < n) n = most;
    const s32 seats = at<s32>(at<const u8 *>(at<const u8 *>(coaster, 0x128), 4), 8);
    if (seats < n) n = seats;
    if ((u32)n != at<u32>(coaster, 0xf0)) Coaster_SetupCars(coaster, (u32)n, 0, 1);
}

void __cdecl CoasterCars_InitUnusedOne() { *reinterpret_cast<float *>(0x007923a0) = 1.0f; }

void __cdecl CoasterCars_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007923a4) = 1.0f / 8388608.0f; }
}
