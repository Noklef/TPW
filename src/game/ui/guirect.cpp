#include "guirect.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void set(u8 *rect, u16 left, u16 top, u16 right, u16 bottom) {
    at<u16>(rect, 4) = left;
    at<u16>(rect, 6) = top;
    at<u16>(rect, 8) = right;
    at<u16>(rect, 0xa) = bottom;
}

s32 half_way(u8 *rect, u32 lo) { return at<s16>(rect, lo) + ((at<s16>(rect, lo + 4) - at<s16>(rect, lo)) >> 1); }
}

extern "C" {

u8 *__fastcall GuiRect_Ctor(u8 *rect) {
    Fn_65a9d0(rect);
    at<u32>(rect, 0) = 0x705920;
    set(rect, 0, 0, 0, 0);
    return rect;
}

u8 *__fastcall GuiRect_CtorWith(u8 *rect, void *, u32 left, u32 top, u32 right, u32 bottom) {
    Fn_65a9d0(rect);
    at<u32>(rect, 0) = 0x705920;
    set(rect, (u16)left, (u16)top, (u16)right, (u16)bottom);
    return rect;
}

u32 __fastcall GuiRect_Dtor(u8 *rect) {
    at<u32>(rect, 0) = 0x705920;
    return Fn_66bff7(rect);
}

u32 __fastcall GuiRect_Contains(u8 *rect, void *, u32 x, u32 y) {
    const s32 px = (s16)x, py = (s16)y;
    return px >= at<s16>(rect, 4) && py >= at<s16>(rect, 6) && px < at<s16>(rect, 8) && py < at<s16>(rect, 0xa) ? 1 : 0;
}

void __fastcall GuiRect_Centre(u8 *rect, void *, u16 *x, u16 *y) {
    if (x) *x = (u16)half_way(rect, 4);
    if (y) *y = (u16)half_way(rect, 6);
}

void __fastcall GuiRect_Offset(u8 *rect, void *, u32 dx, u32 dy) {
    at<u16>(rect, 4) += (u16)dx;
    at<u16>(rect, 8) += (u16)dx;
    at<u16>(rect, 6) += (u16)dy;
    at<u16>(rect, 0xa) += (u16)dy;
}

void __fastcall GuiRect_Adjust(u8 *rect, void *, u32 dl, u32 dt, u32 dr, u32 db) {
    at<u16>(rect, 4) += (u16)dl;
    at<u16>(rect, 8) += (u16)dr;
    at<u16>(rect, 6) += (u16)dt;
    at<u16>(rect, 0xa) += (u16)db;
}

u8 *__fastcall GuiRect_CopyCtor(u8 *rect, void *, const u8 *source) {
    Fn_6617d0(rect, nullptr, (u32)(uintptr_t)source);
    for (u32 off = 4; off < 0xc; off += 2)
        at<u16>(rect, off) = at<u16>(const_cast<u8 *>(source), off);
    at<u32>(rect, 0) = 0x705920;
    return rect;
}

u8 *__fastcall GuiRectNode_Ctor(u8 *node) {
    GuiRect_Ctor(node + 4);
    Fn_659dac(node + 4, nullptr, 0, 0, 0, 0);
    at<u32>(node, 0) = 0;
    return node;
}

u8 *__fastcall GuiRectNode_CtorFrom(u8 *node, void *, const u8 *source) {
    GuiRect_Ctor(node + 4);
    if (source) GuiRect_Assign(node + 4, nullptr, source);
    at<u32>(node, 0) = 0;
    return node;
}

u8 *__fastcall GuiRect_Assign(u8 *rect, void *, const u8 *source) {
    Fn_65aa80(rect, nullptr, (u32)(uintptr_t)source);
    for (u32 off = 4; off < 0xc; off += 2)
        at<u16>(rect, off) = at<u16>(const_cast<u8 *>(source), off);
    return rect;
}

u32 __fastcall GuiRect_Split(u8 *rect, void *, u8 *shape, u8 *first, u8 *second) {
    if (!shape) return 0;
    const u32 how = GuiRect_Overlap(rect, nullptr, shape) & 0xffff;
    const u32 l = at<u16>(rect, 4), t = at<u16>(rect, 6), r = at<u16>(rect, 8), b = at<u16>(rect, 0xa);
    const u32 sl = at<u16>(shape, 4), st = at<u16>(shape, 6), sr = at<u16>(shape, 8), sb = at<u16>(shape, 0xa);
    if (how & 0x10) {
        if (first) Fn_659dac(first, nullptr, l, t, r, st);
        if (second) Fn_659dac(second, nullptr, l, st, r, b);
        return 1;
    }
    if (how & 0x20) {
        if (first) Fn_659dac(first, nullptr, l, sb, r, b);
        if (second) Fn_659dac(second, nullptr, l, t, r, sb);
        return 1;
    }
    if (how & 0x40) {
        if (first) Fn_659dac(first, nullptr, l, t, sl, b);
        if (second) Fn_659dac(second, nullptr, sl, t, r, b);
        return 1;
    }
    if (how & 0x80) {
        if (first) Fn_659dac(first, nullptr, sr, t, r, b);
        if (second) Fn_659dac(second, nullptr, l, t, sr, b);
        return 1;
    }
    if (how & 0x100) return 0;
    if (first) Fn_659dac(first, nullptr, l, t, r, b);
    return 0;
}

u32 __fastcall GuiRect_Overlap(u8 *rect, void *, u8 *shape) {
    const s32 l = at<s16>(rect, 4), t = at<s16>(rect, 6), r = at<s16>(rect, 8), b = at<s16>(rect, 0xa);
    const s32 sl = at<s16>(shape, 4), st = at<s16>(shape, 6), sr = at<s16>(shape, 8), sb = at<s16>(shape, 0xa);
    if (sl == l && st == t && sr == r && sb == t) return 0x200;   // (the bottom against the top, sic)
    if ((u16)vcall<u32>(shape, 0, (u32)(l + 1), (u32)(t + 1)) && (u16)vcall<u32>(shape, 0, (u32)(r - 1), (u32)(b - 1))) return 0x100;
    u16 bits = 0;
    if ((u16)vcall<u32>(rect, 0, (u32)(u16)sl, (u32)(u16)st)) bits |= 0x51;
    if ((u16)vcall<u32>(rect, 0, (u32)(u16)sr, (u32)(u16)st)) bits |= 0x92;
    if ((u16)vcall<u32>(rect, 0, (u32)(u16)sr, (u32)(u16)sb)) bits |= 0xa8;
    if ((u16)vcall<u32>(rect, 0, (u32)(u16)sl, (u32)(u16)sb)) bits |= 0x64;
    if (bits & 0xf) return bits;
    if (sl <= l && sr >= r) {
        if (st > t && st < b) bits |= 0x10;
        if (sb > t && sb < b) bits |= 0x20;
    }
    if (st <= t && sb >= b) {
        if (sl > l && sl < r) bits |= 0x40;
        if (sr > l && sr < r) bits |= 0x80;
    }
    return bits;
}

void __fastcall GuiRect_GetSize(u8 *rect, void *, u16 *width, u16 *height) {
    if (width) *width = (u16)(at<s16>(rect, 8) - at<s16>(rect, 4));
    if (height) *height = (u16)(at<s16>(rect, 0xa) - at<s16>(rect, 6));
}

u32 __fastcall GuiRect_Equals(const u8 *rect, void *, const u8 *other) {
    for (u32 k = 4; k < 0xc; k += 2)
        if (*reinterpret_cast<const s16 *>(rect + k) != *reinterpret_cast<const s16 *>(other + k)) return 0;
    return 1;
}

u8 *__fastcall GuiRect_DeletingDtor(u8 *rect, void *, u32 flags) {
    if (flags & 2) {
        CRT_VectorDtorIterator(rect, 0xc, at<s32>(rect - 4, 0), reinterpret_cast<void(__fastcall *)(void *)>(&GuiRect_Dtor));
        GameHeap_Free(rect - 4);
    } else {
        GuiRect_Dtor(rect);
        if (flags & 1) GameHeap_Free(rect);
    }
    return rect;
}
}
