#include "textlayout.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
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
