// Text through the text driver: see text.h.
#include "text.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const TextEnv kGameEnv = {
    at<const TextDriver **>(0x00fa76a0),
    at<void **>(0x00fa76a4),
    at<DrawDriver **>(0x00fa76d0),
};
const TextEnv *g_env = &kGameEnv;

const u32 kSpriteSlot = 14, kSpriteShapeSlot = 15;

const RleSprite *glyph(const RleSprite *font, u32 ch) {
    return at<const RleSprite *>(static_cast<u32>(reinterpret_cast<std::uintptr_t>(font)) + ch * 8 - 0x100);
}

}  // namespace

void Text_SetEnv(const TextEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __cdecl Text_Draw(s32 x, s32 y, const char *text, DrawColour colour) {
    s32 cx = x;
    for (; *text; ++text) {
        if (*text == '\n') {
            y += static_cast<s32>(Text_LineHeight(' '));
            cx = x;
        } else {
            DrawColour c;
            c.rgb = colour.rgb;
            c.index = colour.index;  // the padding is left as it lies
            cx += static_cast<s32>((*g_env->driver)->draw(*g_env->context, cx, y, static_cast<u8>(*text), c));
        }
    }
}

s32 *__cdecl Text_SetDriver(s32 *out, const TextDriver *driver, u32 param) {
    const TextDriver *const old = *g_env->driver;
    if (old && old->shutdown) old->shutdown(*g_env->context, driver, param);
    *g_env->driver = driver;
    if (driver->init) {
        s32 result;
        if (*driver->init(&result, g_env->context, param) < 0) {
            *g_env->driver = nullptr;
            *out = -1;
            return out;
        }
    } else {
        *g_env->context = at<void *>(param);
    }
    *out = 0;
    return out;
}

u32 __cdecl Text_LineHeight(u32 ch) {
    TextSize size = {0, 0};
    (*g_env->driver)->measure(*g_env->context, &size, ch);
    return size.height;
}

u32 __cdecl Font_DrawGlyph(const RleSprite *font, s32 x, s32 y, u32 ch, DrawColour) {
    const RleSprite *const g = glyph(font, ch);
    Draw_Sprite(x, y, g);
    return g->width;
}

u32 __cdecl Font_DrawGlyphShape(const RleSprite *font, s32 x, s32 y, u32 ch, DrawColour colour) {
    const RleSprite *const g = glyph(font, ch);
    DrawColour c;
    c.rgb = colour.rgb;
    c.index = colour.index;
    Draw_SpriteShape(x, y, g, c);
    return g->width;
}

u32 __cdecl Font_Measure(const RleSprite *font, TextSize *size, u32 ch) {
    const RleSprite *const g = glyph(font, ch);
    size->width = g->width;
    size->height = g->height;
    return static_cast<u32>(reinterpret_cast<std::uintptr_t>(font));  // EAX left as the font (unused)
}

void __cdecl Draw_Sprite(s32 x, s32 y, const RleSprite *sprite) {
    DrawDriver *const d = *g_env->drawDriver;
    reinterpret_cast<void(__fastcall *)(DrawDriver *, void *, s32, s32, const RleSprite *)>(d->table[kSpriteSlot])(
        d, nullptr, x, y, sprite);
}

void __cdecl Draw_SpriteShape(s32 x, s32 y, const RleSprite *sprite, DrawColour colour) {
    DrawDriver *const d = *g_env->drawDriver;
    DrawColour c;
    c.rgb = colour.rgb;
    c.index = colour.index;
    reinterpret_cast<void(__fastcall *)(DrawDriver *, void *, s32, s32, const RleSprite *, DrawColour)>(
        d->table[kSpriteShapeSlot])(d, nullptr, x, y, sprite, c);
}

}  // extern "C"
