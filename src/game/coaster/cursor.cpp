#include "cursor.h"

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
}
