#include "windowscript.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "guishape.h"
#include "window.h"
#include "window705950.h"
#include "window7059f0.h"
#include "window705bc8.h"
#include "windowfactory.h"
#include "windowreg.h"

namespace {

u32 &cursor() { return *reinterpret_cast<u32 *>((uintptr_t)0x00faa620); }

u16 next(u8 *script) {
    const u16 word = *reinterpret_cast<u16 *>(script + 2 * cursor());
    ++cursor();
    return word;
}

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

struct Rect {
    u16 a, b, c, d;
};

Rect rect(u8 *script) {
    Rect r;
    WindowScript_ReadRect(script, &r.a, &r.b, &r.c, &r.d);
    return r;
}

// A child of the window made from a rectangle read next, and its own script run on it; the child is returned.
u8 *child(u8 *window, u8 *script, u32 kind, u32 flags, u32 id) {
    const Rect r = rect(script);
    u8 *const made = Window_CreateChild(window, nullptr, kind, flags, id, r.a, r.b, r.c, r.d);
    WindowScript_Run(made, nullptr, script);
    return made;
}

u32 kind(u8 *window) { return vcall<u32>(window, 0xc); }

// Opcode 4: a hit area (1 a rectangle, 2 a circle, 3 an ellipse, 4 a polygon) read next; another sub-opcode does nothing.
void hit_area(u8 *window, u8 *script) {
    const s16 shape = (s16)WindowScript_ReadWord(script);
    switch (shape) {
    case 1: {
        const Rect r = rect(script);
        u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, p ? GuiRect_CtorWith(p, nullptr, r.a, r.b, r.c, r.d) : nullptr);
        break;
    }
    case 2: {
        u16 x, y;
        WindowScript_ReadPair(script, &x, &y);
        const u16 radius = (u16)WindowScript_ReadWord(script);
        u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, p ? GuiCircle_CtorWith(p, nullptr, x, y, radius) : nullptr);
        break;
    }
    case 3: {
        const Rect r = rect(script);
        u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, p ? GuiEllipse_CtorWith(p, nullptr, r.a, r.b, r.c, r.d) : nullptr);
        break;
    }
    case 4: {
        const s16 count = (s16)WindowScript_ReadWord(script);
        u16 *const points = static_cast<u16 *>(GameHeap_Alloc((u32)(s32)count << 2));
        for (s32 left = count; left != 0; --left) {   // (a negative count runs on until it wraps, sic)
            u16 x, y;
            WindowScript_ReadPair(script, &x, &y);
            points[2 * (count - left)] = x;
            points[2 * (count - left) + 1] = y;
        }
        u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, p ? GuiPolygon_CtorPoints(p, nullptr, (u16)count, points) : nullptr);
        GameHeap_Free(points);
        break;
    }
    default:
        break;
    }
}
}  // namespace

extern "C" {

u32 __cdecl WindowScript_ReadDword(u8 *script) {
    u8 *const at = script + 2 * cursor();
    cursor() += 2;
    return *reinterpret_cast<u32 *>(at);
}

u32 __cdecl WindowScript_ReadWord(u8 *script) {
    const u32 old = cursor();
    return (old & 0xffff0000u) | next(script);
}

u16 *__cdecl WindowScript_ReadRect(u8 *script, u16 *a, u16 *b, u16 *c, u16 *d) {
    *a = next(script);
    *b = next(script);
    *c = next(script);
    *d = next(script);
    return d;
}

u16 *__cdecl WindowScript_ReadPair(u8 *script, u16 *a, u16 *b) {
    *a = next(script);
    *b = next(script);
    return b;
}

u8 *__fastcall WindowScript_Run(u8 *window, void *, u8 *script) {
    u8 *made = nullptr;
    for (bool running = true; running;) {
        switch ((s16)WindowScript_ReadWord(script)) {
        case 0: {
            const s16 kind_ = (s16)WindowScript_ReadWord(script);
            const u32 flags = WindowScript_ReadDword(script);
            const u32 id = WindowScript_ReadDword(script);
            made = child(window, script, (u32)(s32)kind_, flags, id);
            break;
        }
        case 1:
            Window_SetObject98(window, nullptr, reinterpret_cast<u8 *>(GuiWindows_FindByHash((s32)WindowScript_ReadDword(script))));
            break;
        case 2:
            Window_SetImpl(window, nullptr, reinterpret_cast<u8 *>(GuiWindows_FindByHash((s32)WindowScript_ReadDword(script))));
            break;
        case 3: {
            const Rect r = rect(script);
            Fn_659dac(window + 0x2c, nullptr, r.a, r.b, r.c, r.d);
            if (at<u32>(window, 0x48) & 0x400) Window_OffsetByParents(window, nullptr, window + 0x2c);
            break;
        }
        case 4:
            hit_area(window, script);
            break;
        case 6:
        case 0xe:
            made = child(window, script, 2, 0x21, 1);
            break;
        case 7:
        case 0xf:
            made = child(window, script, 2, 0x21, 2);
            break;
        case 8: {
            const Rect r = rect(script);
            made = Window_CreateChild(window, nullptr, 2, 1, 3, r.a, r.b, r.c, r.d);
            Window_SetHandler(made, nullptr, 0x0066bb9b);
            WindowScript_Run(made, nullptr, script);
            break;
        }
        case 9:
            if (kind(window) == 4 && !Window_FindChild(window, nullptr, 1)) {
                made = child(window, script, 3, 0x10, 1);
            } else if (kind(window) == 7 && !Window_FindChild(window, nullptr, 1)) {
                made = child(window, script, 3, 0x10, 1);
            }
            break;
        case 0xa:
            if (kind(window) == 4) {
                const Rect r = rect(script);
                Window705950_SetInner(window, nullptr, r.a, r.b, r.c, r.d);
            } else if (kind(window) == 7) {
                const Rect r = rect(script);
                Window7059f0_SetInner(window, nullptr, r.a, r.b, r.c, r.d);
            }
            break;
        case 0xb: {
            const s16 count = (s16)WindowScript_ReadWord(script);
            Window7059f0_SetColumns(window, nullptr, (u16)count);
            for (s16 i = 0; i < count; ++i) {
                u16 left, right;
                WindowScript_ReadPair(script, &left, &right);
                Window7059f0_SetColumn(window, nullptr, (u16)i, left, right);
            }
            break;
        }
        case 0xc: {
            const s16 id = (s16)WindowScript_ReadWord(script);
            made = child(window, script, 2, 1, (u32)((s32)id + 0x10));
            break;
        }
        case 0xd: {
            const s32 low = (s16)WindowScript_ReadWord(script);
            const s32 high = (s16)WindowScript_ReadWord(script);
            Window705bc8_SetRange(window, nullptr, &low, &high);
            break;
        }
        case 0x11:
            Fn_661830(window, nullptr, WindowScript_ReadDword(script));
            break;
        case 0x12:
            Fn_661850(window, nullptr, WindowScript_ReadDword(script));
            break;
        case 0x10: {
            const s16 id = (s16)WindowScript_ReadWord(script);
            made = child(window, script, 6, 1, (u32)(s32)id | 0x11100000u);
            break;
        }
        default:   // 5 ends the window's script, and so does any opcode past 0x12
            running = false;
            break;
        }
    }
    return made;
}

u8 *__fastcall WindowScript_Build(u8 *window, void *, u8 *script, u32 handler) {
    cursor() = 0;
    u8 *const made = WindowScript_Run(window, nullptr, script);
    if (made && handler) Window_SetHandler(made, nullptr, handler);
    return made;
}
}
