#include "basecells.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../math/x87.h"
#include "../world/mapcell.h"
#include "baseuvs.h"
#include "worldverts.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const BaseCellsEnv kPortEnv = {&MapGrid_MarkEdges, &GlobalMem_Alloc, &WorldVerts_TransformFrame, &RideBase_TidyAllUVs};
const BaseCellsEnv *g_env = &kPortEnv;

struct Vec {
    float x, y, z;
};

// 0x54 bytes.
struct Cell {
    float acrossBest, alongBest;
    s32 group, triangle;
    Vec across, along;
    u32 acrossEdge, alongEdge;
    Vec corners[3];
};

struct UV {
    float u, v;
};

const u32 kRow = 20;

s32 ftol(X87Reg x) { return (s32)(long long)x.v; }

// A triangle's edges against the bests so far.
struct Edges {
    Vec *across = nullptr, *along = nullptr;
    u32 acrossEdge = 0, alongEdge = 0;
    float acrossBest = 0.0f, alongBest = 0.0f;
};

void measure_edge(Vec &edge, const Vec &from, const Vec &to, u32 k, Edges &out) {
    edge = to;
    const X87Reg dx = X87Reg(edge.x) - from.x, dy = X87Reg(edge.y) - from.y, dz = X87Reg(edge.z) - from.z;
    const float fx = (float)dx, fy = (float)dy;
    const X87Reg sum = dx * dx + dy * fy + dz * dz;
    const float length = (float)X87_Sqrt(sum);
    const X87Reg nx = X87Reg(fx) / length, nz = dz / length, ny = X87Reg(fy) / length;
    edge.x = (float)nx;
    edge.y = (float)ny;
    edge.z = (float)nz;
    const double acrossAbs = std::fabs(nx.v);
    if (acrossAbs > (double)out.acrossBest) {
        out.across = &edge;
        out.acrossBest = (float)acrossAbs;
        out.acrossEdge = k;
    }
    const float alongAbs = (float)std::fabs(nz.v);
    if (alongAbs > out.alongBest) {
        out.along = &edge;
        out.alongBest = alongAbs;
        out.alongEdge = k;
    }
}

void place_triangle(Cell *cells, const u8 *marks, const u8 *group, const u8 *triangle, s32 groupIndex, s32 triangleIndex, const u16 *map, u32 all) {
    const Vec *const vertices = global<const Vec *>(0x7aecd4);
    const Vec *const corners[3] = {&vertices[map[at<u16>(triangle, 2)]], &vertices[map[at<u16>(triangle, 4)]], &vertices[map[at<u16>(triangle, 6)]]};
    const float midZ = (float)(X87Reg(corners[0]->z) + corners[1]->z);
    const X87Reg midX = (X87Reg(corners[0]->x) + corners[1]->x + corners[2]->x) * 0x1.555556p-2f;
    const float centreZ = (float)((X87Reg(corners[2]->z) + midZ) * 0x1.555556p-2f);
    const u8 *const area = at<const u8 *>(global<const u8 *>(0x7a0854), 0x6c);
    const s32 x = ftol(midX / at<float>(area, 0x10));
    const s32 y = ftol(X87Reg(centreZ) / at<float>(area, 0x14));
    Cell &cell = cells[(s32)(x + y * kRow)];
    u16 &flags = *at<u16 *>(group, 0);
    if (!all && marks && !marks[(s32)((u32)x << 4) + y]) flags &= 0xf7ff;
    else flags |= 0x800;

    Vec edges[3];
    Edges best;
    for (u32 k = 0; k < 3; k++) measure_edge(edges[k], *corners[k], *corners[(k + 1) % 3], k, best);
    if (!(best.acrossBest > 0.9f) || !(best.alongBest > 0.9f)) return;
    if (!(best.acrossBest > cell.acrossBest) && !(best.alongBest > cell.alongBest)) return;
    cell.acrossBest = best.acrossBest;
    cell.alongBest = best.alongBest;
    cell.group = groupIndex;
    cell.triangle = triangleIndex;
    for (u32 k = 0; k < 3; k++) cell.corners[k] = *corners[k];
    cell.across = *best.across;
    cell.along = *best.along;
    cell.acrossEdge = best.acrossEdge;
    cell.alongEdge = best.alongEdge;
}

// The four corners of the texture's rectangle: three from the triangle, the fourth, `missing`, made from them.
void read_corners(const Cell &cell, const u8 *model, const u8 *triangle, UV *p) {
    const u32 a = cell.acrossEdge, b = cell.alongEdge;
    const bool acrossUp = cell.across.x > 0.0f, alongUp = cell.along.z > 0.0f;
    const u32 e1 = acrossUp ? a : (a + 1) % 3, e2 = acrossUp ? (a + 1) % 3 : a;
    const u32 f1 = alongUp ? b : (b + 1) % 3, f2 = alongUp ? (b + 1) % 3 : b;
    u32 slots[4] = {};
    u32 missing;
    if (e1 == f1) {
        missing = 2;
        slots[0] = e1, slots[1] = f2, slots[3] = e2;
    } else if (f2 == e1) {
        missing = 3;
        slots[0] = f1, slots[1] = f2, slots[2] = e2;
    } else if (e2 == f2) {
        missing = 0;
        slots[1] = e1, slots[2] = e2, slots[3] = f1;
    } else {
        missing = 1;
        slots[0] = e1, slots[2] = f2, slots[3] = e2;
    }
    for (u32 i = 0; i < 4; i++) {
        if (i == missing) continue;
        const u32 corner = at<u16>(triangle, 2 + 2 * slots[i]);
        const u8 *const block = at<const u8 *>(model, 0x68) + (corner >> 2) * 0x20;
        p[i].u = at<float>(block, 4 * (corner & 3));
        p[i].v = at<float>(block, 0x10 + 4 * (corner & 3));
    }
    const auto make = [&](const UV &plus, const UV &minus, const UV &base) {
        p[missing].u = (float)(X87Reg(plus.u) - minus.u + base.u);
        p[missing].v = (float)(X87Reg(plus.v) - minus.v + base.v);
    };
    switch (missing) {
    case 0: make(p[3], p[2], p[1]); break;
    case 1: make(p[2], p[3], p[0]); break;
    case 2: make(p[3], p[0], p[1]); break;
    default: make(p[2], p[1], p[0]); break;
    }
}

u16 corner_flags(const UV *p) {
    float leastU = p[0].u;
    X87Reg leastV(p[0].v);
    for (u32 i = 1; i < 4; i++) {
        if (!(p[i].u >= leastU)) leastU = p[i].u;
        if (leastV.v > (double)p[i].v) leastV = X87Reg(p[i].v);
    }
    float closest = 0x1.fffffep+127f;
    u32 nearest = 0;
    for (u32 i = 0; i < 4; i++) {
        const X87Reg du = X87Reg(p[i].u) - leastU;
        const X87Reg dv = X87Reg(p[i].v) - leastV;
        const X87Reg d = du * (float)du + dv * dv;
        if (!(d.v >= (double)closest)) {
            closest = (float)d;
            nearest = i;
        }
    }
    static const u16 kCorner[4] = {0x80, 0x88, 0x90, 0xa0};
    u16 flags = kCorner[nearest];
    const u32 next = (nearest + 1) & 3;
    const double dv = (X87Reg(p[nearest].v) - X87Reg(p[next].v)).v;
    const double du = (X87Reg(p[nearest].u) - p[next].u).v;
    if (std::fabs(du) > std::fabs(dv)) flags |= 0x40;
    return flags;
}

void fill_cell(const Cell &cell, u16 *out, const u8 *model, const u8 *holder) {
    if (cell.group == -1) return;
    const u8 *const group = at<const u8 *>(model, 0x6c) + ((u32)cell.group << 4);
    const u8 *const triangle = at<const u8 *>(model, 0x70) + ((u32)cell.triangle + at<u16>(group, 4)) * 8;
    if (!(cell.acrossBest > 0.9f) || !(cell.alongBest > 0.9f)) return;
    UV p[4];
    read_corners(cell, model, triangle, p);
    out[0] = corner_flags(p);
    out[1] = (u16)((s32)(at<u32>(group, 0) - at<u32>(holder, 0x50)) >> 3);
}

}  // namespace

void BaseCells_SetEnv(const BaseCellsEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" void __cdecl RideBase_FillCells(const u8 *object, const u8 *grid, u32 width, u32 height, u32 all) {
    const u8 *const marks = g_env->markEdges(grid);
    u8 *const part = at<u8 *>(object, 8);
    const u8 *const holder = at<const u8 *>(part, 4);
    u8 *const model = at<u8 *>(part, 0x2c) ? at<u8 *>(part, 0x2c) : at<u8 *>(holder, 0x78);
    const u16 *const map = at<const u16 *>(model, 0x94);
    at<void *>(part, 0x18) = g_env->alloc(width * height * 4, 1);
    g_env->transform(model);

    Cell cells[400];
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            Cell &cell = cells[y * kRow + x];
            cell.acrossBest = 0.0f;
            cell.alongBest = 0.0f;
            cell.group = -1;
            cell.triangle = -1;
        }
    }

    const u8 *group = at<const u8 *>(model, 0x6c);
    for (u32 g = 0; g < at<u16>(model, 0x5a); g++, group += 0x10) {
        const u8 *triangle = at<const u8 *>(model, 0x70) + at<u16>(group, 4) * 8u;
        for (u32 t = 0; t < at<u16>(group, 6); t++, triangle += 8) place_triangle(cells, marks, group, triangle, (s32)g, (s32)t, map, all);
    }
    g_env->tidy(object, model, 0.02f, marks, global<const float *>(0x7aecd4));

    u32 offset = 0;
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++, offset += 4) {
            u16 *const out = reinterpret_cast<u16 *>(at<u8 *>(part, 0x18) + offset);
            out[0] = 1;
            out[1] = 0x2710;
            if (!all) {
                if (marks && !marks[x * 16 + y]) continue;
                if (!at<u32>(grid, (y * kRow + x) * 8 + 8)) continue;
            }
            fill_cell(cells[y * kRow + x], out, model, holder);
        }
    }
    at<u32>(part, 0) |= 0x40;
}
