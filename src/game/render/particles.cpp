#include "particles.h"

#include <cstdint>

#include "../core/voices.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *system(u32 slot) { return reinterpret_cast<u8 *>(0x0080cefc + slot * 0x140); }

template <typename T> T &field(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

// The sine table's (at [0x80ceec], 0x200 entries) entry for an angle in eighths of an entry (divided truncating toward 0); quarter 0x80: the cosine.
s32 Sine(s32 angle, s32 quarter) { return global<s32 *>(0x0080ceec)[(u32)(angle / 8 + quarter) & 0x1ff]; }

// v times a sine entry (wrapping) over 256 (arithmetic shift).
s32 Scale(s32 sine, s32 v) { return (s32)((u32)sine * (u32)v << 2) >> 10; }

// pos's x and z turned by angle (eighths of a sine entry): x cos - z sin and z cos + x sin, wrapping.
void Rotate(s32 *pos, s32 angle) {
    const u32 sine = (u32)Sine(angle, 0) << 2, cosine = (u32)Sine(angle, 0x80) << 2;
    const u32 x = (u32)pos[0], z = (u32)pos[2];
    pos[0] = (s32)(cosine * x - sine * z) >> 10;
    pos[2] = (s32)(cosine * z + sine * x) >> 10;
}

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

// The next seed.
static s32 NextSeed() {
    u32 &seed = global<u32>(0x0080cee0);
    seed = seed * 214013u + 2531011u;
    return (s32)seed;
}

s32 __cdecl Particle_RandSigned(s32 n) {
    if (n == 0)
        return 0;
    return (NextSeed() >> 16) % n;
}

s32 __cdecl Particle_Rand(s32 n) {
    if (n == 0)
        return 0;
    return ((NextSeed() >> 16) & 0x7fff) % n;
}

void __cdecl ParticleSystem_Release(s32 slot) {
    if (slot < 0)
        return;
    u8 *s = system((u32)slot);
    const s16 prev = *reinterpret_cast<s16 *>(s + 0xd2), next = *reinterpret_cast<s16 *>(s + 0xd0);
    if (global<s16>(0x00816d1c) == slot)
        global<s16>(0x00816d1c) = next;
    if (prev >= 0)
        *reinterpret_cast<s16 *>(system((u32)prev) + 0xd0) = *reinterpret_cast<s16 *>(s + 0xd0);
    if (next >= 0)
        *reinterpret_cast<s16 *>(system((u32)next) + 0xd2) = *reinterpret_cast<s16 *>(s + 0xd2);
    *reinterpret_cast<u16 *>(s) = 0;
    *reinterpret_cast<s16 *>(s + 0xd0) = global<s16>(0x00816d20);
    *reinterpret_cast<u16 *>(s + 0xd2) = 0xffff;
    *reinterpret_cast<u16 *>(s + 0xa) = 0;
    global<s16>(0x00816d20) = (s16)slot;
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

void __cdecl Particle_Move(u8 *p) {
    s32 *const pos = reinterpret_cast<s32 *>(p + 0x14);
    s32 *const vel = reinterpret_cast<s32 *>(p + 0x2c);
    const s32 drag = *reinterpret_cast<const s16 *>(p + 0x24);
    const auto damp = [drag](s32 v) { return (s32)((u32)v - (u32)((s32)((u32)v * (u32)drag) / 1024)); };
    const s32 vx = damp(vel[0]);
    vel[0] = vx;
    const s32 vy = (s32)((u32)damp(vel[1]) - (u32)(s32)*reinterpret_cast<const s16 *>(p + 0x28));
    vel[1] = vy;
    const s32 vz = damp(vel[2]);
    vel[2] = vz;
    pos[0] = (s32)((u32)pos[0] + (u32)vx);
    pos[1] = (s32)((u32)pos[1] + (u32)vy);
    pos[2] = (s32)((u32)pos[2] + (u32)vz);
    if (*reinterpret_cast<const s16 *>(p + 0x76) == 0) return;
    const auto ground = global<s32(__cdecl *)(s32, s32)>(0x0080cef4);
    const s32 h = ground((s32)((u32)pos[0] << 4), (s32)((u32)pos[2] << 4)) >> 4;
    const s32 y = pos[1];
    if (y >= h) return;
    const s32 v = vel[1];
    if ((s32)((u32)y - (u32)h) < v) {  // (falling faster than it is below the ground: stepped back)
        pos[0] = (s32)((u32)pos[0] - (u32)vel[0]);
        pos[1] = (s32)((u32)y - (u32)v);
        vel[0] = -(vel[0] / 2);
        vel[1] = v / 2;
        vel[2] = -(vel[2] / 2);
    } else {
        pos[1] = h;
        vel[1] = -(v / 2);
        vel[0] = vel[0] / 2;
        vel[2] = vel[2] / 2;
    }
}

s32 __cdecl Particle_Emit(u32 slot) {
    const s32 index = FreeList_Take((s32)slot);
    if (index < 0)
        return index;
    u8 *const s = system(slot);
    u8 *const p = global<u8 *>(0x0080cef8) + index * 0x34;
    ++global<u32>(0x0080ced0);
    ++field<u16>(s, 8);
    field<u16>(p, 0x24) = 2;
    field<u16>(p, 0x26) = (u16)slot;
    field<u16>(p, 0x28) = field<u16>(s, 0x74);
    field<s16>(p, 0x2a) = (s8)s[4];
    const s32 spread = Particle_RandSigned(field<s32>(s, 0x78) >> 2);
    field<u16>(p, 0x2c) = 0;
    const s32 life = (s32)((u32)field<s32>(s, 0x78) + (u32)spread);
    field<s32>(p, 0x20) = life;
    field<s32>(p, 0x1c) = life != 0 ? life : 1;
    field<u16>(p, 0x2e) = s[0x6d] != 0 ? (u16)Particle_Rand(0x8000) : 0;
    field<u32>(p, 0x18) = field<u32>(s, 0x58) != 0 ? field<u32>(s, 0xd8 + Particle_Rand(16) * 4) : field<u32>(s, 0x114);

    // The position: the system's own with no spawn volume, else a point of the volume about it
    s32 *const pos = reinterpret_cast<s32 *>(p);
    if (field<s32>(s, 0x44) == 0 && field<s32>(s, 0x48) == 0 && field<s32>(s, 0x4c) == 0) {
        pos[0] = field<s32>(s, 0x14);
        pos[1] = field<s32>(s, 0x18);
        pos[2] = field<s32>(s, 0x1c);
    } else {
        const auto height = [s] { return field<u16>(s, 0xa0) != 0 ? Particle_Rand(field<s32>(s, 0x48)) : Particle_RandSigned(field<s32>(s, 0x48)); };
        if (s[0xc2] != 0) {  // a box
            pos[0] = Particle_RandSigned(field<s32>(s, 0x44));
            pos[1] = height();
            pos[2] = Particle_RandSigned(field<s32>(s, 0x4c));
        } else {  // an ellipse at a random angle: its edge, or within it
            const s32 angle = (NextSeed() >> 16) & 0xfff;
            if (field<u16>(s, 0xa8) != 0) {
                pos[0] = Scale(Sine(angle, 0x80), field<s32>(s, 0x44));
                pos[1] = height();
                pos[2] = -Scale(Sine(angle, 0), field<s32>(s, 0x4c));
            } else {
                const s32 x = Particle_Rand(field<s32>(s, 0x44));
                pos[0] = Scale(Sine(angle, 0x80), x);
                pos[1] = height();
                pos[2] = -Scale(Sine(angle, 0), Particle_Rand(field<s32>(s, 0x4c)));
            }
        }
        if (field<s32>(s, 0xac) != 0)
            Rotate(pos, field<s32>(s, 0xac));
        for (u32 k = 0; k < 3; ++k)
            pos[k] = (s32)((u32)pos[k] + field<u32>(s, 0x14 + 4 * k));
    }
    const s16 push = field<s16>(s, 0xc4);
    if (push != 0) {
        pos[0] = (s32)((u32)pos[0] + (u32)Scale(Sine(field<s32>(s, 0x54), 0), push));
        pos[2] = (s32)((u32)pos[2] + (u32)Scale(Sine(field<s32>(s, 0x54), 0x80), field<s16>(s, 0xc4)));
    }

    // The velocity: a random one in a cylinder, or the system's spread about its own
    s32 *const vel = pos + 3;
    if (field<s32>(s, 0x5c) != 0) {
        const s32 angle = (NextSeed() >> 16) & 0xfff;
        const s32 r = Particle_Rand(field<s32>(s, 0x5c));
        vel[0] = Scale(Sine(angle, 0x80), r);
        vel[1] = Particle_RandSigned(field<s32>(s, 0x5c));
        vel[2] = Scale(Sine(angle, 0), r);
    } else {
        for (u32 k = 0; k < 3; ++k)
            vel[k] = (s32)(field<u32>(s, 0x38 + 4 * k) + (u32)Particle_RandSigned(field<s16>(s, 0x50)));
    }
    if (s[0x6c] != 0) {
        for (u32 k = 0; k < 3; ++k)
            vel[k] = (s32)((u32)vel[k] + field<u32>(s, 0x2c + 4 * k));
    }
    return index;
}
}
