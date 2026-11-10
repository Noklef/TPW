#include "heightmap.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../motion/animator.h"

namespace {

AddrCalls g_calls;

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
                } else if (!(p[2] < gHeightmap_79fb08)) {
                    if (!((X87Reg(gHeightmap_79fb08) - kMinusOne).v >= p[2])) {
                        Hit(node, hit, clipped);
                        ok = 0;
                    }
                    p[2] = (float)(X87Reg(gHeightmap_79fb08) - kInset);
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

namespace {

// The clone rasterized once, the grid's globals set first (its size * 0x6fe3e4 and in fifths).
void RasterizeClone(u8 *clone, u8 *map, u32 block, u32 time, u32 *clipped) {
    u8 *const held = at<u8 *>(clone, 8);
    const u32 width = at<u16>(map, 8), height = at<u16>(map, 0xa);
    g<float>(0x0079fb20) = (float)X87_MulInt(width, g<const float>(0x006fe3e4));
    gHeightmap_79fb08 = (float)X87_MulInt(height, g<const float>(0x006fe3e4));
    g<u32>(0x0079f7d4) = width * 5;
    gHeightmap_79fb0c = height * 5;
    Matrix44 matrix;
    g_calls.Cd(0x0046f150, &matrix);   // Matrix_Identity
    u8 *const model = at<u8 *>(held, 4);
    u8 *hit;
    g_calls.Cd(0x00451e20, at<u32>(model, 0x78), model, matrix, map, &hit, block, time, clipped);   // HeightMap_Rasterize
}

void PlayBlock(u8 *clone, const u8 *model, u8 *map, u32 i, u32 *clipped) {
    const u32 block = at<u32>(at<u8 *>(clone, 0xac), 4 * i);
    u32 time = 0, bank, entry, third;
    g_calls.Cd(0x00468600, clone, i, &bank, &entry, &third);   // RsseScript_EventName
    u8 layer[0x38];
    at<u32>(layer, 4) = 0xc;
    at<u32>(layer, 8) = 0;
    at<u32>(layer, 0xc) = 0x3f800000;
    g_calls.Cd(0x004709d0, time);   // Motion_SetTime
    g_calls.Cd(0x00472bc0, layer, block, 1u, 0u);   // Layer_Start
    g_calls.Cd(0x00472c60, layer, &time, 1u);   // Layer_Advance
    g_calls.Cd(0x00472f60, clone, bank, entry, layer, 0u, 0x3f800000u);   // Layer_Play
    while (Layer_TimeLeft(layer) >= 0.0) {   // (0x6fe3dc; unordered is "below")
        g_calls.Cd(0x004721f0, layer, at<u32>(clone, 8), block);   // Motion_Apply
        RasterizeClone(clone, map, block, (u32)Ftol(X87Reg(at<const float>(layer, 0x20))), clipped);
        u8 *const held = at<u8 *>(clone, 8);
        g_calls.Cd(0x00472270, at<u32>(held, 8), held, 1u);   // Motion_CopyVertices
        u32 *part = at<u32 *>(model, 0x70);
        for (u32 n = at<u16>(model, 0x44); n; --n, part += 0xa0 / 4)
            *part &= 0xff7fffff;
        time += 0x1e;
        g_calls.Cd(0x004709d0, time);
        g_calls.Cd(0x00472c60, layer, &time, 1u);
    }
    g_calls.Cd(0x00472310, clone, at<u32>(at<u8 *>(clone, 0x20 + 0xc * at<u32>(layer, 4)), 4 + 8 * at<u32>(layer, 8)), 0u);   // Motion_ResetChannels
}

}  // namespace

void HeightMap_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

static void copy_grid(u8 *to, const u8 *from, u32 bytes) {
    if (from)
        for (u32 i = 0; i < bytes; ++i) to[i] = from[i];
}

void __cdecl HeightMap_ReadOrBuild(u8 *instance, u32 width, u32 height, const char *path, const u8 *flags, u32 useScale) {
    u32 got = 0;
    u8 *map = nullptr;
    if (const u32 stream = g_calls.Cd(0x005c4d50, path, 0x0074cfb8u)) {
        const u32 size = g_calls.Cd(0x005c5000, stream);
        map = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, size, 1u));
        if (map) got = g_calls.Cd(0x005c4f60, map, size, 1u, stream);
        g_calls.Cd(0x005c4f80, stream);
    }
    if (!got) {
        g_calls.Cd(0x0045ba00, map);
        map = nullptr;
    }
    if (map) {
        if (at<u32>(map, 0) == 0xab1e0003 && at<u32>(map, 4) == 0x640005) {
            for (u32 offset = 0xc; offset <= 0x14; offset += 4) at<u32>(map, offset) += (u32)(uintptr_t)map;
        } else {
            g_calls.Cd(0x0045ba00, map);
            map = nullptr;
        }
    }
    if (!map || width != at<u16>(map, 8) || height != at<u16>(map, 0xa)) {
        map = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x00451880, instance, width, height, flags, useScale));
        if (map) {
            const u32 cells = static_cast<u32>(at<u16>(map, 0xa)) * at<u16>(map, 8);
            const u32 total = cells * 25 + cells * 2 + 0x30;
            u8 *const block = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, total, 0u));
            for (u32 offset = 0; offset < 0x30; offset += 4) at<u32>(block, offset) = at<u32>(map, offset);
            copy_grid(block + 0x30, at<const u8 *>(map, 0xc), cells * 25);
            copy_grid(block + 0x30 + cells * 25, at<const u8 *>(map, 0x10), cells);
            copy_grid(block + 0x30 + cells * 26, at<const u8 *>(map, 0x14), cells);
            at<u32>(block, 0xc) = 0x30;
            at<u32>(block, 0x10) = 0x30 + cells * 25;
            at<u32>(block, 0x14) = 0x30 + cells * 26;
            if (const u32 file = g_calls.Cd(0x0067b8a0, path, 0x0074cfb4u)) {
                g_calls.Cd(0x0067b6d0, block, total, 1u, file);
                g_calls.Cd(0x0067b620, file);
            }
            g_calls.Cd(0x0045ba00, block);
        }
    }
    u8 *const to = at<u8 *>(at<u8 *>(instance, 8), 4) + 0x80;
    for (u32 i = 0; i < 6; ++i) at<u32>(to, 4 * i) = at<u32>(map, 0x18 + 4 * i);
    at<u8 *>(instance, 0xcc) = map;
}

u8 *__cdecl HeightMap_Build(u8 *instance, u32 width, u32 height, const u8 *flags, u32 useScale) {
    u8 *clone;
    if (!g_calls.Cd(0x004557c0, instance, 0u, 0x211fu, &clone, 0u))   // Instance_Clone
        return nullptr;
    g_calls.Cd(0x00473e30, clone, 1u, 0u, 0u);   // Animator_Reset
    u8 *const model = at<u8 *>(at<u8 *>(clone, 8), 4);
    u8 *const map = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, 0x30u, 1u));   // GlobalMem_Alloc
    at<u16>(map, 8) = (u16)width;
    at<u16>(map, 0xa) = (u16)height;
    const u32 squares = width * height;
    at<u32>(map, 0) = 0xab1e0003;
    at<u32>(map, 4) = 0x640005;
    u8 *const cells = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, 27 * squares, 1u));
    at<u8 *>(map, 0xc) = cells;
    at<u8 *>(map, 0x10) = cells + 25 * squares;
    at<u8 *>(map, 0x14) = cells + 26 * squares;
    u32 clipped = 0;
    bool moves = false;
    if (at<u32>(clone, 0xa8)) {
        for (u32 i = 2; i < 0xc; ++i)
            if (at<u32>(clone, 0x1c + 0xc * i))
                moves = true;
    }
    if (moves) {
        for (u32 i = 0; i < 3; ++i) {
            at<u32>(map, 0x18 + 4 * i) = 0x7f7fffff;
            at<u32>(map, 0x24 + 4 * i) = 0x800000;
        }
        for (u32 i = 0; i < at<u32>(clone, 0xa8); ++i) {
            const u32 block = at<u32>(at<u8 *>(clone, 0xac), 4 * i);
            if (!block) continue;
            u8 *const first = at<u8 *>(clone, 0x20), *const second = at<u8 *>(clone, 0x2c);
            if (first && block == at<u32>(first, 4)) continue;
            if (second && block == at<u32>(second, 4)) continue;
            PlayBlock(clone, model, map, i, &clipped);
        }
    } else {
        const u8 *const bounds = at<u8 *>(at<u8 *>(instance, 8), 4) + 0x80;
        for (u32 i = 0; i < 6; ++i)
            at<u32>(map, 0x18 + 4 * i) = at<u32>(bounds, 4 * i);
        RasterizeClone(clone, map, 0, 0, &clipped);
    }
    g_calls.Cd(0x00455ad0, clone, 0u, 0u);   // Instance_Dispose
    u32 base = 0, row = 0, flag = 1;
    for (u32 left = height; left; --left) {
        for (u32 x = 0; x < width; ++x) {
            const u8 *cell = at<u8 *>(map, 0xc) + row + 5 * x;
            u8 top = 0;
            for (u32 i = 0; i < 5; ++i, cell += width * 5)
                for (u32 k = 0; k < 5; ++k)
                    if (cell[k] > top) top = cell[k];
            at<u8 *>(map, 0x10)[x + base] = useScale ? (u8)Ftol(X87Reg(at<const float>(model, 0x90)) * kRise) : top;
            at<u8 *>(map, 0x14)[x + base] = flags && !at<u32>(flags, 8 * (flag + x)) ? 0 : 1;
        }
        base += width;
        row += width * 25;
        flag += 0x14;
    }
    return map;
}

