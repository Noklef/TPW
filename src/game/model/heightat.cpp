#include "heightat.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }
// The view's (0x79fb18) model's (+4) floor offset (+0xa0), read afresh each time as the original does.
u32 &view_offset() { return *reinterpret_cast<u32 *>(global<uintptr_t>(global<uintptr_t>(0x0079fb18) + 4) + 0xa0); }
template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

X87Reg neg(X87Reg a) { return -a.v; }  // fchs

}  // namespace

extern "C" {

float __cdecl Model_HeightAt(const u8 *model, const u8 *instance, float x, float z, const float *normals, s32 first) {
    float best = -100.0f;
    const u32 groups = at<u16>(model, 0x5a);
    const u8 *group = at<const u8 *>(model, 0x6c);
    const u8 *const faces = at<const u8 *>(model, 0x70);
    for (u32 g = 0; g < groups; ++g, group += 0x10) {
        const u16 *face = reinterpret_cast<const u16 *>(faces + at<u16>(group, 4) * 8);
        const u8 *verts = global<const u8 *>(0x0079f7e0);
        if (instance) verts += at<u16>(instance, 0x10) * 12;
        for (u32 left = at<u16>(group, 6); left; --left, face += 4) {
            if (!(face[0] & 0x8000)) continue;
            const float *normal;
            if (normals) {
                normal = normals;
                normals += 3;
            } else {
                normal = reinterpret_cast<const float *>(at<const u8 *>(model, 0x64) + (face[0] & 0x7fff) * 12);
            }
            const u16 *const remap = at<const u16 *>(model, 0x94);
            float v[3][3];
            for (u32 k = 0; k < 3; ++k) {
                const u32 *const from = reinterpret_cast<const u32 *>(verts + remap[face[1 + k]] * 12);
                for (u32 c = 0; c < 3; ++c) reinterpret_cast<u32 &>(v[k][c]) = from[c];
            }
            bool under = true;
            for (u32 e = 3; e; --e) {  // edges 2 -> 0, 1 -> 2, 0 -> 1
                const float *const a = v[e - 1], *const b = v[e % 3];
                const X87Reg dz = X87Reg(b[2]) - a[2];
                const X87Reg nx = neg(b[0]) - neg(a[0]);
                const float dzf = (float)dz;  // through a local
                const X87Reg c = dz * a[0] + nx * a[2];
                const X87Reg side = neg(c) + X87Reg(dzf) * x + nx * z;
                if (!(side.v >= 0.0)) {  // fcomp 0: below, or unordered
                    under = false;
                    break;
                }
            }
            if (!under) continue;
            const float ny = normal[1];
            X87Reg pa = 0.0, pb = 0.0;  // the normal's z and x, turned with the view
            switch (at<s32>(global<const u8 *>(0x0079fb18), 0x10)) {
            case 90:
                pa = neg(normal[0]);
                pb = normal[2];
                break;
            case 180:
                pa = neg(normal[2]);
                pb = neg(normal[0]);
                break;
            case 270:
                pa = normal[0];
                pb = neg(normal[2]);
                break;
            default:
                pa = normal[2];
                pb = normal[0];
                break;
            }
            const X87Reg d = pb * v[0][0] + X87Reg(ny) * v[0][1] + pa * v[0][2];
            const X87Reg h = neg((neg(d) + pb * x + pa * z) / ny);
            if (h.v > best) best = (float)h;  // fcom: not when below, equal or unordered
            if (first) return best;
        }
    }
    return best;
}

float __cdecl Model_FloorHeight(float x, float z, s32 noParts, s32 noMain) {
    const u8 *view = global<const u8 *>(0x0079fb18);
    const u8 *const model = at<const u8 *>(view, 4);
    const u8 *const holder = at<const u8 *>(view, 0x28);
    float best = -100.0f;
    if (!noMain && at<const u8 *>(view, 0x2c)) {
        best = Model_HeightAt(at<const u8 *>(view, 0x2c), nullptr, x, z, at<const float *>(view, 0x30), 1);
        view = global<const u8 *>(0x0079fb18);
        if (!(best != -100.0f)) {  // fcomp: equal, or unordered
            at<u32>(&best, 0) = at<u32>(at<const u8 *>(at<const u8 *>(view, 4), 0x78), 0x44);
        }
    }
    if (noParts || !holder || !(at<u8>(holder, 0) & 8)) return best;
    const u8 *node = at<const u8 *>(model, 0x70) + at<u16>(model, 0x46) * 0xa0;  // sic: big nodes only
    const u8 *flags = at<const u8 *>(model, 0x7c);
    const u8 *instance = at<const u8 *>(at<const u8 *>(view, 0x28), 4);
    for (u32 left = at<u16>(model, 0x48); left; --left, node += 0xa0, flags += 0x14, instance += 0x14) {
        const u32 bits = at<u32>(flags, 0);
        if (!(bits & 2)) continue;
        const float h = Model_HeightAt(node, instance, x, z, nullptr, bits & 4);
        if (!(h > best)) continue;  // fcom: below, equal or unordered
        if (at<u8>(flags, 0) & 4) return h;
        best = h;
    }
    return best;
}

void __cdecl View_FloorAt(float x, float z, float *out) {
    float offset;
    std::memcpy(&offset, &view_offset(), 4);
    if (std::isnan(offset)) view_offset() = 0;   // _isnan (0x67b410)
    const u8 mode = global<u8>(0x0079fb10);
    double height;
    if (mode & 1) {
        height = Terrain_HeightAt(x, z);
    } else if (mode & 8) {
        *reinterpret_cast<u32 *>(out) = view_offset();
        return;
    } else if (mode & 2) {
        height = Model_FloorHeight(x, z, 0, 1);
    } else if (mode & 4) {
        height = Model_FloorHeight(x, z, 1, 0);
    } else {
        *reinterpret_cast<u32 *>(out) = 0;
        return;
    }
    std::memcpy(&offset, &view_offset(), 4);
    *out = (float)(X87Reg(height) + offset).v;
}
}
