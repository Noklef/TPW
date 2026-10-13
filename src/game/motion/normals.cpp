// Face normals of animated nodes: reimplementation of 0x473a40 and 0x473c30.
#include "normals.h"

#include <cmath>

#include "../math/x87.h"

namespace {

template <typename T> T field(const void *p, u32 offset) {
    return *reinterpret_cast<const T *>(static_cast<const u8 *>(p) + offset);
}
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

struct Vec {
    float x, y, z;
};

// A vertex of the node's position blocks, through its remap table.
Vec vertex(const u8 *node, u16 index) {
    const u32 slot = field<const u16 *>(node, 0x94)[index];
    const float *lane = reinterpret_cast<const float *>(field<const u8 *>(node, 0x60) + 0x30 * (slot >> 2)) + (slot & 3);
    return {lane[0], lane[4], lane[8]};
}

}  // namespace

extern "C" void __cdecl Node_UpdateNormals(u8 *node) {
    // In the x87's registers (24-bit mantissa, wide exponent) except where the original
    // stores to a float and reloads (f): cz and the products never leave the registers, so
    // huge coordinates do not overflow where float arithmetic would.
    const auto r = [](double x) { return X87_Round(x); };
    const auto f = [](double x) { return (double)(float)x; };
    const u16 *face = field<const u16 *>(node, 0x70);
    for (u32 n = field<u16>(node, 0x5c); n != 0; --n, face += 4) {
        float *normal = reinterpret_cast<float *>(field<u8 *>(node, 0x64) + 12 * (face[0] & 0x7fff));
        const Vec a = vertex(node, face[2]), b0 = vertex(node, face[1]), c0 = vertex(node, face[3]);
        const double bx = f(r((double)b0.x - a.x)), by = f(r((double)b0.y - a.y)), bz = f(r((double)b0.z - a.z));
        const double cx = f(r((double)c0.x - a.x)), cy = f(r((double)c0.y - a.y)), cz = r((double)c0.z - a.z);
        const double nx = r(r(bz * cy) - r(by * cz));
        const double ny = r(r(bx * cz) - r(bz * cx));
        const double nz = r(r(by * cx) - r(bx * cy));
        const double length = f(r(std::sqrt(r(r(r(nx * nx) + r(ny * f(ny))) + r(nz * f(nz))))));
        normal[0] = (float)r(f(nx) / length);
        normal[1] = (float)r(f(ny) / length);
        normal[2] = (float)r(f(nz) / length);
        if (std::isnan(normal[0])) {  // _isnan (0x67b410)
            normal[0] = 0;
            normal[1] = -1;
            normal[2] = 0;
        }
    }
    at<u32>(node, 0) &= ~0x10000u;
}

extern "C" void __cdecl Model_UpdateNormals(u8 *model) {
    u8 *node = field<u8 *>(model, 0x70);
    for (u32 n = field<u16>(model, 0x44); n != 0; --n, node += 0xa0)
        if (field<u32>(node, 0) & 0x10000) Node_UpdateNormals(node);
    at<u32>(model, 0x30) &= ~0x40000u;
}
