#include "clippoly.h"

#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "clipedge.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

typedef void(__cdecl *EdgeFn)(u8 *out, const u8 *a, const u8 *b, s32 stride);

// The planes (px, py, pz, pw) at 0x766180, 0x766170, ... 0x766130 and the functions at 0x7661a4, 0x7661a0, ... 0x766190.
const float kPlanes[6][4] = {{0, 0, 1, 0}, {0, 0, -1, 1}, {1, 0, 0, 1}, {-1, 0, 0, 1}, {0, 1, 0, 1}, {0, -1, 0, 1}};
const EdgeFn kEdges[6] = {ClipEdge_Z0, ClipEdge_ZW, ClipEdge_XNegW, ClipEdge_XW, ClipEdge_YNegW, ClipEdge_YW};

// ((w pw + y py) + z pz) + px x, fcomp 0 (0x701120): C0 (below or unordered)
bool outside(const float *p, const u8 *v) {
    const float *f = reinterpret_cast<const float *>(v);
    const X87Reg d = ((X87Reg(f[3]) * p[3] + X87Reg(f[1]) * p[1]) + X87Reg(f[2]) * p[2]) + X87Reg(p[0]) * f[0];
    return !(d.v >= 0);
}

}  // namespace

extern "C" {

s32 __fastcall ClipPoly_Indexed(void *, void *, s32 flags, u8 *vertices, ClipPolygon *poly, u32 *vertexCount, s32 stride) {
    u16 localIdx[12];
    ClipPolygon local;
    local.idx = localIdx;
    ClipPolygon *cur = poly, *other = &local;
    for (int plane = 0; plane < 6 && flags != 0; plane++, flags >>= 1) {
        if (!(flags & 1))
            continue;
        u8 *out = vertices + *vertexCount * (u32)stride;
        const u8 *prev = vertices + cur->idx[cur->count - 1] * (u32)stride;
        s32 n = 0;
        for (s32 i = 0; i < cur->count; i++) {
            const u8 *v = vertices + cur->idx[i] * (u32)stride;
            const int code = outside(kPlanes[plane], v) + 2 * outside(kPlanes[plane], prev);
            if (code == 1 || code == 2) {
                if (code == 1)
                    kEdges[plane](out, v, prev, stride);
                else
                    kEdges[plane](out, prev, v, stride);
                out += stride;
                other->idx[n] = (u16)*vertexCount;
                *vertexCount += 1;
                n++;
            }
            if (code == 0 || code == 2)
                other->idx[n++] = cur->idx[i];
            prev = v;
        }
        ClipPolygon *t = cur;
        cur = other;
        other = t;
        cur->count = n;
    }
    if (poly == other) {
        memcpy(other->idx, cur->idx, (u32)cur->count * 2);
        other->count = cur->count;
    }
    return poly->count - 2;
}

u16 *__stdcall ClipPoly_TriangleFan(s32 flags, u16 *end, u8 *vertices, u32 *vertexCount, s32 stride) {
    ClipPolygon poly;
    poly.count = 3;
    poly.idx = end - 3;
    ClipPoly_Indexed(global<void *>(0x00879a20), nullptr, flags, vertices, &poly, vertexCount, stride);
    const s32 n = poly.count;
    u16 *const idx = end - 3;
    if (n <= 3)
        return end + n - 3;
    u16 *w = idx + (n - 2) * 3;
    const u16 *r = idx + n - 1;
    for (s32 k = n - 3; k != 0; k--) {
        *--w = *r--;
        *--w = *r;
        *--w = idx[0];
    }
    return idx + (n - 2) * 3;
}
}
