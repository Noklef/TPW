#include "guirect.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/initializers.h"

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

u8 *__fastcall GuiRect_Assign(u8 *rect, void *, const u8 *source) {
    Fn_65aa80(rect, nullptr, (u32)(uintptr_t)source);
    for (u32 off = 4; off < 0xc; off += 2)
        at<u16>(rect, off) = at<u16>(const_cast<u8 *>(source), off);
    return rect;
}
}
