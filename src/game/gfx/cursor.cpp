// The software mouse cursor: see cursor.h.
#include "cursor.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const CursorEnv kGameEnv = {
    at<Cursor *>(0x00fa7a80),
    at<u8 *>(0x00fa7a78),
    at<u8 *>(0x00775874),
    0x00703640,
    at<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
    at<void(__cdecl *)()>(0x005fadc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
};
const CursorEnv *g_env = &kGameEnv;

// Surface_BltFast, then the busy byte cleared; the result.
s32 blt(Surface *dst, s32 x, s32 y, Surface *src, const Rect *rect, u32 flags) {
    s32 r;
    Surface_BltFast(&r, dst, static_cast<u32>(x), static_cast<u32>(y), src, rect, flags);
    *g_env->busy = 0;
    return r;
}

void setKey(SurfaceBuffer *b, const u8 *key) {
    std::memcpy(&b->surface.keyRgb, key, 4);
    b->surface.keyIndex = key[4];
}

}  // namespace

void Cursor_SetEnv(const CursorEnv *e) { g_env = e ? e : &kGameEnv; }
const CursorEnv *Cursor_GetEnv() { return g_env; }

extern "C" {

SurfaceInfo *__cdecl Surface_GetInfo(SurfaceInfo *out, Surface *s) {
    *out = *Surface_Refresh(s);
    return out;
}

Cursor *__cdecl Cursor_Get() {
    const CursorEnv *e = g_env;
    if (!(*e->guard & 1)) {
        *e->guard |= 1;
        Cursor_Ctor(e->instance);
        e->atExit(e->onExit);
    }
    return e->instance;
}

void __cdecl Cursor_AtExit() { Cursor_Dtor(g_env->instance); }

Cursor *__fastcall Cursor_Ctor(Cursor *c) {
    c->created = 0;
    c->key[0] = c->key[1] = c->key[2] = 0;
    c->key[3] = 0xff;
    c->key[4] = 0;
    c->maxWidth = c->maxHeight = 0;
    c->width = c->height = 0;
    c->dirty = 0;
    c->clip = Rect{};
    c->hotX = c->hotY = 0;
    c->x = c->y = 0x7fffffff;
    for (SurfaceBuffer &b : c->buffers) SurfaceBuffer_Ctor(&b);  // the eh vector constructor iterator
    for (int i = 0; i < 3; ++i) c->place[i] = c->source[i] = c->screen[i] = c->overlapRect[i] = Rect{};
    c->vtable = g_env->vtable;
    return c;
}

Cursor *__fastcall Cursor_DeletingDtor(Cursor *c, void *, u32 flags) {
    Cursor_Dtor(c);
    if (flags & 1) g_env->free(c);
    return c;
}

void __fastcall Cursor_Dtor(Cursor *c) {
    c->vtable = g_env->vtable;
    for (int i = 2; i >= 0; --i) SurfaceBuffer_Dtor(&c->buffers[i]);  // the eh vector destructor iterator
}

void __fastcall Cursor_Draw(Cursor *c, void *, const s32 *pos, Surface *target) {
    c->target[0] = target;
    if (!c->created) return;
    if (c->shown && pos[0] == c->x && pos[1] == c->y && !c->dirty) return;
    c->dirty = 0;
    Cursor_Move(c, nullptr, pos);
    *g_env->busy = 1;
    if (!Cursor_SaveUnder(c)) return;
    Cursor_EraseShown(c);
    Cursor_Paint(c);
    Cursor_Flip(c);
}

void __fastcall Cursor_Discard(Cursor *c) {
    c->shown = 0;
    Cursor_ReleaseBuffers(c);
}

s32 *__fastcall Cursor_CreateBuffers(Cursor *c, void *, s32 *out) {
    Cursor_Discard(c);
    const u32 size[2] = {c->maxWidth, c->maxHeight};
    for (SurfaceBuffer &b : c->buffers) {
        s32 made;
        if (*SurfaceBuffer_Create(&b, nullptr, &made, size, 0x800) < 0) {  // DDSCAPS_SYSTEMMEMORY
            *out = made;
            return out;
        }
        setKey(&b, c->key);
    }
    c->shown = 0;
    c->next = 1;
    c->created = 1;
    c->parked = 0;
    *out = 0;
    return out;
}

void __fastcall Cursor_Flip(Cursor *c) {
    c->shown = c->next;
    switch (c->next) {
    case 0: c->next = (c->parked != 2) + 1; break;  // the one not parked
    case 1: c->next = 2; break;
    case 2: c->next = 1; break;
    }
}

void __fastcall Cursor_ReleaseBuffers(Cursor *c) {
    for (SurfaceBuffer &b : c->buffers) SurfaceBuffer_Release(&b);
    c->created = 0;
}

s32 *__fastcall Cursor_SetImage(Cursor *c, void *, s32 *out, Surface *image, const Rect *rect, const s32 *hot) {
    if (!image) return Cursor_ClearImage(c, nullptr, out);
    if (static_cast<u32>(rect->right - rect->left) > c->maxWidth ||
        static_cast<u32>(rect->bottom - rect->top) > c->maxHeight) {
        if (c->created) {
            if (c->shown) {
                const s32 pos[2] = {c->screen[0].left, c->screen[0].top};
                Cursor_Draw(c, nullptr, pos, nullptr);
            }
            Cursor_ReleaseBuffers(c);
        }
        const u32 w = static_cast<u32>(rect->right - rect->left);
        if (c->maxWidth <= w) c->maxWidth = w;
        const u32 h = static_cast<u32>(rect->bottom - rect->top);
        if (c->maxHeight <= h) c->maxHeight = h;
    }
    if (!c->created) {
        s32 made;
        if (*Cursor_CreateBuffers(c, nullptr, &made) < 0) {
            *out = made;
            return out;
        }
    }
    for (Rect &p : c->place) {
        Rect_MoveTo(&p, nullptr, 0, 0);
        p.right = p.left + static_cast<s32>(c->width);  // the old size
        p.bottom = p.top + static_cast<s32>(c->height);
    }
    c->height = static_cast<u32>(rect->bottom - rect->top);
    c->width = static_cast<u32>(rect->right - rect->left);
    const s32 r = blt(&c->buffers[0].surface, c->place[0].left, c->place[0].top, image, rect, 0);
    if (r < 0) {
        *out = r;
        return out;
    }
    c->hotX = -hot[0];
    c->hotY = -hot[1];
    c->dirty = 1;
    *out = 0;
    return out;
}

u8 __fastcall Cursor_SaveUnder(Cursor *c) {
    const s32 n = c->next;
    if (!n) return 1;
    c->screen[n] = c->screen[0];
    const Rect &p = c->place[n];
    c->source[n] = Rect{p.left + c->source[0].left, p.top + c->source[0].top, c->source[0].right + p.left,
                        c->source[0].bottom + p.top};
    c->target[n] = c->target[0];
    if (blt(&c->buffers[n].surface, c->source[n].left, c->source[n].top, c->target[n], &c->screen[n], 0) < 0) {
        c->next = 0;
        return 0;
    }
    const s32 s = c->shown;
    if (s && c->overlap) {
        // the part the old cursor covers comes from the old buffer
        if (blt(&c->buffers[n].surface, c->overlapRect[n].left, c->overlapRect[n].top, &c->buffers[s].surface,
                &c->overlapRect[s], 0) < 0) {
            c->next = 0;
            return 0;
        }
    }
    return 1;
}

u8 __fastcall Cursor_EraseShown(Cursor *c) {
    const s32 s = c->shown;
    if (!s) return 1;
    if (c->overlap) {
        // the new cursor's overlapping part goes into the old background first
        if (blt(&c->buffers[s].surface, c->overlapRect[s].left, c->overlapRect[s].top, &c->buffers[0].surface,
                &c->overlapRect[0], 1) < 0)
            return 0;
    }
    if (blt(c->target[s], c->screen[s].left, c->screen[s].top, &c->buffers[s].surface, &c->source[s], 0) < 0)
        return 0;
    return 1;
}

u8 __fastcall Cursor_Paint(Cursor *c) {
    if (!c->target[0]) return 1;
    return blt(c->target[0], c->screen[0].left, c->screen[0].top, &c->buffers[0].surface, &c->source[0], 1) >= 0;
}

void __fastcall Cursor_Move(Cursor *c, void *, const s32 *pos) {
    c->x = pos[0];
    c->y = pos[1];
    const s32 ox = pos[0] + c->hotX, oy = pos[1] + c->hotY;
    const Rect image = {ox, oy, ox + static_cast<s32>(c->width), oy + static_cast<s32>(c->height)};
    Rect clipped;
    Rect &s = c->screen[0];
    s = *Rect_Clip(&image, nullptr, &clipped, &c->clip);
    c->source[0] = Rect{s.left - ox, s.top - oy, s.right - ox, s.bottom - oy};
    if (!c->target[0]) c->next = 0;
    if (s.left > s.right || s.top > s.bottom || s.left == s.right || s.top == s.bottom) {
        c->next = 0;
        c->target[0] = nullptr;
        c->overlap = 0;
        return;
    }
    const s32 sh = c->shown;
    c->overlap = c->next && sh && Rect_Overlaps(&s, nullptr, &c->screen[sh]) ? 1 : 0;
    if (!c->overlap) return;
    Rect o;
    Rect_Clip(&s, nullptr, &o, &c->screen[sh]);
    c->overlapRect[0] = Rect{o.left - ox, o.top - oy, o.right - ox, o.bottom - oy};
    const Rect &n = c->overlapRect[0];
    const Rect &p = c->place[c->next];
    c->overlapRect[c->next] = Rect{n.left + p.left, n.top + p.top, n.right + p.left, n.bottom + p.top};
    // the overlap in the shown buffer: from the target to its source rectangle
    const s32 dx = c->source[sh].left - c->screen[sh].left, dy = c->source[sh].top - c->screen[sh].top;
    Rect r;
    c->overlapRect[sh] = *Rect_Set(&r, nullptr, o.left + dx, o.top + dy, o.right + dx, o.bottom + dy);
}

s32 *__fastcall Cursor_ClearImage(Cursor *c, void *, s32 *out) {
    c->width = 0;
    c->height = 0;
    *out = 0;
    return out;
}

void __fastcall Cursor_SetKey(Cursor *c, void *, const u8 *key) {
    std::memcpy(c->key, key, 5);
    if (!c->created) return;
    for (SurfaceBuffer &b : c->buffers) setKey(&b, key);
}

void __fastcall Cursor_Update(Cursor *c, void *, const s32 *pos, u32, Surface *target, u8 park, u8 also) {
    if (park && also) c->parked = c->shown;
    c->shown = 0;
    Cursor_Draw(c, nullptr, pos, target);
}

void __fastcall Cursor_Restore(Cursor *c, void *, Surface *target, Surface *dst, u8 erase, u8 useParked) {
    if (erase) {
        const s32 i = useParked ? c->parked : c->shown;
        if (i) blt(dst, c->screen[i].left, c->screen[i].top, &c->buffers[i].surface, &c->source[i], 0);
    }
    if (c->shown) c->target[c->shown] = target;
    c->parked = 0;
}

}  // extern "C"
