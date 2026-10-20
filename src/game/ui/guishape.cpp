#include "guishape.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/initializers.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 mul(s32 a, s32 b) { return (s32)((u32)a * (u32)b); }  // (IMUL: wraps)

u8 *base(u8 *shape, u32 vtable) {
    Fn_65a9d0(shape);
    at<u32>(shape, 0) = vtable;
    return shape;
}

u16 *points(u8 *polygon) { return at<u16 *>(polygon, 8); }

u8 *deleting(u8 *shape, u32 flags) {
    if (flags & 1) GameHeap_Free(shape);
    return shape;
}

}  // namespace

extern "C" {

u8 *__fastcall GuiShape_DeletingDtor(u8 *shape, void *, u32 flags) {
    Fn_66bff7(shape);
    return deleting(shape, flags);
}

u8 *__fastcall GuiCircle_Ctor(u8 *circle) { return GuiCircle_CtorWith(circle, nullptr, 0, 0, 0); }

u8 *__fastcall GuiCircle_CtorWith(u8 *circle, void *, u32 x, u32 y, u32 radius) {
    base(circle, 0x705b50);
    at<u16>(circle, 4) = (u16)x;
    at<u16>(circle, 6) = (u16)y;
    at<u16>(circle, 8) = (u16)radius;
    return circle;
}

u32 __fastcall GuiCircle_Contains(u8 *circle, void *, u32 x, u32 y) {
    const s32 dx = (s16)x - at<s16>(circle, 4), dy = (s16)y - at<s16>(circle, 6);
    const s32 radius = at<s16>(circle, 8);
    return (s32)((u32)mul(dx, dx) + (u32)mul(dy, dy)) < radius * radius ? 1 : 0;
}

void __fastcall GuiCircle_Offset(u8 *circle, void *, u32 dx, u32 dy) {
    at<u16>(circle, 4) += (u16)dx;
    at<u16>(circle, 6) += (u16)dy;
}

void __fastcall GuiEllipse_Offset(u8 *ellipse, void *, u32 dx, u32 dy) { GuiCircle_Offset(ellipse, nullptr, dx, dy); }

u8 *__fastcall GuiEllipse_Ctor(u8 *ellipse) { return GuiEllipse_CtorWith(ellipse, nullptr, 0, 0, 0, 0); }

u8 *__fastcall GuiEllipse_CtorWith(u8 *ellipse, void *, u32 x, u32 y, u32 rx, u32 ry) {
    base(ellipse, 0x705b68);
    at<u16>(ellipse, 4) = (u16)x;
    at<u16>(ellipse, 6) = (u16)y;
    at<u16>(ellipse, 8) = (u16)rx;
    at<u16>(ellipse, 0xa) = (u16)ry;
    return ellipse;
}

u32 __fastcall GuiEllipse_Contains(u8 *ellipse, void *, u32 x, u32 y) {
    const s32 ry = at<s16>(ellipse, 0xa);
    const s32 dx = mul((s16)x - at<s16>(ellipse, 4), ry) / at<s16>(ellipse, 8);
    const s32 dy = (s16)y - at<s16>(ellipse, 6);
    return (s32)((u32)mul(dx, dx) + (u32)mul(dy, dy)) < ry * ry ? 1 : 0;
}

u8 *__fastcall GuiCircle_DeletingDtor(u8 *circle, void *, u32 flags) {
    GuiCircle_Dtor(circle);
    return deleting(circle, flags);
}

u32 __fastcall GuiCircle_Dtor(u8 *circle) { return Fn_66bff7(circle); }

u8 *__fastcall GuiPolygon_Ctor(u8 *polygon) {
    base(polygon, 0x705b80);
    at<u16 *>(polygon, 8) = nullptr;
    at<u16>(polygon, 4) = 0;
    return polygon;
}

u8 *__fastcall GuiPolygon_CtorTriangle(u8 *polygon, void *, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2) {
    base(polygon, 0x705b80);
    at<u16>(polygon, 4) = 3;
    at<u16 *>(polygon, 8) = static_cast<u16 *>(GameHeap_Alloc(0xc));
    const u32 words[] = {x0, y0, x1, y1, x2, y2};
    for (u32 i = 0; i < 6; ++i) points(polygon)[i] = (u16)words[i];
    return polygon;
}

u8 *__fastcall GuiPolygon_CtorQuad(u8 *polygon, void *, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3) {
    base(polygon, 0x705b80);
    at<u16>(polygon, 4) = 4;
    at<u16 *>(polygon, 8) = static_cast<u16 *>(GameHeap_Alloc(0x10));
    const u32 words[] = {x0, y0, x1, y1, x2, y2, x3, y3};
    for (u32 i = 0; i < 8; ++i) points(polygon)[i] = (u16)words[i];
    return polygon;
}

u8 *__fastcall GuiPolygon_CtorPoints(u8 *polygon, void *, u32 count, const u16 *source) {
    base(polygon, 0x705b80);
    at<u16>(polygon, 4) = (u16)count;
    at<u16 *>(polygon, 8) = static_cast<u16 *>(GameHeap_Alloc((u32)at<s16>(polygon, 4) << 2));
    for (s32 i = 0; i < (s16)count; ++i) {
        points(polygon)[2 * i] = source[2 * i];
        points(polygon)[2 * i + 1] = source[2 * i + 1];
    }
    return polygon;
}

u32 __fastcall GuiPolygon_Dtor(u8 *polygon) {
    at<u32>(polygon, 0) = 0x705b80;
    if (points(polygon)) GameHeap_Free(points(polygon));
    return Fn_66bff7(polygon);
}

u32 __fastcall GuiPolygon_Contains(u8 *polygon, void *, u32 x, u32 y) {
    u32 inside = 0;
    if (at<s16>(polygon, 4) == 0 || !points(polygon)) return inside;
    const s32 px = (s16)x, py = (s16)y;
    const s16 *prev = reinterpret_cast<const s16 *>(points(polygon)) + 2 * (at<s16>(polygon, 4) - 1);
    const s16 *cur = reinterpret_cast<const s16 *>(points(polygon));
    u32 prev_above = prev[1] >= py ? 1 : 0;
    for (u32 n = (u32)(s32)at<s16>(polygon, 4); n != 0; --n, prev = cur, cur += 2) {
        const u32 above = cur[1] >= py ? 1 : 0;
        if (prev_above != above) {
            const u32 side = mul(cur[1] - py, prev[0] - cur[0]) >= mul(cur[0] - px, prev[1] - cur[1]) ? 1 : 0;
            if (side == above) inside = inside == 0 ? 1 : 0;
        }
        prev_above = above;
    }
    return inside;
}

void __fastcall GuiPolygon_Offset(u8 *polygon, void *, u32 dx, u32 dy) {
    if (at<s16>(polygon, 4) == 0 || !points(polygon)) return;
    for (s32 i = 0; i < at<s16>(polygon, 4); ++i) {
        points(polygon)[2 * i] += (u16)dx;
        points(polygon)[2 * i + 1] += (u16)dy;
    }
}

u8 *__fastcall GuiPolygon_DeletingDtor(u8 *polygon, void *, u32 flags) {
    GuiPolygon_Dtor(polygon);
    return deleting(polygon, flags);
}
}
