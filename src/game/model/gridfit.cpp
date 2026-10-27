#include "gridfit.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/matrix.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830)

// fld a; fcomp b (C0: below or unordered).
bool Below(X87Reg a, double b) { return !(a.v >= b); }

const float kOne = 1.0f, kMinusOne = -1.0f;   // 0x6fe850, 0x6fe864

// The grid's height at a cell index (fld / fstp where it is copied).
const float &Height(const u8 *grid, s32 cell) { return at<const float *>(grid, 0x28)[cell]; }

}  // namespace

extern "C" {

void __cdecl Frame_FitToGrid(const u8 *node, u8 *frame, u16 **cursor, GridFitMatrix matrix, const u8 *grid, u32 place, s32 *hits,
                             float lift, u8 **bestFrame, s32 *bestCount, u32 flags, const u8 *only, u32 fine) {
    const float near = fine ? 0.1f : 0.01f;   // [esp+0x38]
    const float level = fine ? 100.0f : 0.01f;   // [esp+0x74]
    Matrix_PreMul(reinterpret_cast<const float *>(frame + 0x10), matrix.m);
    const u32 frameFlags = at<u32>(frame, 0);
    const bool onGrid = (frameFlags & 0x20000) == 0;   // [esp+0x40]
    bool skip;
    if (!onGrid) {
        skip = (flags & 4) != 0;
    } else {
        skip = (flags & 2) != 0 || (place != 0 && (frameFlags & 0x1000) == 0);
    }
    if (!skip && (frameFlags & 0x200) == 0 && (only == nullptr || only == frame)) {
        float inverse[16];   // (set only when placing, the only use)
        if (place) {
            float scratch[16];
            std::memcpy(inverse, Matrix_Invert(scratch, matrix.m), sizeof inverse);
        }
        s32 count = 0;   // [esp+0x6c]
        for (u32 i = 0; (s32)i < (s32)at<u16>(frame, 0x58); ++i) {
            const bool take = !onGrid || !place || ((at<u32>(frame, 0) & 0x1000) != 0 && i == **cursor);
            if (take) {
                const u32 lane = i & 3, group = (i >> 2) * 0x30;
                const float *const v = reinterpret_cast<const float *>(at<u8 *>(node, 0x60) + group);
                float p[3] = {X87_Through(v[lane]), X87_Through(v[lane + 4]), X87_Through(v[lane + 8])};
                Matrix_TransformPoint(matrix.m, p);
                u32 sides = 0;   // EBX: 1 / 4 the cell's or the next column, 8 / 2 the cell's or the next row
                s32 misses = fine ? 2 : 1;   // [esp+0x58]
                bool snapX = false, snapZ = false;   // [esp+0x44], [esp+0x30]
                s32 cell = 0;   // [esp+0x14]
                u32 column = 0, row = 0;   // [esp+0x4c], [esp+0x48]
                s32 corner = 0;   // [esp+0x68]
                s32 c00 = 0, c01 = 0, c10 = 0, c11 = 0;   // [esp+0x54] (x, z), [esp+0x64] (x + 1, z), [esp+0x5c] (x, z + 1), [esp+0x60]
                float whole = 0, fraction = 0, fracX = 0, fracZ = 0;   // [esp+0x10], [esp+0x24], [esp+0x2c], [esp+0x34]
                for (u32 axis = 0; axis < 2; ++axis) {
                    const X87Reg q = axis == 0 ? (X87Reg(p[0]) - at<float>(grid, 8)) / at<float>(grid, 0x10)
                                               : (X87Reg(p[2]) - at<float>(grid, 0xc)) / at<float>(grid, 0x14);
                    float stored = (float)q;   // [esp+0x28]
                    if (Below(q, 0.0)) stored = 0;
                    double wholeD;
                    const double frac = std::modf((double)stored, &wholeD);   // CRT modf (0x67b280)
                    whole = (float)wholeD;
                    fraction = (float)frac;
                    s32 n;   // ESI
                    if (axis == 0) {
                        corner = Ftol(X87Reg(stored));
                        n = Ftol(X87Reg(whole));
                        fracX = (float)frac;
                        c00 = c10 = n;
                        c01 = c11 = n + 1;
                    } else {
                        const u32 w = at<u32>(grid, 0x18);
                        corner += Ftol(X87Reg(stored)) * (s32)w;
                        const s32 stride = (s32)(w + 1);
                        n = Ftol(X87Reg(whole));
                        c00 += stride * n;
                        c01 += stride * n;
                        const s32 up = Ftol(X87Reg(whole) - kMinusOne) * stride;
                        fracZ = (float)frac;
                        c10 += up;
                        c11 += up;
                    }
                    if (onGrid) {
                        const u32 limit = at<u32>(grid, axis == 0 ? 0x18 : 0x1c) + 1;
                        if (Below(X87Reg(fraction), 0.5)) {
                            if (axis == 0) {
                                sides |= 1;
                                cell = n;
                                column = (u32)n;
                            } else {
                                sides |= 8;
                                cell += (s32)(at<u32>(grid, 0x18) + 1) * n;
                                row = (u32)n;
                            }
                            if (Below(X87Reg(whole), 0.0)) return;
                            if (!((double)limit > (double)whole)) return;   // fild (limit, 0); fcomp whole: below, equal or unordered
                        } else {
                            const X87Reg next = X87Reg(whole) - kMinusOne;
                            if (axis == 0) {
                                sides |= 4;
                                cell = Ftol(next);
                                column = (u32)n;
                            } else {
                                sides |= 2;
                                cell += (s32)(at<u32>(grid, 0x18) + 1) * Ftol(next);
                                row = (u32)n;
                            }
                            if (Below(next, 0.0)) return;
                            if (!(next.v < (double)limit)) return;
                        }
                    }
                    if (place == 0 && onGrid) {
                        if (axis == 0) {
                            const u32 nx = global<u32>(0x007afe18);
                            if (column >= nx) {
                                if (sides & 1) sides = (sides & ~1u) | 4;
                                column = nx - 1;
                            }
                        } else {
                            const u32 nz = global<u32>(0x007b0460);
                            if (row >= nz) {
                                if (sides & 8) sides = (sides & ~8u) | 2;
                                row = nz - 1;
                            }
                        }
                    }
                    const bool nearLine = !(std::fabs((double)fraction) >= near) || !(std::fabs((X87Reg(fraction) - kOne).v) >= near);
                    if (place != 0) {
                        if (onGrid && nearLine) {
                            const bool low = Below(X87Reg(fraction), 0.5);
                            if (axis == 0) {
                                p[0] = low ? (float)(X87Reg(at<float>(grid, 0x10)) * whole + at<float>(grid, 8))
                                           : (float)((X87Reg(whole) - kMinusOne) * at<float>(grid, 0x10) + at<float>(grid, 8));
                                snapX = true;
                            } else {
                                p[2] = low ? (float)(X87Reg(at<float>(grid, 0x14)) * whole + at<float>(grid, 0xc))
                                           : (float)((X87Reg(whole) - kMinusOne) * at<float>(grid, 0x14) + at<float>(grid, 0xc));
                                snapZ = true;
                            }
                        }
                    } else if (nearLine) {
                        (axis == 0 ? snapX : snapZ) = true;
                    } else if (misses != 0) {
                        --misses;
                    }
                }
                if (place != 0) {
                    if (onGrid) {
                        if (fine == 0) {
                            p[1] = X87_Through(Height(grid, cell));
                        } else if (snapX) {
                            const s32 w = (s32)at<u32>(grid, 0x18);
                            X87Reg y = 0.0;
                            if (sides & 2) {
                                const X87Reg a = X87Reg(kOne) - fracZ;
                                y = a * X87Reg(Height(grid, cell - w - 1)) + X87Reg(Height(grid, cell)) * (X87Reg(kOne) - a);
                            } else {
                                const X87Reg a = X87Reg(kOne) - fracZ;
                                y = X87Reg(Height(grid, cell)) * a + X87Reg(Height(grid, cell + w + 1)) * fracZ;
                            }
                            p[1] = (float)(y - lift + p[1]);
                        } else {
                            X87Reg y = 0.0;
                            const X87Reg b = X87Reg(kOne) - fracX;
                            if (sides & 4) {
                                y = b * X87Reg(Height(grid, cell - 1)) + (X87Reg(kOne) - b) * Height(grid, cell);
                            } else {
                                y = X87Reg(Height(grid, cell)) * b + X87Reg(Height(grid, cell + 1)) * fracX;
                            }
                            p[1] = (float)(y - lift + p[1]);
                        }
                    } else {
                        bool upper, other;   // EAX, ECX
                        if (at<const u8>(at<const u8 *>(grid, 0x2c), (u32)corner * 4) & 4) {
                            upper = !Below(X87Reg(fracX), (X87Reg(kOne) - fracZ).v);
                            other = upper;
                        } else {
                            upper = !Below(X87Reg(fracX), (double)fracZ);
                            other = !upper;
                        }
                        const X87Reg d = upper ? X87Reg(Height(grid, c11)) - Height(grid, c01) : X87Reg(Height(grid, c10)) - Height(grid, c00);
                        const X87Reg e = X87Reg(kOne) - fracX;
                        X87Reg y = 0.0;
                        if (other) {
                            const X87Reg s = e * Height(grid, c10) + X87Reg(Height(grid, c11)) * fracX;
                            y = s + (X87Reg(kOne) - fracZ) * X87Reg(-d.v);
                        } else {
                            const X87Reg s = e * Height(grid, c00) + X87Reg(Height(grid, c01)) * fracX;
                            y = s + X87Reg(fracZ) * d;
                        }
                        p[1] = (float)(y - lift + p[1]);
                    }
                } else if (onGrid && misses != 0) {
                    bool hit = false;
                    if (!(std::fabs((double)p[1]) >= level)) {
                        if (flags & 1) {
                            hit = true;
                        } else {
                            const u32 edge = column + row * 20;
                            const u8 *const marks = reinterpret_cast<const u8 *>(0x007afe20);
                            if (snapX && (sides & 1) && (marks[edge * 4] & 2)) hit = true;
                            if (snapZ && (sides & 2) && (marks[edge * 4] & 4)) hit = true;
                            if (snapX && (sides & 4) && (marks[edge * 4] & 8)) hit = true;
                            if (snapZ && (sides & 8) && (marks[edge * 4] & 1)) hit = true;
                        }
                    }
                    if (hit) {
                        ++count;
                        at<u32>(frame, 0) |= 0x1000;
                        ++*hits;
                        **cursor = (u16)i;
                        ++*cursor;
                    }
                }
                if (bestCount != nullptr && count > *bestCount) {
                    *bestCount = count;
                    *bestFrame = frame;
                }
                if (place != 0) {
                    Matrix_TransformPoint(inverse, p);
                    float *const out = reinterpret_cast<float *>(at<u8 *>(frame, 0x60) + group);
                    out[lane] = X87_Through(p[0]);
                    out[lane + 4] = X87_Through(p[1]);
                    out[lane + 8] = X87_Through(p[2]);
                    for (u32 k = 0; k < 3; ++k) {
                        float &low = at<float>(frame, 0x78 + 4 * k), &high = at<float>(frame, 0x84 + 4 * k);
                        if (Below(X87Reg(p[k]), (double)low)) {
                            low = p[k];
                        } else if ((double)p[k] > (double)high) {   // (test ah, 0x41: not below, equal or unordered)
                            high = p[k];
                        }
                    }
                    if (at<u32>(frame, 0) & 0x1000) ++*cursor;
                }
            }
        }
    }
    const u8 *child = at<const u8 *>(node, 0xc);
    u8 *childFrame = at<u8 *>(frame, 0xc);
    if (child == nullptr || childFrame == nullptr) return;
    while (child != nullptr) {
        Frame_FitToGrid(child, childFrame, cursor, matrix, grid, place, hits, lift, bestFrame, bestCount, flags, only, fine);
        child = at<const u8 *>(child, 8);
        childFrame = at<u8 *>(childFrame, 8);
    }
}

void __cdecl Model_FitToGrid(const u8 *object, const u8 *owner, u32 lift, u32 flags) {
    const u8 *const root = at<const u8 *>(object, 4);
    const u8 *const holder = at<const u8 *>(root, 0x9c);
    if (owner == nullptr || at<const u8 *>(owner, 0x6c) == nullptr) return;
    GridFitMatrix identity;
    Matrix_Identity(identity.m);
    u16 *cursor = at<u16 *>(at<const u8 *>(object, 8), 0x14);
    if (at<u32>(root, 0x30) & 0x100) flags |= 1;
    float liftF;
    std::memcpy(&liftF, &lift, 4);
    Frame_FitToGrid(at<const u8 *>(holder, 0x78), at<u8 *>(root, 0x78), &cursor, identity, at<const u8 *>(owner, 0x6c), 1, nullptr, liftF,
                    nullptr, nullptr, flags, nullptr, 0);
    const u8 *part = at<const u8 *>(root, 0x70);
    for (u32 n = at<u16>(root, 0x44); n != 0; --n, part += 0xa0) {
        const u8 *const sub = at<const u8 *>(part, 0x98);
        if (sub == nullptr) continue;
        cursor = at<u16 *>(sub, 4);
        Frame_FitToGrid(at<const u8 *>(holder, 0x78), at<u8 *>(root, 0x78), &cursor, identity, at<const u8 *>(owner, 0x6c), 1, nullptr, liftF,
                        nullptr, nullptr, flags, part, 1);
    }
    at<u32>(at<u8 *>(root, 0x78), 0) |= 0x10000;
    at<u32>(const_cast<u8 *>(root), 0x30) |= 0x40000;
}
}
