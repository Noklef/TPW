#include "heightmap.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &g(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const float kMinusOne = -1.0f;   // 0x6fe3ec
const float kInset = 0.001f;     // 0x6fe3f0
const float kHalf = 0.5f;        // 0x6fe3fc
const float kRise = 2.55f;       // 0x6fe3e8

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol

// A vertex past the grid: the node to *hit, *clipped set.
void Hit(u8 *node, u8 **hit, u32 *clipped) {
    if (hit)
        *hit = node;
    if (!*clipped)
        *clipped = 1;
}

// The row table's entry (least x, greatest x, the height at each) widened by (x, y).
void Mark(s32 *row, s32 x, s32 y) {
    if (x < row[0]) {
        row[0] = x;
        row[2] = y;
    }
    if (x > row[1]) {
        row[1] = x;
        row[3] = y;
    }
}

// The grid's cell (x, z) raised to y.
void Raise(const u8 *grid, s32 x, s32 z, s32 y) {
    u8 *const cell = at<u8 *>(grid, 0xc) + g<s32>(0x0079f7d4) * z + x;
    if (y > (s32)*cell)
        *cell = (u8)y;
}

}  // namespace

extern "C" {

u32 __cdecl HeightMap_Rasterize(u8 *node, u32 unused, Matrix44 m, u8 *grid, u8 **hit, u32 unused2, u32 unused3, u32 *clipped) {
    u32 ok = 1;
    Matrix_PreMul(&at<float>(node, 0x10), m.m);
    if (!(at<u32>(node, 0) & 0x8210)) {
        const u32 count = at<u16>(node, 0x58);
        for (u32 i = 0; i < count; ++i) {
            const u32 lane = i & 3, block = (i >> 2) * 0x30;
            {
                const float *const v = &at<float>(at<u8 *>(node, 0x60), block);
                float p[3] = {v[lane], v[lane + 4], v[lane + 8]};
                Matrix_TransformPoint(m.m, p);
                float *const box = &at<float>(grid, 0x18);
                if (!(p[0] >= box[0]))   // (fcomp: below or unordered)
                    box[0] = p[0];
                if (!(p[1] >= box[1]))
                    box[1] = p[1];
                if (!(p[2] >= box[2]))
                    box[2] = p[2];
                if (p[0] > box[3])
                    box[3] = p[0];
                if (p[1] > box[4])
                    box[4] = p[1];
                if (p[2] > box[5])
                    box[5] = p[2];

                float x = p[0];
                if (!(p[0] >= 0.0f)) {
                    if (!(p[0] >= kMinusOne)) {
                        Hit(node, hit, clipped);
                        ok = 0;
                    }
                    x = 0.0f;
                } else if (!(p[0] < g<float>(0x0079fb20))) {
                    if (X87Reg(p[0]).v > (X87Reg(g<float>(0x0079fb20)) - kMinusOne).v) {
                        Hit(node, hit, clipped);
                        ok = 0;
                    }
                    x = (float)(X87Reg(g<float>(0x0079fb20)) - kInset);
                }
                if (!(p[2] >= 0.0f)) {
                    if (!(p[2] >= kMinusOne)) {
                        Hit(node, hit, clipped);
                        ok = 0;
                    }
                    p[2] = 0.0f;
                } else if (!(p[2] < g<float>(0x0079fb08))) {
                    if (!((X87Reg(g<float>(0x0079fb08)) - kMinusOne).v >= p[2])) {
                        Hit(node, hit, clipped);
                        ok = 0;
                    }
                    p[2] = (float)(X87Reg(g<float>(0x0079fb08)) - kInset);
                }
                if (!(p[1] >= 0.0f))
                    p[1] = 0.0f;
                if (p[1] >= 100.0f) {   // (fcomp 100: not below)
                    if (!*clipped)
                        *clipped = 1;
                    at<u32>(&p[1], 0) = 0x42c7ff7d;   // 99.999
                }
                float *const out = &at<float>(at<u8 *>(node, 0x60), block);
                out[lane] = x;
                out[lane + 4] = p[1];
                out[lane + 8] = p[2];
            }
        }

        const u32 groups = at<u16>(node, 0x5a);
        for (u32 k = 0; k < groups; ++k) {
            const u8 *const group = at<u8 *>(node, 0x6c) + 0x10 * k;
            const u16 *face = &at<u16>(at<u8 *>(node, 0x70), at<u16>(group, 4) * 8);
            for (u32 n = at<u16>(group, 6); n; --n, face += 4) {
                const u16 *const remap = at<u16 *>(node, 0x94);
                const u8 *const vertices = at<u8 *>(node, 0x60);
                s32 x[3], y[3], z[3];   // corner c from the face's u16 +2c+2
                for (int c = 2; c >= 0; --c) {
                    const u32 index = remap[face[c + 1]];
                    const float *const v = &at<float>(vertices, (index >> 2) * 0x30) + (index & 3);
                    x[c] = Ftol(X87Reg(v[0]) * kHalf);
                    y[c] = Ftol(X87Reg(v[4]) * kRise);
                    z[c] = Ftol(X87Reg(v[8]) * kHalf);
                }
                s32 low = z[0], high = z[0];
                if (z[1] < z[0])
                    low = z[1];
                if (z[1] > z[0])
                    high = z[1];
                if (z[2] < low)
                    low = z[2];
                if (z[2] > high)
                    high = z[2];
                const s32 rows = high - low + 1;
                s32 *const table = &g<s32>(0x0079f7e8);
                for (u32 r = (u32)rows, *e = (u32 *)table; r; --r, e += 4) {
                    e[0] = 1000000;
                    e[1] = (u32)-1000000;
                }
                for (int a = 2; a >= 0; --a) {   // the edge from corner a to corner (a + 1) % 3
                    const int b = (a + 1) % 3;
                    const s32 dx = x[b] - x[a], dy = y[b] - y[a];
                    s32 dz = z[b] - z[a];
                    const s32 half = dz / 2;
                    s32 step = 1;
                    if (dz < 0) {
                        step = -1;
                        dz = -dz;
                    }
                    s32 *row = table + 4 * (z[a] - low);
                    Mark(row, x[a], y[a]);
                    Mark(row + 4 * (dz * step), x[b], y[b]);
                    if (dz > 1) {
                        s32 nx = half + dx, ny = dy;
                        for (s32 left = dz - 1; left; --left) {
                            row += 4 * step;
                            Mark(row, nx / dz + x[a], ny / dz + y[a]);
                            nx += dx;
                            ny += dy;
                        }
                    }
                }
                if (rows > 0) {
                    const s32 *row = table;
                    for (s32 r = rows, zz = low; r; --r, ++zz, row += 4) {
                        const s32 x0 = row[0], x1 = row[1], y0 = row[2], y1 = row[3];
                        const s32 dy = y1 - y0, width = x1 - x0;
                        Raise(grid, x0, zz, y0);
                        Raise(grid, x1, zz, y1);
                        s32 num = dy;
                        for (s32 i = 1; i < width; ++i, num += dy)
                            Raise(grid, x0 + i, zz, num / width + y0);
                    }
                }
            }
        }
    }
    for (u8 *child = at<u8 *>(node, 0xc); child; child = at<u8 *>(child, 8))
        if (!HeightMap_Rasterize(child, unused, m, grid, hit, unused2, unused3, clipped))
            ok = 0;
    return ok;
}
}
