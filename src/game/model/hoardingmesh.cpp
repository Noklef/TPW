#include "hoardingmesh.h"

#include <cmath>
#include <cstdint>

#include "../core/crt.h"
#include "../core/globalmem.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "holder.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl PortFormat(char *to, const char *format, const char *level) { return Crt().sprintf(to, format, level); }

const HoardingMeshEnv kPortEnv = {&GlobalMem_Alloc, &PortFormat, &Animator_Create};
const HoardingMeshEnv *g_env = &kPortEnv;

// The constants, as the game's memory holds them: 0.5, -0.5, -1, -5 pi / 4, 2 pi, -10, 5, 10, the most a float can be, 0.
const float &kHalf = gHoardingmesh_6fe444;
const float &kLessHalf = gHoardingmesh_6fe448;
const float &kLessOne = gHoardingmesh_6fe44c;
const float &kAngle = gHoardingmesh_6fe450;
const double &kTurn = gHoardingmesh_6fe458;
const float &kLessTen = gHoardingmesh_6fe460;
const float &kFive = gHoardingmesh_6fe464;
const float &kTen = gHoardingmesh_6fe468;
const float &kFar = gHoardingmesh_6fe46c;
const float &kNone = gHoardingmesh_6fe470;

const u32 kOne = 0x3f800000, kLessOneBits = 0xbf800000;

u8 CellByte(const u8 *grid, s32 x, s32 y) { return at<u8>(grid, 0xc + (u32)(y * 0x14 + x) * 8); }

float Bearing(X87Reg x, X87Reg y) { return (float)std::fmod((X87Reg(X87_Atan2(y.v, x.v)) - kAngle).v, kTurn); }

// A board's vertices: the x and z of each of its four, y 10, 10, 0, 0.
void Vertices(const u8 *animator, u32 slot, float x0, float x1, float z0, float z1) {
    u8 *const v = at<u8 *>(animator, 0x10) + slot * 0x30;
    const float xs[4] = {x0, x1, x0, x1}, zs[4] = {z0, z1, z0, z1};
    for (u32 k = 0; k < 4; ++k) {
        at<float>(v, k * 4) = xs[k];
        at<u32>(v, 0x10 + k * 4) = k < 2 ? 0x41200000 : 0;
        at<float>(v, 0x20 + k * 4) = zs[k];
    }
}

u32 Back(u32 bits) { return bits ? bits ^ 0x80000000 : 0; }

// Its normal four times, and its back's once after all the boards' four.
void Normals(const u8 *animator, u32 slot, u32 boards, u32 x, u32 z) {
    u8 *const normals = at<u8 *>(animator, 0x14);
    for (u32 k = 0; k < 4; ++k) {
        u8 *const n = normals + (slot * 4 + k) * 0xc;
        at<u32>(n, 0) = x;
        at<u32>(n, 4) = 0;
        at<u32>(n, 8) = z;
    }
    u8 *const back = normals + (slot + boards * 4) * 0xc;
    at<u32>(back, 0) = Back(x);
    at<u32>(back, 4) = 0;
    at<u32>(back, 8) = Back(z);
}

void KeepBits(u8 *word, u32 value) { at<u16>(word, 0) = (u16)((at<u16>(word, 0) & 0x8000) | (value & 0x7fff)); }

void Triangle(u8 *entry, u32 normal, u32 a, u32 b, u32 c) {
    KeepBits(entry, normal);
    at<u16>(entry, 2) = (u16)a;
    at<u16>(entry, 4) = (u16)b;
    at<u16>(entry, 6) = (u16)c;
}

}  // namespace

void HoardingMesh_SetEnv(const HoardingMeshEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" void __cdecl HoardingMesh_Build(const char *level, u8 *object, const u8 *grid) {
    if (at<u16>(at<u8 *>(object, 8), 0xc) < 2) return;
    const s32 width = at<s32>(grid, 0), height = at<s32>(grid, 4);
    const float halfWidth = (float)(X87Reg((double)width) * kHalf), halfHeight = (float)(X87Reg((double)height) * kHalf);
    const bool single = width == 1 && height == 1;

    // (The frame's arrays, as big as the original's: 20 by 20 cells of four sides.)
    float angles[1600];
    s32 pointers[1600], sorted[1600], slots[1600];
    // Sic, not kept: the original's table is this size too and nothing limits it. A mesh of over 1023 points runs off its frame, here off this one.
    float points[1023][3];
    char folder[0x104];

    s32 boards = 0;
    for (s32 y = 0; y < height; ++y) {
        for (s32 x = 0; x < width; ++x) {
            const u8 cell = CellByte(grid, x, y);
            const X87Reg fx = (double)x, fy = (double)y;
            float *const angle = angles + y * 0x50 + x * 4;
            if (cell & 1) {
                angle[0] = Bearing((fx - kLessHalf) - halfWidth, (fy - kLessOne) - halfHeight);
                pointers[boards++] = (s32)(uintptr_t)&angle[0];
            }
            if (cell & 4) {
                angle[1] = Bearing((fx - kLessOne) - halfWidth, (fy - kLessHalf) - halfHeight);
                pointers[boards++] = (s32)(uintptr_t)&angle[1];
            }
            if (cell & 0x10) {
                angle[2] = Bearing((fx - kLessHalf) - halfWidth, fy - halfHeight);
                pointers[boards++] = (s32)(uintptr_t)&angle[2];
            }
            if (cell & 0x40) {
                angle[3] = Bearing(fx - halfWidth, (fy - kLessHalf) - halfHeight);
                pointers[boards++] = (s32)(uintptr_t)&angle[3];
            }
        }
    }
    Sort_MergeByKey(boards, pointers, sorted, 0, nullptr);

    for (s32 y = 0; y < height; ++y) {
        for (s32 x = 0; x < width; ++x) {
            const u8 cell = CellByte(grid, x, y);
            for (s32 side = 0; side < 4; ++side) {
                if (!(cell & (1 << (side * 2)))) continue;
                const s32 index = y * 0x50 + x * 4 + side;
                for (s32 k = 0; k < boards; ++k)
                    if (sorted[k] == (s32)(uintptr_t)&angles[index]) slots[index] = k;
            }
        }
    }

    u8 *const animator = static_cast<u8 *>(g_env->alloc(0x40, 1));
    u8 *const part = static_cast<u8 *>(g_env->alloc(0x30, 1));
    u8 *const edges = static_cast<u8 *>(g_env->alloc((u32)boards * 8, 1));
    const u32 vertices = (u32)boards * 4;
    at<u32>(animator, 0) = 3;
    at<u16>(animator, 4) = 1;
    at<u8 *>(animator, 8) = part;
    at<u16>(part, 4) = at<u16>(part, 6) = at<u16>(part, 8) = (u16)vertices;
    at<u32>(part, 0x10) = at<u32>(part, 0x18) = 0xc0a00000;
    at<u32>(part, 0) = 4;
    at<u16>(part, 0xc) = (u16)boards;
    at<u16>(part, 0xa) = 1;
    at<u32>(part, 0x14) = 0xc1700000;
    at<u32>(part, 0x20) = 0x41c80000;
    at<float>(part, 0x1c) = (float)(kFive - X87Reg((double)width) * kLessTen);
    at<float>(part, 0x24) = (float)(kFive - X87Reg((double)height) * kLessTen);

    g_env->format(folder, gHoardingMesh_Build_74cfc0, level);
    gHoardingMesh_Build_74d010 = folder;
    g_env->create(animator, reinterpret_cast<u8 *>((uintptr_t)0x0074d008), gText_HoardingMesh_Build_74d080, 1);
    Matrix_Identity(at<float *>(part, 0x28));
    u8 *const range = at<u8 *>(part, 0x2c);
    at<u32>(range, 0) = gHoardingMesh_Build_74d020;
    at<u16>(range, 4) = 0;
    at<u16>(range, 6) = at<u16>(part, 4);
    at<u16>(range, 8) = 0;
    at<u16>(range, 0xa) = boards ? (u16)(vertices - 1) : 0;

    const u8 *const mesh = at<u8 *>(at<u8 *>(object, 8), 0x2c);
    if (!mesh) return;
    const s32 count = at<u16>(mesh, 0x58);
    for (s32 i = 0; i < count; ++i) {
        const u8 *const block = at<u8 *>(mesh, 0x60) + (u32)(i >> 2) * 0x30;
        const u32 k = (u32)i & 3;
        const float kept = (float)X87Reg(at<float>(block, k * 4)), keptZ = (float)X87Reg(at<float>(block, 0x20 + k * 4));
        const X87Reg px = at<float>(block, k * 4), py = at<float>(block, 0x10 + k * 4), pz = at<float>(block, 0x20 + k * 4);
        points[i][1] = (float)(((px * at<float>(mesh, 0x14) + py * at<float>(mesh, 0x24)) + X87Reg(keptZ) * at<float>(mesh, 0x34)) + at<float>(mesh, 0x44));
        points[i][2] = (float)(((px * at<float>(mesh, 0x18) + py * at<float>(mesh, 0x28)) + pz * at<float>(mesh, 0x38)) + at<float>(mesh, 0x48));
        points[i][0] = (float)(((py * at<float>(mesh, 0x20) + pz * at<float>(mesh, 0x30)) + X87Reg(at<float>(mesh, 0x10)) * kept) + at<float>(mesh, 0x40));
    }

    // Sic, not kept: with under 2 points no corner finds a second one, so the original sets only the first move (to 0, 0) and uses the other six as what its frame held. 0 here.
    float moves[8] = {};
    for (s32 y = 0; y < height; ++y) {
        const float yg = (float)(X87Reg((double)y) * kTen);
        const float yn = (float)(X87Reg(yg) - kLessTen);
        for (s32 x = 0; x < width; ++x) {
            const float xg = (float)(X87Reg((double)x) * kTen);
            const float xn = (float)(X87Reg(xg) - kLessTen);
            if (single) {
                const u32 plus = 0x3e99999a, minus = 0xbe99999a;  // 0.3, -0.3
                const u32 fixed[8] = {plus, minus, minus, minus, plus, plus, plus, plus};
                for (u32 m = 0; m < 8; ++m) at<u32>(moves, m * 4) = fixed[m];
            } else {
                const float corners[4][2] = {{xg, yn}, {xn, yn}, {xn, yg}, {xg, yg}};
                for (u32 j = 0; j < 4; ++j) {
                    X87Reg best = kFar;
                    float secondDistance;
                    at<u32>(&secondDistance, 0) = 0x7f7fffff;
                    s32 first = -1, second = -1;
                    for (s32 k = 0; k < count; ++k) {
                        const X87Reg dx = X87Reg(points[k][0]) - corners[j][0];
                        const X87Reg dz = X87Reg(points[k][2]) - corners[j][1];
                        const float distance = (float)X87_Sqrt(dx * dx + dz * dz);
                        if (!(distance >= best.v)) {
                            second = first;
                            first = k;
                            secondDistance = (float)best;
                            best = distance;
                        } else if (!(X87Reg(distance).v >= secondDistance)) {
                            second = k;
                            secondDistance = distance;
                        }
                    }
                    if (second != -1) {
                        moves[j * 2] = (float)((X87Reg(points[second][0]) - corners[j][0]) * kHalf);
                        moves[j * 2 + 1] = (float)((X87Reg(points[second][2]) - corners[j][1]) * kHalf);
                    } else {
                        moves[0] = moves[1] = 0;  // (sic: these two whichever the corner)
                    }
                }
            }

            const u8 cell = CellByte(grid, x, y);
            const u32 up = (u32)(y + 1) & 0xffff, row = (u32)y & 0xffff, right = (u32)(x + 1) << 16, left = (u32)x << 16;
            const s32 index = y * 0x50 + x * 4;
            const auto entry = [&](u32 slot, u32 a, u32 b) {
                at<u32>(edges, slot * 8) = a;
                at<u32>(edges, slot * 8 + 4) = b;
            };
            if (cell & 1) {
                const u32 slot = (u32)slots[index];
                const float side = cell & 4 ? moves[2] : 0.0f;
                const X87Reg reach = X87Reg(cell & 0x40 ? moves[0] : kNone);
                const float z = (float)((X87Reg(moves[1]) + X87Reg(yg)) - kLessTen);
                Vertices(animator, slot, (float)((X87Reg(side) + xg) - kLessTen), (float)(reach + xg), z, z);
                Normals(animator, slot, (u32)boards, 0, kOne);
                entry(slot, right | up, left | up);
            }
            if (cell & 4) {
                const u32 slot = (u32)slots[index + 1];
                const float side = cell & 0x10 ? moves[5] : 0.0f;
                const X87Reg reach = X87Reg(cell & 1 ? moves[3] : kNone);
                const float x0 = (float)((X87Reg(moves[2]) + xg) - kLessTen);
                Vertices(animator, slot, x0, x0, (float)(X87Reg(side) + X87Reg(yg)), (float)((reach + X87Reg(yg)) - kLessTen));
                Normals(animator, slot, (u32)boards, kOne, 0);
                entry(slot, right | row, right | up);
            }
            if (cell & 0x10) {
                const u32 slot = (u32)slots[index + 2];
                const float side = cell & 0x40 ? moves[6] : 0.0f;
                const X87Reg reach = X87Reg(cell & 4 ? moves[4] : kNone);
                const float z = (float)(X87Reg(moves[7]) + X87Reg(yg));
                Vertices(animator, slot, (float)(X87Reg(side) + xg), (float)((reach + xg) - kLessTen), z, z);
                Normals(animator, slot, (u32)boards, 0, kLessOneBits);
                entry(slot, left | row, right | row);
            }
            if (cell & 0x40) {
                const u32 slot = (u32)slots[index + 3];
                const float side = cell & 1 ? moves[1] : 0.0f;
                const X87Reg reach = X87Reg(cell & 0x10 ? moves[7] : kNone);
                const float x0 = (float)(X87Reg(moves[0]) + xg);
                Vertices(animator, slot, x0, x0, (float)((X87Reg(side) + X87Reg(yg)) - kLessTen), (float)(reach + X87Reg(yg)));
                Normals(animator, slot, (u32)boards, kLessOneBits, 0);
                entry(slot, left | up, left | row);
            }
        }
    }

    for (u32 i = 0; i < (u32)boards; ++i) {
        u8 *const uv = at<u8 *>(animator, 0x18) + i * 0x20;
        for (u32 k = 0; k < 4; ++k) {
            at<u32>(uv, k * 4) = k & 1 ? kOne : 0;
            at<u32>(uv, 0x10 + k * 4) = k < 2 ? kOne : 0;
        }
        u8 *const triangles = at<u8 *>(animator, 0x1c) + i * 0x20;
        const u32 v = i * 4, back = vertices + i;
        Triangle(triangles, v, v, v + 1, v + 2);
        Triangle(triangles + 8, v, v + 1, v + 3, v + 2);
        Triangle(triangles + 0x10, back, v + 2, v + 1, v);
        Triangle(triangles + 0x18, back, v + 2, v + 3, v + 1);
    }

    u8 *const info = static_cast<u8 *>(g_env->alloc(0x10, 1));
    at<s32>(info, 0) = boards;
    at<s32>(info, 4) = at<s32>(grid, 0);
    at<s32>(info, 8) = at<s32>(grid, 4);
    at<u8 *>(info, 0xc) = edges;
    at<u8 *>(object, 0xb0) = info;
    at<u32>(object, 4) |= 2;
    at<u8 *>(object, 0xb4) = animator;
}
