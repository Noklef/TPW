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

void __cdecl ParticleSystem_SetVectorA(u32 handle, s32 x, s32 y, s32 z) {
    if (!Particle_IsAlive(handle))
        return;
    s32 *v = reinterpret_cast<s32 *>(system((u16)handle) + 0x14);
    v[0] = x >> 4;
    v[1] = y >> 4;
    v[2] = z >> 4;
}

void __cdecl ParticleSystem_SetVectorB(u32 handle, s32 x, s32 y, s32 z) {
    if (!Particle_IsAlive(handle))
        return;
    s32 *v = reinterpret_cast<s32 *>(system((u16)handle) + 0x44);
    v[0] = x >> 4;
    v[1] = y >> 4;
    v[2] = z >> 4;
}

void __cdecl ParticleSystem_SetScaled(u32 handle, s32 x, s32 y, s32 z) {
    if (!Particle_IsAlive(handle))
        return;
    u8 *s = system((u16)handle);
    const s32 scale = *reinterpret_cast<s32 *>(s + 0xb0);
    s32 *v = reinterpret_cast<s32 *>(s + 0x38);
    const s32 in[3] = {x, y, z};
    for (u32 i = 0; i < 3; i++) {
        const s32 product = (s32)((u32)in[i] * (u32)scale);
        v[i] = (s32)((u32)product + ((u32)(product >> 31) & 0x3ff)) >> 10;
    }
}

void __cdecl Particles_EndAll() {
    if (global<u32>(0x00816d40) == 0)
        return;
    for (s32 slot = global<s16>(0x00816d1c); slot >= 0;) {
        u8 *s = system((u32)slot);
        if (s[5]) {
            *reinterpret_cast<s32 *>(s + 0x20) = -2;
            *reinterpret_cast<u16 *>(s + 0x2a) = 1;
        }
        slot = *reinterpret_cast<s16 *>(s + 0xd0);
    }
    for (s32 slot = global<s16>(0x00816d1e); slot >= 0;) {
        u8 *e = reinterpret_cast<u8 *>(0x008164fc + (u32)slot * 0x68);
        if (e[0x45])
            *reinterpret_cast<s32 *>(e + 0x4c) = -2;
        slot = *reinterpret_cast<s16 *>(e + 0x60);
    }
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
