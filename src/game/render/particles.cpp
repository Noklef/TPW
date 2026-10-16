#include "particles.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *system(u32 slot) { return reinterpret_cast<u8 *>(0x0080cefc + slot * 0x140); }

}  // namespace

extern "C" {

u32 __cdecl Particle_IsAlive(u32 handle) {
    if (handle == 0 || global<u32>(0x00816d40) == 0 || (u16)handle >= 0x78) return 0;
    return *reinterpret_cast<u16 *>(system((u16)handle) + 0xa) == (u16)(handle >> 16) ? 1 : 0;
}

u32 __cdecl ParticleEffector_IsAlive(u32 handle) {
    if (global<u32>(0x00816d40) == 0 || (u16)handle >= 0x14) return 0;
    return global<u16>(0x00816504 + (u16)handle * 0x68) == (u16)(handle >> 16) ? 1 : 0;
}

void __cdecl Particles_SetReduced(u32 reduced) { global<u32>(0x0080ced8) = reduced; }

void __cdecl ParticleEffect_Clear118(u32 effect) {
    if (global<u32>(0x00816d40) != 0 && effect < 0x69) global<u8>(0x00804370 + effect * 0x140 + 0x118) = 0;
}

void __cdecl ParticleSystem_End(u32 handle) {
    if (Particle_IsAlive(handle)) *reinterpret_cast<s32 *>(system((u16)handle) + 0x20) = -2;
}

void __cdecl ParticleSystem_SetAge(u32 handle, u32 age) {
    if (Particle_IsAlive(handle)) *reinterpret_cast<u32 *>(system((u16)handle) + 0x20) = age;
}

void __cdecl ParticleSystem_Set2a(u32 handle, u32 value) {
    if (Particle_IsAlive(handle)) *reinterpret_cast<u16 *>(system((u16)handle) + 0x2a) = (u16)value;
}

void __cdecl ParticleSystem_SetA2(u32 handle, u32 value) {
    if (Particle_IsAlive(handle)) *reinterpret_cast<u16 *>(system((u16)handle) + 0xa2) = (u16)value;
}

s32 __cdecl ParticleSystem_QuarterByte(u32 slot) {
    const u8 *s = system(slot);
    const s32 lifetime = *reinterpret_cast<const s32 *>(s + 0xd4);
    if (lifetime != 0) {
        const s32 quarter = (s32)(*reinterpret_cast<const u32 *>(s + 0x20) << 2) / lifetime;
        if (quarter == 0) return (s8)s[0x6b];
        if (quarter == 1) return (s8)s[0x6a];
        if (quarter == 2) return (s8)s[0x69];
    }
    return (s8)s[0x68];
}
}
