#include "spriterle.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kPitch = 0x00faa4d8, kClipWidth = 0x00faa4dc, kClipHeight = 0x00faa4e0, kFlags = 0x00faa4e4, kBits = 0x00faa4e8,
          kGhost = 0x00faa4ec, kClipLeft = 0x00faa4f8, kClipTop = 0x00faa4fc;

typedef long long s64;

s32 Wrap(s64 v) { return (s32)(u32)v; }

// ADD / SUB then JLE: the exact result at most 0 (SF != OF) or the register 0; r gets the register.
bool AddLe(s32 &r, s64 v) {
    const s64 exact = (s64)r + v;
    r = Wrap(exact);
    return exact < 0 || r == 0;
}

// Rows of the data passed over (at least one).
const u8 *SkipRows(const u8 *src, u32 rows) {
    do {
        u8 c;
        while ((c = *src++) != 0)
            if (!(c & 0x80)) src += c;
    } while (--rows != 0);
    return src;
}

u8 Blend(u32 mode, u8 source, u8 dest) {
    const u8 *ghost = global<const u8 *>(kGhost);
    return mode == 8 ? ghost[(u32)source << 8 | dest] : ghost[(u32)dest << 8 | source];
}

// count pixels from src at d, stepping dir (do-while: a count of 0 runs on, sic).
void Copy(u32 mode, const u8 *&src, u8 *&d, s32 dir, u32 count) {
    do {
        *d = mode ? Blend(mode, *src, *d) : *src;
        ++src;
        d += dir;
    } while (--count != 0);
}

// The unclipped rows: every run drawn.
void Rows(u32 mode, const u8 *src, u8 *dest, s32 dir, s32 rows, s32 step) {
    u8 *d = dest;
    do {
        const u8 c = *src++;
        if (c & 0x80) {
            d += dir * (s32)(u8)(0 - c);
        } else if (c == 0) {
            dest += step;
            d = dest;
            --rows;
        } else {
            Copy(mode, src, d, dir, c);
        }
    } while (rows != 0);
}

// The clipped rows: skip (pixels passed over first, in data order), then at most width drawn, the rest of the row passed over.
void ClippedRows(u32 mode, const u8 *src, u8 *dest, s32 dir, s32 rows, s32 step, s32 width, s32 skip) {
    do {
        u8 *d = dest;
        s32 left = width, pass = skip;
        u32 c = 0;
        enum { kSkip, kDraw, kData, kRest } at = pass != 0 ? kSkip : kDraw;
        for (;;) {
            if (at == kSkip) {
                c = *src;
                if (c == 0) break;
                ++src;
                if (!(c & 0x80)) {
                    if ((s32)c <= pass) {
                        src += c;
                        pass -= c;
                        continue;
                    }
                    src += pass;
                    c -= pass;
                    at = left == 0 ? kRest : kData;
                } else {
                    c = (u8)(0 - c);
                    if ((s32)c < pass) {
                        pass -= c;
                        continue;
                    }
                    c -= pass;
                    d += dir * (s32)c;
                    at = AddLe(left, -(s64)c) ? kRest : kDraw;
                }
            } else if (at == kDraw) {
                c = *src;
                if (c == 0) break;
                ++src;
                if (c & 0x80) {
                    c = (u8)(0 - c);
                    d += dir * (s32)c;
                    if (AddLe(left, -(s64)c)) at = kRest;
                } else {
                    at = kData;
                }
            } else if (at == kData) {
                if ((s32)c < left) {
                    left -= c;
                    Copy(mode, src, d, dir, c);
                    at = kDraw;
                } else {
                    const u32 rest = c - (u32)left;
                    Copy(mode, src, d, dir, (u32)left);
                    src += rest;
                    at = kRest;
                }
            } else {
                c = *src;
                if (c == 0) break;
                src += (c & 0x80) ? 1 : c + 1;
            }
        }
        ++src;
        dest += step;
    } while (--rows != 0);
}

}  // namespace

extern "C" {

void __cdecl Sprite_DrawRle(s32 x, s32 y, const RleSprite *sprite) {
    if (sprite->width == 0 || sprite->height == 0) return;
    const u8 *src = sprite->data;
    const s32 height = sprite->height;
    s32 rows = height, step, row;
    if (!(global<u32>(kFlags) & 2)) {
        step = global<s32>(kPitch);
        const s32 bottom = Wrap((s64)height + y), clipHeight = global<s32>(kClipHeight);
        if (bottom > clipHeight) {
            rows = Wrap((s64)rows - bottom);
            if (AddLe(rows, clipHeight)) return;
        }
        row = y;
        if (y < 0) {
            if (AddLe(rows, y)) return;
            src = SkipRows(src, Wrap(-(s64)y));
            row = 0;
        }
    } else {
        step = Wrap(-(s64)global<s32>(kPitch));
        row = Wrap((s64)y + height - 1);
        if (y < 0 && AddLe(rows, y)) return;
        const s32 last = Wrap((s64)global<s32>(kClipHeight) - 1);
        if (row > last) {
            rows = Wrap((s64)rows - row);
            if (AddLe(rows, last)) return;
            src = SkipRows(src, Wrap((s64)row - last));
            row = last;
        }
    }
    const bool flipX = (global<u32>(kFlags) & 1) != 0;
    const s32 width = sprite->width, clipWidth = global<s32>(kClipWidth);
    if (x >= clipWidth) return;
    s32 visible = width, skip = 0, column;
    bool clipped;
    if (!flipX) {
        s32 right = x;
        if (AddLe(right, width)) return;
        clipped = right > clipWidth || x < 0;
        column = x;
        if (clipped) {
            if (right > clipWidth) visible = Wrap((s64)clipWidth - x);
            if (x < 0) {
                skip = Wrap(-(s64)x);
                visible = Wrap((s64)visible - skip);
                column = 0;
            }
        }
    } else {
        const s64 exact = (s64)Wrap((s64)x - 1) + width;
        if (exact < 0) return;
        column = Wrap(exact);
        clipped = column >= clipWidth || column < width;
        if (clipped) {
            if (column >= clipWidth) {
                skip = Wrap((s64)column - clipWidth + 1);
                column = Wrap((s64)clipWidth - 1);
            }
            if (column < width) visible = Wrap((s64)column + 1);
        }
    }
    u8 *const dest = reinterpret_cast<u8 *>((uintptr_t)((u32)column + global<u32>(kClipLeft) +
                                                        ((u32)row + global<u32>(kClipTop)) * global<u32>(kPitch) + global<u32>(kBits)));
    const u32 flags = global<u32>(kFlags);
    const u32 mode = (flags & 8) ? 8 : (flags & 0x10) ? 0x10 : 0;
    const s32 dir = flipX ? -1 : 1;
    if (clipped)
        ClippedRows(mode, src, dest, dir, rows, step, visible, skip);
    else
        Rows(mode, src, dest, dir, rows, step);
}
}
