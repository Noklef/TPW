#include "soundset.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void set(u32 address, u32 value) {
    global<u32>(0x00803ac8) = 1;
    global<u32>(address) = value;
}

}  // namespace

extern "C" {

void __cdecl Sound_Set803ab8(u32 value) { set(0x00803ab8, value); }
void __cdecl Sound_Set803ac0(u32 value) { set(0x00803ac0, value); }
void __cdecl Sound_Set803abc(u32 value) { set(0x00803abc, value); }
void __cdecl Sound_Set803ac4(u32 value) { set(0x00803ac4, value); }
void __cdecl Sound_Set803adc(u32 value) { set(0x00803adc, value); }
void __cdecl Sound_Set803ad8(u32 value) { set(0x00803ad8, value); }
void __cdecl Sound_Set803ae0(u32 value) { set(0x00803ae0, value); }
}
