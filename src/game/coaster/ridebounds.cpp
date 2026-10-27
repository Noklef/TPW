#include "ridebounds.h"

#include <cstdint>

#include "../math/x87.h"
#include "../model/parts.h"
#include "../render/particles.h"
#include "../world/objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

}  // namespace

extern "C" {

u32 __cdecl Ride_Bounds(s32 index, float *out) {
    const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[index];
    if (!ride) return 0;
    const u8 *const block = at<const u8 *>(ride, 0xcc);
    if (!block) return 0;
    const u32 low = at<u32>(ride, 0xc0), high = at<u32>(ride, 0xc4);
    const float base = at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 4), 0x78), 0x44);
    // fild of each 16-bit half, exact; the far corner is the last tile's + 1 (- -1.0)
    out[0] = (float)(X87Reg(low >> 16) * 10.0f);
    out[1] = (float)(X87Reg(at<float>(block, 0x1c)) + base);
    out[2] = (float)(X87Reg(low & 0xffff) * 10.0f);
    out[3] = (float)((X87Reg(high >> 16) - -1.0f) * 10.0f);
    out[4] = (float)(X87Reg(at<float>(block, 0x28)) + base);
    out[5] = (float)((X87Reg(high & 0xffff) - -1.0f) * 10.0f);
    return 1;
}

u32 __cdecl Ride_Centre(const s32 *handle, float *x, float *y, float *z) {
    float box[6];
    if (*handle == 0 || !Ride_Bounds(*handle, box)) {
        *z = 0.0f;
        *y = 0.0f;
        *x = 0.0f;
        return 1;
    }
    const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[*handle];
    const float cx = (float)((X87Reg(box[0]) + box[3]) * 0.5f);
    const float cz = (float)((X87Reg(box[2]) + box[5]) * 0.5f);
    *y = X87_Through(at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 4), 0x78), 0x44));
    *x = cx;
    *z = cz;
    return 0;
}

void __cdecl Ride_SetEffect(u8 *context, u32 on) {
    if (!on) {
        const u32 handle = at<u32>(context, 0xcc);
        if (handle) {
            ParticleSystem_SetAge(handle, 0xfffffffeu);
            at<u32>(context, 0xcc) = 0;
        }
        return;
    }
    const u16 slot = at<u16>(context, 0xac);
    if (!slot) return;
    const u8 *const thing = *reinterpret_cast<const u8 *const *>((uintptr_t)0x007cfb90 + slot * 0x14u);
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, at<u16>(thing, 0xe));
    const s32 effect = at<s32>(def, 0x68);
    if (!effect) return;
    const s32 index = at<s32>(context, 0xc8);
    float box[6], x = 0.0f, y = 0.0f;
    X87Reg z = 0.0;
    if (index && Ride_Bounds(index, box)) {
        const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[index];
        x = (float)((X87Reg(box[0]) + box[3]) * 0.5f);
        z = (X87Reg(box[2]) + box[5]) * 0.5f;
        y = X87_Through(at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 4), 0x78), 0x44));
    }
    const s32 iz = Ftol(z * 1024.0f);
    const s32 iy = Ftol(X87Reg(y) * 1024.0f);
    const s32 ix = Ftol(X87Reg(x) * 1024.0f);
    at<s32>(context, 0xcc) = Particle_Spawn(effect, ix, iy, iz);
}

u32 __cdecl Ride_PartPlace(const u8 *obj, float *pos, float *dir, s32 key, u32 mask) {
    const u8 *const *const rides = reinterpret_cast<const u8 *const *>(0x007a4610);
    const u32 index = at<u32>(obj, 0xc8);
    if (key < 0 || index == 0) {
        float box[6];
        if (index == 0 || !Ride_Bounds(index, box)) {
            pos[0] = 0.0f;
            pos[1] = 0.0f;
            pos[2] = 0.0f;
            return 1;
        }
        const float cx = (float)((X87Reg(box[0]) + box[3]) * 0.5f);  // 0x700fd0
        const float cz = (float)((X87Reg(box[2]) + box[5]) * 0.5f);
        const float y = X87_Through(at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(rides[index], 8), 4), 0x78), 0x44));
        pos[0] = cx;
        pos[1] = y;
        pos[2] = cz;
        return 1;
    }
    const u32 part = Model_FindPartByKey4(rides[index], mask, (u32)key);
    if (part == 0xffffffff) return 0;  // (and the empty Debug_Printf)
    const auto entry = [&] {
        return at<const u8 *>(at<const u8 *>(at<const u8 *>(rides[at<u32>(obj, 0xc8)], 8), 0x28), 4) + part * 0x14;
    };
    const u8 *node = at<const u8 *>(entry(), 4);
    // x through a temporary and z loaded first, both by the x87; y copied as it is
    const float z = node ? X87_Through(at<float>(node, 0x38)) : 0.0f;  // 0x700fd8
    const float x = node ? X87_Through(at<float>(node, 0x30)) : 0.0f;
    const u32 y = node ? at<u32>(node, 0x34) : 0;
    at<u32>(pos, 4) = y;
    pos[0] = x;
    pos[2] = z;
    if (!dir) return 1;
    const u8 *const e = entry();
    node = at<const u8 *>(e, 4);
    X87Reg dx = 0.0f, dy = 0.0f, dz = 1.0f;  // 0x700fd8, 0x700fc8
    if (node) {
        float v[3];
        for (int i = 0; i < 3; i++) v[i] = at<float>(node, 0x20 + 4 * i);
        if (at<u8>(e, 0) & 0x10)
            for (int i = 0; i < 3; i++) v[i] = -X87_Through(v[i]);  // fchs; fstp
        const float length = (float)X87_Sqrt(X87Reg(v[0]) * v[0] + X87Reg(v[1]) * v[1] + X87Reg(v[2]) * v[2]);
        dx = X87Reg(v[0]) / length;
        dy = X87Reg(v[1]) / length;
        dz = X87Reg(v[2]) / length;
    }
    dir[0] = (float)(dx * 1024.0f);  // 0x700fdc
    dir[1] = (float)(dy * 1024.0f);
    dir[2] = (float)(dz * 1024.0f);
    return 1;
}

void __cdecl Ride_PartAxes(s32 index, const u32 *part, float *dir, float *up) {
    const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[index];
    const u8 *const entry = at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 0x28), 4) + *part * 0x14;
    const u8 *const node = at<const u8 *>(entry, 4);
    if (!node) {
        dir[1] = 0.0f;
        dir[0] = 0.0f;
        dir[2] = 1.0f;
        if (up) {
            up[2] = 0.0f;
            up[0] = 0.0f;
            up[1] = 1.0f;
        }
        return;
    }
    if (at<u8>(entry, 0) & 0x10)
        for (int i = 0; i < 3; i++) dir[i] = -X87_Through(at<float>(node, 0x20 + 4 * i));  // fchs; fstp
    else
        for (int i = 0; i < 3; i++) at<u32>(dir, 4 * i) = at<u32>(node, 0x20 + 4 * i);
    // each component read back from dir (y through a temporary), squared and summed, fsqrt stored as a float, then divided by it
    float x = dir[0], y = dir[1], z = dir[2];
    float length = (float)X87_Sqrt(X87Reg(x) * x + X87Reg(y) * y + X87Reg(z) * z);
    x = dir[0], y = dir[1], z = dir[2];
    dir[0] = (float)(X87Reg(x) / length);
    dir[1] = (float)(X87Reg(y) / length);
    dir[2] = (float)(X87Reg(z) / length);
    if (!up) return;
    at<u32>(up, 0) = at<u32>(node, 0x10);
    at<u32>(up, 4) = at<u32>(node, 0x14);
    x = up[0], y = up[1], z = at<float>(node, 0x18);
    const float sum_z = z;
    length = (float)X87_Sqrt(X87Reg(x) * x + X87Reg(y) * y + X87Reg(sum_z) * sum_z);
    x = up[0], y = up[1];
    up[2] = X87_Through(z);  // fstp, before the length is stored
    z = up[2];
    up[0] = (float)(X87Reg(x) / length);
    up[1] = (float)(X87Reg(y) / length);
    up[2] = (float)(X87Reg(z) / length);
}
}
