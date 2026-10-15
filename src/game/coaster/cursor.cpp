#include "cursor.h"

#include "train.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u32 nodes(u32 piece) { return at<u32>(at<const u8 *>((const void *)piece, 0x90), 0); }
u32 next(u32 piece) { return at<u32>((const void *)piece, 0x64); }
u32 prev(u32 piece) { return at<u32>((const void *)piece, 0x68); }
const u8 *node(u32 piece, u32 index) { return at<const u8 *>(at<const u8 *>((const void *)piece, 0x90), 4) + index * 0x58; }

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
}
