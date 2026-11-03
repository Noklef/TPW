#include "randomsegments.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vdtor.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The next number's low 30 bits.
u32 Draw() {
    u32 &state = at<u32>(global<u8 *>(0x00f82884), 0x34);
    const u32 next = state * 0x19660du + 0x3c6ef35fu;
    state = next >> 13 | next << 19;
    return state & 0x3fffffffu;
}

float Axis(const u8 *object, u32 centre, u32 size) {
    float scale = 9.313225746154785e-10f, half = 0.5f;   // [0x702c58], [0x702c5c]
    const X87Reg drawn = X87Reg((double)Draw()) * scale * at<float>(object, size);
    const X87Reg middle = X87Reg(at<float>(object, size)) * half;
    return (float)((drawn - middle) + at<float>(object, centre));
}

// fcom's C3: equal or unordered.
bool isZero(float v) { return !((double)v < 0.0) && !((double)v > 0.0); }

// Start at +start, end 0xc on, direction 0x18 on.
void Segment(u8 *object, u32 start) {
    for (u32 i = 0; i < 6; ++i) at<float>(object, start + i * 4) = Axis(object, 0x58 + i % 3 * 4, 0x64 + i % 3 * 4);
    float *const d = &at<float>(object, start + 0x18);
    for (u32 i = 0; i < 3; ++i) d[i] = (float)(X87Reg(at<float>(object, start + 0xc + i * 4)) - at<float>(object, start + i * 4));
    if (isZero(d[0]) && isZero(d[1]) && isZero(d[2])) {
        at<u32>(d, 0) = 0x3f800000u;
        at<u32>(d, 4) = 0;
        at<u32>(d, 8) = 0;
    }
    const X87Reg x(d[0]), y(d[1]), z(d[2]);
    const X87Reg length = X87_Sqrt(x * x + y * y + z * z);
    for (u32 i = 0; i < 3; ++i) d[i] = (float)(X87Reg(d[i]) / length);
}

}  // namespace

extern "C" {

u8 *__fastcall RandomSegments_Ctor(u8 *object) {
    Class702d48_Ctor(object);
    at<u32>(object, 0) = 0x00702cb0;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (block) {
        at<u32>(block, 0) = 0x00702cac;
        at<u32>(block, 4) = 0;
        at<u32>(block, 8) = 0;
        at<u32>(block, 0xc) = 0;
    }
    at<u8 *>(object, 8) = block;
    for (u32 i = 0xc; i < 0x28; i += 4) at<u32>(object, i) = 0;
    const u8 *const source = global<const u8 *>(0x00f83778);
    for (u32 i = 0; i < 0xc; i += 4) {
        at<u32>(object, 0x34 + i) = at<u32>(source, 8 + i);
        at<u32>(object, 0x28 + i) = at<u32>(source, 8 + i);
        at<u32>(object, 0x4c + i) = at<u32>(source, 0x14 + i);
        at<u32>(object, 0x40 + i) = at<u32>(source, 0x14 + i);
    }
    at<u32>(object, 0x58) = 0x43fa0000u;   // 500.0
    at<u32>(object, 0x5c) = 0x42960000u;   // 75.0
    at<u32>(object, 0x60) = 0x43fa0000u;
    at<u32>(object, 0x64) = 0x43c80000u;   // 400.0
    at<u32>(object, 0x68) = 0x42480000u;   // 50.0
    at<u32>(object, 0x6c) = 0x43c80000u;
    at<u32>(object, 0x70) = 0x3f800000u;   // 1.0
    at<u32>(object, 0x74) = 0x42c80000u;   // 100.0
    Segment(object, 0x78);
    at<u32>(object, 0xa8) = 0x40000000u;   // 2.0
    at<u32>(object, 0xac) = 0x40000000u;
    at<u32>(object, 0xb0) = 0x42480000u;   // 50.0
    Segment(object, 0xb4);
    return object;
}
}
