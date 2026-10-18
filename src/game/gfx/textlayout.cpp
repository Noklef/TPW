#include "textlayout.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "surface.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *&font(u8 *style) { return at<u8 *>(style, 4); }

// a vector's three dwords (begin, end, capacity) at offset: the block freed, null too, then the three cleared
void FreeVector(u8 *layout, u32 offset) {
    GameHeap_Free(at<void *>(layout, offset));
    at<u32>(layout, offset) = 0;
    at<u32>(layout, offset + 4) = 0;
    at<u32>(layout, offset + 8) = 0;
}

// one N-byte element copied a dword at a time, in order
template <u32 N> void CopyElement(u8 *to, const u8 *from) {
    for (u32 i = 0; i < N; i += 4) at<u32>(to, i) = at<u32>(const_cast<u8 *>(from), i);
}

// the signed element count between two pointers: 8 and 16 by SAR (rounded down), 12 by a signed division (rounded towards 0)
template <u32 N> s32 Elements(const u8 *a, const u8 *b) {
    const s32 bytes = (s32)((u32)(uintptr_t)a - (u32)(uintptr_t)b);
    return N == 12 ? bytes / 12 : bytes >> (N == 8 ? 3 : 4);
}

// MSVC6's vector<T>::insert(position, count, value) for an N-byte T (first +4, last +8, end +0xc): every copy into new room skips a null
// destination, the value is read again for every copy.
template <u32 N> void InsertN(u8 *vec, u8 *position, u32 count, const u8 *value) {
    u8 *const last = at<u8 *>(vec, 8);
    if ((u32)Elements<N>(at<u8 *>(vec, 0xc), last) >= count) {
        const u32 bytes = count * N;
        if ((u32)Elements<N>(last, position) >= count) {
            if (count == 0) return;
            u8 *to = last;
            for (u8 *from = last - bytes; from != last; from += N, to += N)
                if (to) CopyElement<N>(to, from);
            u8 *e = at<u8 *>(vec, 8) - bytes;
            to = at<u8 *>(vec, 8);
            if (position != e) {
                do {
                    e -= N;
                    to -= N;
                    CopyElement<N>(to, e);
                } while (e != position);
            }
            u8 *const stop = position + bytes;
            for (u8 *p = position; p != stop; p += N) CopyElement<N>(p, value);
        } else {
            if (position != last) {
                u8 *to = position + bytes;
                u8 *from = position;
                do {
                    if (to) CopyElement<N>(to, from);
                    from += N;
                    to += N;
                } while (from != last);
            }
            u8 *fill = at<u8 *>(vec, 8);
            for (u32 n = count - (u32)Elements<N>(fill, position); n != 0; --n, fill += N)
                if (fill) CopyElement<N>(fill, value);
            u8 *const end = at<u8 *>(vec, 8);
            for (u8 *p = position; p != end; p += N) CopyElement<N>(p, value);
        }
        at<u8 *>(vec, 8) += bytes;
        return;
    }
    u8 *const first = at<u8 *>(vec, 4);
    const u32 size = first ? (u32)Elements<N>(last, first) : 0;
    const u32 grow = count >= size ? count : size;
    const s32 capacity = (s32)(grow + size);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc((u32)(capacity < 0 ? 0 : capacity) * N));
    u8 *to = block;
    for (u8 *from = first; from != position; from += N, to += N)
        if (to) CopyElement<N>(to, from);
    u8 *fill = to;
    for (u32 n = count; n != 0; --n, fill += N)
        if (fill) CopyElement<N>(fill, value);
    u8 *const oldLast = at<u8 *>(vec, 8);
    u8 *dest = to + count * N;
    for (u8 *from = position; from != oldLast; from += N, dest += N)
        if (dest) CopyElement<N>(dest, from);
    GameHeap_Free(at<u8 *>(vec, 4));
    at<u8 *>(vec, 0xc) = block + (u32)capacity * N;  // the capacity before it was clamped at 0
    const u32 kept = at<u8 *>(vec, 4) ? (u32)Elements<N>(at<u8 *>(vec, 8), at<u8 *>(vec, 4)) : 0;
    at<u8 *>(vec, 4) = block;
    at<u8 *>(vec, 8) = block + (count + kept) * N;
}
}

extern "C" {

u8 *__fastcall TextStyle_Ctor(u8 *style) {
    at<u32>(style, 4) = 0;
    at<u32>(style, 0xc) = 3;
    at<u32>(style, 0x10) = 1;
    SurfaceInfo_Ctor(reinterpret_cast<SurfaceInfo *>(style + 0x14));
    at<u32>(style, 0x3c) = 0;
    at<u32>(style, 0x40) = 0;
    for (u32 i = 0x44; i <= 0x48; ++i) at<u8>(style, i) = 0;
    at<u32>(style, 0x4c) = 0;
    at<u32>(style, 0) = 0x0070a0d8;
    return style;
}

void __fastcall TextStyle_Dtor(u8 *style) {
    at<u32>(style, 0) = 0x0070a0d8;
    if (font(style)) {
        vcall<void>(font(style), 0);
        at<u32>(style, 4) = 0;
    }
}

u8 *__fastcall TextStyle_DeletingDtor(u8 *style, void *, u32 flags) {
    TextStyle_Dtor(style);
    if (flags & 1) GameHeap_Free(style);
    return style;
}

s32 *__fastcall TextStyle_SetFont(u8 *style, void *, s32 *status, u8 *newFont) {
    at<u32>(style, 0xc) = 3;
    if (font(style)) {
        vcall<void>(font(style), 0);
        at<u32>(style, 4) = 0;
    }
    font(style) = newFont;
    if (newFont) {
        vcall<void>(newFont, 4);
        at<u32>(style, 8) = vcall<u32>(font(style), 0x24);
    }
    *status = 0;
    return status;
}

s32 *__fastcall TextStyle_SetColour(u8 *style, void *, s32 *status, const u8 *colour) {
    if (!font(style) || !(at<u8>(style, 8) & 2)) {
        *status = -1;
        return status;
    }
    at<u32>(style, 0x44) = *reinterpret_cast<const u32 *>(colour);
    const u8 last = colour[4];
    at<u32>(style, 0xc) = 1;
    at<u8>(style, 0x48) = last;
    *status = 0;
    return status;
}

u8 *__fastcall TextStyle_Assign(u8 *style, void *, const u8 *source) {
    if (source == style) return style;
    at<u32>(style, 0x10) = at<u32>(const_cast<u8 *>(source), 0x10);
    at<u32>(style, 0x44) = at<u32>(const_cast<u8 *>(source), 0x44);
    at<u8>(style, 0x48) = at<u8>(const_cast<u8 *>(source), 0x48);
    u8 *f = at<u8 *>(const_cast<u8 *>(source), 4);
    font(style) = f;
    vcall<void>(f, 4);  // sic: no null check, and the old font keeps its reference
    at<u32>(style, 8) = at<u32>(const_cast<u8 *>(source), 8);
    at<u32>(style, 0xc) = at<u32>(const_cast<u8 *>(source), 0xc);
    for (u32 i = 0x14; i < 0x3c; i += 4) at<u32>(style, i) = at<u32>(const_cast<u8 *>(source), i);  // forward, as REP MOVSD
    at<u32>(style, 0x3c) = at<u32>(const_cast<u8 *>(source), 0x3c);
    at<u32>(style, 0x40) = at<u32>(const_cast<u8 *>(source), 0x40);
    at<u32>(style, 0x4c) = at<u32>(const_cast<u8 *>(source), 0x4c);
    return style;
}

u8 *__fastcall TextStyle_GetFont(u8 *style) {
    if (font(style)) vcall<void>(font(style), 4);
    return font(style);
}

s32 __fastcall TextStyle_FontKind(u8 *style) {
    if (!font(style)) return 5;
    return vcall<s32>(font(style), 0x28);
}

u8 *__fastcall TextStyleHolder_Ctor(u8 *holder) {
    TextStyle_Ctor(holder + 4);
    at<u32>(holder, 0) = 0x0070a13c;
    at<u32>(holder, 0x54) = 0;
    return holder;
}

void __fastcall TextStyleHolder_Dtor(u8 *holder) {
    at<u32>(holder, 0) = 0x0070a13c;
    TextStyle_Dtor(holder + 4);
}

u8 *__fastcall TextStyleHolder_DeletingDtor(u8 *holder, void *, u32 flags) {
    TextStyleHolder_Dtor(holder);
    if (flags & 1) GameHeap_Free(holder);
    return holder;
}

s32 *__fastcall TextStyleHolder_SetStyle(u8 *holder, void *, s32 *status, const u8 *style) {
    at<u32>(holder, 0x54) = 0;
    if ((s32)Source_Field0c(reinterpret_cast<const TrackedSource *>(style)) >= 3 ||
        TextStyle_FontKind(const_cast<u8 *>(style)) >= 5) {
        *status = -1;
        return status;
    }
    u8 *own = holder + 4;
    TextStyle_Assign(own, nullptr, style);
    const u32 f10 = at<u32>(holder, 0x14);
    const u32 kind = (u32)TextStyle_FontKind(own);
    const u32 index = (f10 * 5 + kind) * 3 + Source_Field0c(reinterpret_cast<const TrackedSource *>(own));  // sic: no bounds
    const u32 renderer = *reinterpret_cast<const u32 *>((uintptr_t)(0x00782d38u + index * 4));
    at<u32>(holder, 0x54) = renderer;
    *status = renderer ? 0 : -1;
    return status;
}

s32 *__fastcall TextStyleHolder_Render(u8 *holder, void *, s32 *status, u32 a, u32 b, u32 c) {
    u8 *renderer = at<u8 *>(holder, 0x54);
    if (!renderer) {
        *status = -1;
        return status;
    }
    vcall<void>(renderer, 4, status, a, b, c, holder + 4);
    return status;
}

u8 __stdcall TextLayout_CharClass(u32 character) {
    switch ((u16)character) {
    case 9:
    case 0x20:
        return 0x12;
    case 10:
        return 4;
    case 13:
        return 0xc;
    default:
        return 1;
    }
}

u8 *__fastcall Vector12_Erase(u8 *vector, void *, u8 *first, u8 *last) {
    u8 *const end = at<u8 *>(vector, 8);
    u8 *to = first;
    if (last != end) {
        u8 *from = last;
        do {
            at<u32>(to, 0) = at<u32>(from, 0);
            at<u32>(to, 4) = at<u32>(from, 4);
            at<u32>(to, 8) = at<u32>(from, 8);
            from += 12;
            to += 12;
        } while (from != end);
    }
    at<u8 *>(vector, 8) = to;
    return first;
}

void __fastcall Vector8_InsertN(u8 *vector, void *, u8 *position, u32 count, const u8 *value) {
    InsertN<8>(vector, position, count, value);
}

void __fastcall Vector16_InsertN(u8 *vector, void *, u8 *position, u32 count, const u8 *value) {
    InsertN<16>(vector, position, count, value);
}

void __fastcall Vector12_InsertN(u8 *vector, void *, u8 *position, u32 count, const u8 *value) {
    InsertN<12>(vector, position, count, value);
}

void __fastcall TextLines_Append(u8 *lines, void *, u8 *first, u8 *shown, u8 *next, u32 width) {
    const u32 line[3] = {(u32)(uintptr_t)first, (u32)(uintptr_t)next, width};
    Vector12_InsertN(lines, nullptr, at<u8 *>(lines, 8), 1, reinterpret_cast<const u8 *>(line));
    if (at<u32>(lines, 0x10) < width) at<u32>(lines, 0x10) = width;
    if (shown != next) {
        u8 *record = shown;
        do {
            at<u8>(record, 4) |= 0x40;
            record += 0x10;
        } while (record != next);
    }
}

u8 *__fastcall TextLayout_Ctor(u8 *layout) {
    TextStyleHolder_Ctor(layout + 4);
    const u8 b = (u8)((uintptr_t)layout >> 24);  // sic: the top byte of the pushed ECX, read as an allocator temporary
    at<u8>(layout, 0x5c) = b;
    at<u32>(layout, 0x60) = 0;
    at<u32>(layout, 0x64) = 0;
    at<u32>(layout, 0x68) = 0;
    at<u16>(layout, 0x6c) = 0;
    at<u8>(layout, 0x70) = b;
    for (u32 i = 0x74; i <= 0x80; i += 4) at<u32>(layout, i) = 0;
    at<u8>(layout, 0x84) = b;
    at<u32>(layout, 0x88) = 0;
    at<u32>(layout, 0x8c) = 0;
    at<u32>(layout, 0x90) = 0;
    at<u8>(layout, 0x94) = 0;
    at<u32>(layout, 0x98) = 0;
    at<u32>(layout, 0x9c) = 0;
    for (u32 i = 0xac; i <= 0xb8; i += 4) at<u32>(layout, i) = 0;
    at<u32>(layout, 0) = 0x0070a10c;
    return layout;
}

void __fastcall TextLayout_Dtor(u8 *layout) {
    at<u32>(layout, 0) = 0x0070a10c;
    FreeVector(layout, 0x88);
    FreeVector(layout, 0x74);
    FreeVector(layout, 0x60);
    TextStyleHolder_Dtor(layout + 4);
}

u8 *__fastcall TextLayout_DeletingDtor(u8 *layout, void *, u32 flags) {
    TextLayout_Dtor(layout);
    if (flags & 1) GameHeap_Free(layout);
    return layout;
}
}
