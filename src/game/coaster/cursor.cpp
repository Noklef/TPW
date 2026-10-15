#include "cursor.h"

#include "train.h"
#include "../math/x87.h"

#include <cmath>
#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u32 nodes(u32 piece) { return at<u32>(at<const u8 *>((const void *)piece, 0x90), 0); }
u32 next(u32 piece) { return at<u32>((const void *)piece, 0x64); }
u32 prev(u32 piece) { return at<u32>((const void *)piece, 0x68); }
const u8 *node(u32 piece, u32 index) { return at<const u8 *>(at<const u8 *>((const void *)piece, 0x90), 4) + index * 0x58; }

// |a - b| (positions at node +0x40) with y scaled by 0.7f (0x6fded8), and b - a to vec when
// given. sic: dy is squared by its float rounding, dz is squared rounded to float.
float Gap(const u8 *a, const u8 *b, float *vec) {
    const float *p = &at<const float>(a, 0x40), *q = &at<const float>(b, 0x40);
    const X87Reg dx = X87Reg(p[0]) - q[0];
    const X87Reg dy = X87Reg(p[1]) * 0.7f - X87Reg(q[1]) * 0.7f;
    const float dz = (float)(X87Reg(p[2]) - q[2]).v;
    const X87Reg sum = (dx * dx + dy * (float)dy.v) + X87Reg(dz) * dz;
    const X87Reg len = sum.v != sum.v ? sum.v : X87_Round(std::sqrt(sum.v));
    if (vec) {
        for (int k = 0; k < 3; ++k) vec[k] = (float)(X87Reg(q[k]) - p[k]).v;
    }
    return (float)len.v;
}

enum { kStart = 0, kStartPiece = 1, kEnd = 2, kEndPiece = 3 };

}  // namespace

extern "C" {

void __cdecl TrackCursor_Next(u32 *c) {
    if (c[kStartPiece] == c[kEndPiece]) {
        if (c[kEnd] < nodes(c[kStartPiece]) - 1) {
            ++c[kStart];
            ++c[kEnd];
        } else {
            c[kEnd] = 0;
            c[kEndPiece] = next(c[kEndPiece]);
            ++c[kStart];
        }
        return;
    }
    const u32 piece = c[kEndPiece];
    c[kStartPiece] = piece;
    if (nodes(piece) > 1) {
        c[kStart] = 0;
        ++c[kEnd];
    } else {
        c[kEndPiece] = next(piece);
        c[kStart] = nodes(c[kStartPiece]) - 1;
    }
}

void __cdecl TrackCursor_Prev(u32 *c) {
    if (c[kStartPiece] == c[kEndPiece]) {
        if (c[kStart] != 0) {
            --c[kStart];
            --c[kEnd];
        } else {
            c[kStartPiece] = prev(c[kStartPiece]);
            c[kEnd] = 0;
            c[kStart] = nodes(c[kStartPiece]) - 1;
        }
        return;
    }
    const u32 piece = c[kStartPiece];
    c[kEndPiece] = piece;
    if (nodes(piece) > 1) {
        c[kEnd] = c[kStart];
        --c[kStart];
    } else {
        c[kStartPiece] = prev(c[kStartPiece]);
        c[kStart] = nodes(c[kStartPiece]) - 1;
    }
}

void __cdecl TrackCursor_Point(const u32 *c, float t, Vec3 *out) {
    TrackSpline_Point(node(c[kStartPiece], c[kStart]), node(c[kEndPiece], c[kEnd]), t, out);
}

u32 __cdecl Track_PieceAt(const u8 *track, u32 n) {
    u32 piece = at<u32>(track, 0x18);
    for (; n != 0; --n) piece = next(piece);
    return piece;
}

u32 __cdecl TrackSelection_ClearFlags() {
    u8 *a = *reinterpret_cast<u8 *const *>(0x0079c628), *b = *reinterpret_cast<u8 *const *>(0x0079c62c);
    if (!a) return 0;
    u8 *from = b, *to = a;
    if (at<u32>(b, 0) > at<u32>(a, 0)) from = a, to = b;
    for (u8 *piece = from; piece != to; piece = at<u8 *>(piece, 0x64)) at<u32>(piece, 8) &= 0xfeffbfff;
    return 1;
}

void __cdecl TrackPieces_Renumber(u8 *piece) {
    if (!piece) return;
    const u8 *prev = at<const u8 *>(piece, 0x68);
    u32 n = prev ? at<u32>(prev, 0) + 1 : 0;
    for (; piece; piece = at<u8 *>(piece, 0x64), ++n) {
        at<u32>(piece, 0) = n;
        at<float>(piece, 0x6c) = (float)(double)n;
    }
}

void __cdecl TrackSelection_SetFirst(u8 *piece, u32 keep) {
    u8 *&first = *reinterpret_cast<u8 **>(0x0079c628);
    u32 changed = keep ? first != piece : TrackSelection_ClearFlags();
    first = piece;
    if (piece) {
        u8 *other = *reinterpret_cast<u8 *const *>(0x0079c62c), *from = other, *to = piece;
        if (at<u32>(other, 0) > at<u32>(piece, 0)) from = piece, to = other;
        for (u8 *p = from; p != to; p = at<u8 *>(p, 0x64)) at<u32>(p, 8) |= 0x1000000;
        changed = 1;
    }
    if (changed) Flags_SetChanged(*reinterpret_cast<u32 *const *>(0x00790fec), 0);
}

void __cdecl TrackSelection_Clear() {
    const u32 changed = TrackSelection_ClearFlags();
    *reinterpret_cast<u32 *>(0x0079c62c) = 0;
    *reinterpret_cast<u32 *>(0x0079c628) = 0;
    *reinterpret_cast<u32 *>(0x007925d0) = 0;
    if (changed) Flags_SetChanged(*reinterpret_cast<u32 *const *>(0x00790fec), 0);
}

void __cdecl Track_AppendPiece(u8 *piece, u8 *track, u32 open) {
    u32 flags = at<u32>(piece, 8);
    flags = open ? flags & ~0x4000u : flags | 0x4000;
    at<u32>(piece, 8) = flags;
    at<u32>(piece, 8) = flags | 0x8000;
    at<u8 *>(piece, 0x68) = at<u8 *>(track, 0x24);
    at<u8 *>(piece, 0x64) = at<u8 *>(at<u8 *>(track, 0x24), 0x64);
    if (u8 *prev = at<u8 *>(piece, 0x68)) at<u8 *>(prev, 0x64) = piece;
    if (u8 *next = at<u8 *>(piece, 0x64)) at<u8 *>(next, 0x68) = piece;
    u8 *last = at<u8 *>(track, 0x24);
    if (at<u8 *>(track, 0x1c) == last) {
        at<u8 *>(track, 0x1c) = piece;
    } else {
        const u32 old = at<u32>(last, 8);
        at<u32>(last, 8) = old & (old & 0x200 ? 0xfffeffff : 0xfffaffff);
        at<u32>(piece, 8) |= 0x50000;
    }
    at<u8 *>(track, 0x24) = piece;
    ++at<u32>(track, 8);
    TrackPieces_Renumber(piece);
}

void __cdecl Track_RemovePiece(u8 *piece, u8 *track) {
    if (!(at<u32>(piece, 8) & 0x8000)) return;
    if (at<u8 *>(track, 0x1c) == at<u8 *>(track, 0x24)) {
        at<u8 *>(track, 0x1c) = at<u8 *>(piece, 0x68);
    } else {
        at<u32>(piece, 8) &= 0xfffaffff;
        at<u32>(at<u8 *>(piece, 0x68), 8) |= 0x50000;
    }
    at<u8 *>(track, 0x24) = at<u8 *>(piece, 0x68);
    if (u8 *prev = at<u8 *>(piece, 0x68)) at<u8 *>(prev, 0x64) = at<u8 *>(piece, 0x64);
    if (u8 *next = at<u8 *>(piece, 0x64)) at<u8 *>(next, 0x68) = at<u8 *>(piece, 0x68);
    at<u8 *>(piece, 0x68) = nullptr;
    at<u8 *>(piece, 0x64) = nullptr;
    at<u32>(piece, 0) = 0;
    --at<u32>(track, 8);
    TrackPieces_Renumber(at<u8 *>(track, 0x24));
    at<u32>(piece, 8) &= ~0x8000u;
}

void __cdecl TrackPiece_NodeGaps(u32 i, const u8 *piece, float *before, float *after, u32 selector,
                                 float *beforeVec, float *afterVec) {
    const u32 list = selector ? 4 : 8;
    const auto nodes = [list](const u8 *p) { return at<const u8 *>(at<const u8 *>(p, 0x90), list); };
    const u8 *ns = nodes(piece);
    const u8 *prevPiece = at<const u8 *>(piece, 0x68), *nextPiece = at<const u8 *>(piece, 0x64);
    if (i)
        *before = Gap(ns + (i - 1) * 0x58, ns + i * 0x58, beforeVec);
    else if (prevPiece)
        *before = Gap(nodes(prevPiece) + (at<u32>(at<const u8 *>(prevPiece, 0x90), 0) - 1) * 0x58, ns, beforeVec);
    else
        *before = 1.0f;
    if (i != at<u32>(at<const u8 *>(piece, 0x90), 0) - 1)
        *after = Gap(ns + i * 0x58, ns + (i + 1) * 0x58, afterVec);
    else if (nextPiece)
        *after = Gap(ns + i * 0x58, nodes(nextPiece), afterVec);
    else
        *after = 1.0f;
}
}
