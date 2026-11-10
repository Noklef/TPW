// A lobby model set flying between two random points: see lobbyflight.h.
#include "lobbyflight.h"

#include <cstdint>
#include <cstdlib>

#include "../core/vcall.h"
#include "../math/x87.h"
#include "frame.h"

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

const float kPerRange = 9.31322575e-10f, kHalf = 0.5f;   // 0x702adc, 0x702ae0

// The centre's component with a random part of the spread's about it.
float Scatter(const float *centre, const float *spread, u32 i) {
    u32 &seed = at<u32>(global<u8 *>(0x00f82884), 0x34);
    seed = _rotr(seed * 0x19660d + 0x3c6ef35f, 13);
    return (float)(X87Reg((double)(seed & 0x3fffffff)) * kPerRange * spread[i] - X87Reg(spread[i]) * kHalf + centre[i]);
}

}  // namespace

void LobbyFlight_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u8 *__fastcall LobbyFlight_Ctor(u8 *self, void *, u8 *owner, u32 a, u32 b, const u32 *centre, const u32 *spread, u32 e, u32 f) {
    at<u32>(self, 0) = (u32)(uintptr_t)Class702ae8_vtable;
    at<u8 *>(self, 4) = owner;
    const u32 model = g_calls.Th(0x005d8870, reinterpret_cast<u32 &>(gFrame_f83774), a, b, 0u, 1u, 0u, 0u);
    at<u32>(self, 8) = model;
    g_calls.Th(0x005d83f0, model, 0u, 1u);
    at<u32>(self, 0xc) = 0;
    u32 *const fields = reinterpret_cast<u32 *>(self);
    for (u32 i = 0; i < 3; ++i) {
        fields[4 + i] = centre[i];
        fields[7 + i] = spread[i];
    }
    at<u32>(self, 0x28) = e;
    at<u32>(self, 0x2c) = f;
    const float *const near_ = reinterpret_cast<const float *>(self + 0x10), *const far_ = reinterpret_cast<const float *>(self + 0x1c);
    float *const v = reinterpret_cast<float *>(self);
    for (u32 i = 0; i < 3; ++i) v[12 + i] = Scatter(near_, far_, i);
    for (u32 i = 0; i < 3; ++i) v[15 + i] = Scatter(near_, far_, i);
    for (u32 i = 0; i < 3; ++i) v[18 + i] = (float)(X87Reg(v[15 + i]) - v[12 + i]);
    const X87Reg length = X87_Sqrt(X87Reg(v[18]) * X87Reg(v[18]) + X87Reg(v[19]) * X87Reg(v[19]) + X87Reg(v[20]) * X87Reg(v[20]));
    for (u32 i = 0; i < 3; ++i) v[18 + i] = (float)(X87Reg(v[18 + i]) / length);
    g_calls.Th(0x005d8190, model, self + 0x30);   // LobbyModel_SetPosition
    vcall<void>(owner, 4, self);
    return self;
}
}
