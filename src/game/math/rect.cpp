// The rectangle helpers: see rect.h.
#include "rect.h"

extern "C" {

void __cdecl Rect_Copy(Rect *dst, const Rect *src) { *dst = *src; }

s32 __cdecl Rect_Width(const Rect *r) { return r->right - r->left; }

s32 __cdecl Rect_Height(const Rect *r) { return r->bottom - r->top; }

u32 __cdecl Rect_Intersect(Rect *a, const Rect *b) {
    a->left = a->left > b->left ? a->left : b->left;
    a->top = a->top > b->top ? a->top : b->top;
    a->right = a->right < b->right ? a->right : b->right;
    a->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    return a->left < a->right && a->top < a->bottom;
}

void __cdecl Rect_Collapse(Rect *r) {
    r->right = r->left;
    r->bottom = r->top;
}

void __cdecl Rect_Normalize(Rect *r) {
    if (r->left > r->right) {
        const s32 t = r->left;
        r->left = r->right;
        r->right = t;
    }
    if (r->top > r->bottom) {
        const s32 t = r->top;
        r->top = r->bottom;
        r->bottom = t;
    }
}

Rect *__fastcall Rect_Clip(const Rect *a, void *, Rect *out, const Rect *b) {
    const s32 left = a->left > b->left ? a->left : b->left;
    const s32 top = a->top > b->top ? a->top : b->top;
    s32 right = a->right < b->right ? a->right : b->right;
    s32 bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    if (right < left) right = left;  // an empty result collapses onto its corner
    if (bottom < top) bottom = top;
    out->left = left;
    out->top = top;
    out->right = right;
    out->bottom = bottom;
    return out;
}

u32 __fastcall Rect_Overlaps(const Rect *a, void *, const Rect *b) {
    return a->left <= b->right && a->right >= b->left && a->top <= b->bottom && a->bottom >= b->top;
}

Rect *__fastcall Rect_Union(const Rect *a, void *, Rect *out, const Rect *b) {
    const s32 left = a->left < b->left ? a->left : b->left;
    const s32 top = a->top < b->top ? a->top : b->top;
    const s32 right = a->right > b->right ? a->right : b->right;
    const s32 bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    out->left = left;
    out->top = top;
    out->right = right;
    out->bottom = bottom;
    return out;
}

void __fastcall Rect_Order(Rect *r) { Rect_Normalize(r); }

void __fastcall Rect_MoveTo(Rect *r, void *, s32 x, s32 y) {
    r->right += x - r->left;
    r->bottom += y - r->top;
    r->left = x;
    r->top = y;
}

Rect *__fastcall Rect_Set(Rect *r, void *, s32 left, s32 top, s32 right, s32 bottom) {
    r->left = left;
    r->top = top;
    r->right = right;
    r->bottom = bottom;
    return r;
}

}  // extern "C"
