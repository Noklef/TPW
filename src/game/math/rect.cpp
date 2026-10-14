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

}  // extern "C"
