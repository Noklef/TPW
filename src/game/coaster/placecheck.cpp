#include "placecheck.h"

#include <cmath>
#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *Ptr(const void *p, u32 offset) { return at<u8 *>(p, offset); }

bool Below(X87Reg a, double b) { return !(a.v >= b); }   // fcomp: C0, below or unordered

// The CRT's acos (0x67b4e0) as the game calls it (see motion/quat.cpp): under single precision 1 + x, 1 - x, their product and the
// square root round to float; fpatan's result leaves as a double.
double CrtAcos(float x) {
    const float r = std::sqrt((1.0f + x) * (1.0f - x));
    return std::atan2((double)r, (double)x);
}

const float kOne = 1.0f;   // 0x6fde70

u32 EditorFlags() { return global<u32>(0x0079c658); }

// The angle at the piece's tile (px, pz) between the tile (x, z) and the next tile (nx, nz), within the block's limits for the
// distance (d2 its square). The registers' order is the original's.
bool AngleFits(const u8 *from, const u8 *block, u32 x, u32 z, u32 px, u32 pz, u32 nx, u32 nz, u32 d2) {
    const X87Reg oz = (double)pz;
    const float oxf = (float)(double)px;   // [esp+0x18]
    const X87Reg dx1 = X87Reg((double)x) - oxf, dx2 = X87Reg((double)nx) - oxf;
    const X87Reg dz1 = X87Reg((double)z) - oz, dz2 = X87Reg((double)nz) - oz;
    const float dx1f = (float)dx1.v, dx2f = (float)dx2.v;   // [esp+0x18], [esp+0x28]
    const double dx1d = dx1.v, dx2d = dx2.v;   // [esp+0x30], [esp+0x38]
    const X87Reg sx1 = X87_MulDouble(dx1.v, dx1d);
    const float dz1f = (float)dz1.v;   // [esp+0x1c]
    const X87Reg sz1 = dz1 * dz1f;
    const float dz2f = (float)dz2.v;   // [esp+0x2c]
    const X87Reg sx2 = X87_MulDouble(dx2.v, dx2d);
    const X87Reg n1 = sx1 + sz1;
    const X87Reg sz2 = dz2 * dz2f;
    const X87Reg len1 = X87_Sqrt(n1);
    const X87Reg n2 = sx2 + sz2;
    const X87Reg len2 = X87_Sqrt(n2);
    const float len1f = (float)len1.v, len2f = (float)len2.v;   // [esp+0x50] in turn
    const X87Reg u1x = X87Reg(dx1f) / len1f, u1z = X87Reg(dz1f) / len1f;
    const X87Reg p1 = u1x * (X87Reg(dx2f) / len2f);
    const X87Reg p2 = u1z * (X87Reg(dz2f) / len2f);
    float dot = (float)(p1 + p2).v;
    if (!(dot >= -1.0f)) dot = -1.0f;   // (a NaN too)
    if (dot > 1.0f) dot = 1.0f;
    const float angle = (float)CrtAcos(dot);   // stored, and taken back with fabs
    const float near = at<float>(block, 0xa4);
    const X87Reg span = X87Reg(at<float>(block, 0xa8)) - near;
    const X87Reg tr = (X87_Sqrt(X87Reg((double)d2)) - near) / span;
    float t = (float)tr.v;
    if (!Below(tr, 1.0)) t = 1.0f;
    if (!(t > 0.0f)) t = 0.0f;   // (below, equal or unordered)
    const X87Reg a = std::fabs((double)angle);
    const u32 q = (at<u8>(Ptr(from, 0x100), 4) & 2) ? 0x74 : 0x84;
    const X87Reg m = X87Reg(kOne) - t;
    const X87Reg lo = X87Reg(at<float>(block, q)) * m + X87Reg(at<float>(block, q + 8)) * t;
    const X87Reg hi = X87Reg(at<float>(block, q + 4)) * m + X87Reg(at<float>(block, q + 0xc)) * t;
    if (Below(a, lo.v)) return false;
    return !(a.v > (double)(float)hi.v);
}

// A node's count under its rules' and its object's flags clear of mask.
bool UnderCount(const u8 *node, u32 mask) {
    if (at<u32>(node, 0x30) >= at<u32>(Ptr(node, 0x18), 0xcc)) return false;
    return !(at<u32>(Ptr(node, 0x34), 8) & mask);
}

}  // namespace

extern "C" {

s32 __cdecl CoasterPiece_CanPlaceAt(u32 x, u32 z, u8 *piece, u32 px, u32 pz, u32 nx, u32 nz) {
    u8 *const coaster = global<u8 *>(0x00790fec);
    u8 *const selected = global<u8 *>(0x0079c6a4);
    s32 ok = 1;   // [esp+0x10]
    u8 *held = nullptr;   // [esp+0x14]
    u8 *from;   // EBP
    if (piece) {
        from = piece;
    } else {
        nx = 0;   // sic: the original leaves its slot unset (see the header)
        const u32 flags = EditorFlags();
        if (at<u8>(coaster, 0x3c) & 2) {
            if (flags & 0x100) {
                u8 *const next = Ptr(selected, 0x68);
                from = (at<u32>(next, 8) & 0x10000) ? nullptr : next;
                held = selected;
            } else {
                from = Ptr(coaster, 0x60);
            }
        } else if (flags & 0x100) {
            from = Ptr(selected, 0x68);
            held = selected;
        } else {
            from = Ptr(coaster, 0x58);
        }
        if (from) {
            const u8 *node = Ptr(from, 0x100);
            px = at<u32>(node, 0xc);
            pz = at<u32>(node, 0x10);
            if (Ptr(from, 0x68)) {
                node = Ptr(Ptr(from, 0x68), 0x100);
                nx = at<u32>(node, 0xc);
                nz = at<u32>(node, 0x10);
            }
        }
    }
    const u8 *const map = Ptr(global<u8 *>(0x007a0854), 0x6c);
    const u32 width = at<u32>(map, 0x18);
    if (x >= width || z >= at<u32>(map, 0x1c)) return 0;
    if (!piece && (at<u8>(Ptr(global<u8 *>(0x0079fc54), 0x2c), (z * width + x) * 4) & 1)) return 0;
    const u32 moving = EditorFlags() & 0x100;   // [esp+0x20]
    u8 *before = nullptr;   // ESI, [esp+0x58]
    u8 *after;   // EDX
    if ((moving || piece) && from) {
        before = Ptr(from, 0x64) ? Ptr(from, 0x64) : Ptr(Ptr(coaster, 0x54), 0x64);
        after = Ptr(from, 0x68) ? Ptr(from, 0x68) : Ptr(coaster, 0x58);
    } else {
        after = from ? Ptr(from, 0x68) : nullptr;
    }
    if (after && moving) {
        const u32 f = at<u32>(after, 8);
        if ((f & 0x10000) || (at<u32>(from, 8) & 0x10000) || (at<u32>(before, 8) & 0x10000))
            if ((f & 0x20000) || (at<u32>(from, 8) & 0x20000) || (at<u32>(before, 8) & 0x20000)) after = nullptr;
    }
    u8 *const type = Ptr(coaster, 4);
    const u8 *block = nullptr;   // ESI
    if (!moving && !piece)
        block = type + ((at<u8>(Ptr(coaster, 0x60), 8) & 0x40) || (EditorFlags() & 0x40) ? 0x1c8 : 0xa8);
    else if (from)
        block = type + ((at<u8>(from, 8) & 0x40) || (EditorFlags() & 0x40) ? 0x1c8 : 0xa8);
    if (from) {
        if (px == x && pz == z) {
            ok = 0;
        } else {
            const s32 dx = (s32)(px - x), dz = (s32)(pz - z);
            const u32 d2 = (u32)(dx * dx) + (u32)(dz * dz);
            if ((d2 < at<u32>(block, 0x9c) || d2 > at<u32>(block, 0xa0)) &&
                (at<u32>(from, 0) == 2 || !(at<u8>(Ptr(from, 0x100), 4) & 2)))
                ok = 0;
            else if (after && !AngleFits(from, block, x, z, px, pz, nx, nz, d2))
                ok = 0;
        }
    }
    const u8 *rules;   // EDI
    if (!moving && !piece)
        rules = type + ((EditorFlags() & 0x40) ? 0x1c8 : 0xa8);
    else if (from)
        rules = Ptr(Ptr(before, 0x100), 0x18);
    else
        rules = Ptr(Ptr(selected, 0x100), 0x18);
    if (ok) {
        const u32 tile = z * width + x;
        const u8 *found = nullptr;
        for (const u8 *link = global<u8 *>(0x00790fe0); link && !found; link = Ptr(link, 0x18))
            for (found = Ptr(link, 0x4c); found; found = Ptr(found, 0x28))
                if (!(at<u8>(found, 4) & 5) && at<u32>(found, 8) == tile) break;
        if (found && !piece) {
            if (Ptr(found, 0x14) != coaster) {
                ok = 0;
            } else if (moving) {
                if (Ptr(found, 0x18) != rules)
                    ok = 0;
                else if (selected && Ptr(selected, 0x100) == found)
                    ok = at<u32>(found, 0x30) > at<u32>(Ptr(found, 0x18), 0xcc) || (at<u32>(Ptr(found, 0x34), 8) & 0x600) ? 0 : 1;
                else if (!UnderCount(found, 0x600))
                    ok = 0;
            } else if (at<u8>(coaster, 0x3c) & 2) {
                u8 *const own = Ptr(coaster, 0x64);
                if (at<u32>(Ptr(own, 0x100), 8) == tile) {
                    if (global<u32>(0x0079c664) == 1) ok = 0;
                    held = own;
                } else if (Ptr(found, 0x18) != rules || !UnderCount(found, 0x600)) {
                    ok = 0;
                }
            } else {
                if (!(at<u8>(coaster, 0x3c) & 1)) {
                    u8 *const own = Ptr(coaster, 0x54);
                    if (tile == at<u32>(Ptr(own, 0x100), 8)) held = own;
                }
                if (Ptr(found, 0x18) != rules || !UnderCount(found, 0x200)) ok = 0;
            }
        }
    }
    if (!held) return ok;
    u8 *const back = Ptr(held, 0x64);
    if (!back || (at<u32>(held, 8) & 0x10000)) return ok;
    const u8 *const heldNext = Ptr(Ptr(held, 0x68) ? Ptr(held, 0x68) : Ptr(coaster, 0x58), 0x100);
    const u8 *const backNode = Ptr(back, 0x100);
    const u32 bx = at<u32>(backNode, 0xc), bz = at<u32>(backNode, 0x10);
    s32 result = 0;
    if (ok && CoasterPiece_CanPlaceAt(bx, bz, held, x, z, at<u32>(heldNext, 0xc), at<u32>(heldNext, 0x10))) result = 1;
    u8 *const back2 = Ptr(held, 0x64);
    u8 *before2 = Ptr(back2, 0x64);
    if ((EditorFlags() & 0x100) && (at<u8>(coaster, 0x3c) & 1) && !before2) before2 = Ptr(Ptr(coaster, 0x54), 0x64);
    if (!before2 || (at<u32>(back2, 8) & 0x10000)) return result;
    const u8 *const node2 = Ptr(before2, 0x100);
    if (!result) return 0;
    return CoasterPiece_CanPlaceAt(at<u32>(node2, 0xc), at<u32>(node2, 0x10), back2, bx, bz, x, z) ? 1 : 0;
}
}
