// Text through the text driver: see text.h.
#include "text.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/methods.h"
#include "../core/vcall.h"

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

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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

u32 __cdecl Text_SelectFont(u32 index, u32 mode) {
    const u8 *const table = *at<const u8 *const *>(0x007b059c);
    if (!table) return 0;
    const u32 font = *reinterpret_cast<const u32 *>(table + index * 0x18 + 0xc);
    if (!font) return 0;
    s32 result;
    if (mode == 1)
        Text_SetDriver(&result, at<const TextDriver *>(0x007031b0), font);
    else if (mode == 0)
        Text_SetDriver(&result, at<const TextDriver *>(0x007031c0), font);
    return 1;
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

u32 __fastcall TextMetrics_Width(u8 *object, void *, const u16 *text, u32 start, u32 end) {
    u32 eax = (u32)(s32)(s16)start;
    s32 remaining = (s32)(s16)end - (s32)(s16)start;
    const u16 *p = text + (s32)(s16)start;
    s32 sum = 0;
    s32 i = 0;
    if (remaining > 0) {
        do {
            const u16 c = *p;
            eax = (eax & 0xffff0000u) | c;
            if (c == 0) break;
            const u32 result = vcall<u32>(object, 0xc, eax);
            ++p;
            eax = (u32)(s32)(s16)result;
            sum += (s32)eax;
            ++i;
        } while (i < remaining);
    }
    return (eax & 0xffff0000u) | (u16)sum;
}

s32 __fastcall TextMetrics_FitIndex(u8 *object, void *, const u16 *text, u32 start, u32 width) {
    u32 eax = (u32)(uintptr_t)text;
    s32 index = (s32)(s16)start;
    const u16 *p = text + index;
    u32 left = width;
    if ((s16)left > 0) {
        do {
            const u16 c = *p;
            eax = (eax & 0xffff0000u) | c;
            if (c == 0) break;
            eax = vcall<u32>(object, 0xc, eax);
            left -= eax;
            ++index;
            ++p;
        } while ((s16)left > 0);
    }
    return index - 1;
}

u32 __fastcall TextMetrics_ForwardSlot8(u8 *object) {
    u8 *const inner = at<u8 *>(object, 0x14);
    if (!inner)
        return 0;
    return vcall<u16>(inner, 0x20);
}

u32 __fastcall TextMetrics_Lookup(u8 *object, void *, u32 key) {
    u8 *const inner = at<u8 *>(object, 0x14);
    if (!inner)
        return 0;
    key = Ranges_LookupAdapter(*reinterpret_cast<const u8 *const *>((uintptr_t)0x0078ba80), nullptr, key);
    u32 local[3];  // (the original's: [0] = &key, [1] handed to slot 7, [2] unused)
    local[0] = static_cast<u32>(reinterpret_cast<uintptr_t>(&key));
    const u32 found = vcall<u32>(inner, 0x14, local);
    const u16 *const value = vcall<const u16 *>(inner, 0x1c, &local[1], found);
    return (static_cast<u32>(reinterpret_cast<uintptr_t>(value)) & 0xffff0000u) | *value;
}

u8 *__fastcall Obj_CtorValue(u8 *object, void *, u32 value) {
    Fn_657d30(object);
    at<u32>(object, 0) = 0x006ff058;
    at<u32>(object, 0x14) = value;
    return object;
}
}  // extern "C"
