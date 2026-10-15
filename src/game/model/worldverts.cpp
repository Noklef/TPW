#include "worldverts.h"

#include <cstdint>
#include <cstring>

#include "../core/globalmem.h"
#include "../math/matrix.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

float *&block() { return *reinterpret_cast<float **>(0x007aecd4); }

}  // namespace

extern "C" {

void __cdecl WorldVerts_Alloc(u32 large) {
    const u32 entries = large ? 0x400 : *reinterpret_cast<const u32 *>(0x007a42ac);
    block() = static_cast<float *>(GlobalMem_Alloc(entries * 12, 1));
}

void __cdecl WorldVerts_Free() { GlobalMem_Free(block()); }

void __cdecl WorldVerts_TransformFrame(const u8 *frame) {
    float m[16];
    std::memcpy(m, frame + 0x10, sizeof m);
    for (const u8 *parent = at<const u8 *>(frame, 4); parent; parent = at<const u8 *>(parent, 4))
        Matrix_Mul(m, reinterpret_cast<const float *>(parent + 0x10));
    float *out = block();
    for (u32 i = 0; i < at<u16>(frame, 0x58); ++i, out += 3) {
        const u32 *const group = at<const u32 *>(frame, 0x60) + (i >> 2) * 12 + (i & 3);
        u32 *const raw = reinterpret_cast<u32 *>(out);  // copied as bits, then read back
        raw[0] = group[0];
        raw[1] = group[4];
        raw[2] = group[8];
        const float x = out[0], y = out[1], z = out[2];
        const X87Reg ny = X87Reg(m[9]) * z + X87Reg(m[5]) * y + X87Reg(m[1]) * x + m[13];
        const X87Reg nz = X87Reg(m[10]) * z + X87Reg(m[6]) * y + X87Reg(m[2]) * x + m[14];
        const X87Reg nx = X87Reg(m[8]) * z + X87Reg(m[4]) * y + X87Reg(m[0]) * x + m[12];
        out[1] = (float)ny;
        out[2] = (float)nz;
        out[0] = (float)nx;
    }
}

void __cdecl WorldVerts_Fetch(const u8 *groups, u32 index, u32 *out) {
    const u32 *const lane = reinterpret_cast<const u32 *>(groups + (index >> 2) * 0x30) + (index & 3);
    out[0] = lane[0];
    out[1] = lane[4];
    out[2] = lane[8];
}
}
