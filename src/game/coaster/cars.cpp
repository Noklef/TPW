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
}
