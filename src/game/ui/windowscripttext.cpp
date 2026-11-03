#include "windowscripttext.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "guishape.h"
#include "guitoken.h"
#include "window.h"
#include "window705950.h"
#include "window7059f0.h"
#include "window705bc8.h"
#include "windowfactory.h"
#include "windowreg.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Four numbers the scan writes whole, of which the original reads the low words.
struct Rect {
    s32 a, b, c, d;
};

char *readRect(char *p, Rect &r) { return GuiToken_ReadRect(p, &r.a, &r.b, &r.c, &r.d); }

u32 kind(u8 *window) { return vcall<u32>(window, 0xc); }

// A child from a rectangle read next, when there is one. Made: the answer 0, and *out.
u8 *child(u8 *window, char *p, u32 childKind, u32 flags, u32 id, u32 &answer, u8 **out) {
    Rect r;
    if (!readRect(p, r)) return nullptr;
    u8 *const c = Window_CreateChild(window, nullptr, childKind, flags, id, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d);
    if (c) {
        answer = 0;
        if (out) *out = c;
    }
    return c;
}

void hitRegion(u8 *window, char *p) {
    char shape[0xc];
    p = GuiToken_ReadString(p, shape, 0xc);
    if (!p) return;
    Rect r;
    if (Crt().strcmp(shape, "RECT") == 0) {
        if (!readRect(p, r)) return;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, block ? GuiRect_CtorWith(block, nullptr, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d) : nullptr);
    } else if (Crt().strcmp(shape, "CIRCLE") == 0) {
        p = GuiToken_ReadPoint(p, &r.a, &r.b);
        if (!p) return;
        u16 radius;
        if (!GuiToken_ReadWord(p, &radius)) return;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, block ? GuiCircle_CtorWith(block, nullptr, (u16)r.a, (u16)r.b, radius) : nullptr);
    } else if (Crt().strcmp(shape, "ELLIPSE") == 0) {
        p = GuiToken_ReadPoint(p, &r.a, &r.b);
        if (!p) return;
        if (!GuiToken_ReadPoint(p, &r.c, &r.d)) return;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, block ? GuiEllipse_CtorWith(block, nullptr, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d) : nullptr);
    } else if (Crt().strcmp(shape, "POLY") == 0) {
        u16 word;
        p = GuiToken_ReadWord(p, &word);
        if (!p) return;
        const s16 count = (s16)word;
        u16 *const points = static_cast<u16 *>(GameHeap_Alloc((u32)(s32)count << 2));
        for (s32 left = count; left != 0; --left) {   // (a negative count runs on until it wraps, sic)
            p = GuiToken_ReadPoint(p, &r.a, &r.b);
            if (p) {
                points[2 * (count - left)] = (u16)r.a;
                points[2 * (count - left) + 1] = (u16)r.b;
            }
        }
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xc));
        Window_SetHitArea(window, nullptr, block ? GuiPolygon_CtorPoints(block, nullptr, (u16)count, points) : nullptr);
        GameHeap_Free(points);
    }
}

}  // namespace

extern "C" {

u32 __fastcall WindowScriptText_Line(u8 *window, void *, char *line, u8 **out) {
    u16 command;
    char *p = GuiToken_ReadCommand(line, &command);
    if (!p) return 0xffff;
    u32 answer = command;
    switch ((s16)command) {
    case 0: {   // (the original also widens the name in a local; nothing reads it)
        u16 childKind;
        u32 flags, id;
        char name[0x10];
        Rect r;
        if ((p = GuiToken_ReadKind(p, &childKind)) && (p = GuiToken_ReadDword(p, &flags)) && (p = GuiToken_ReadString(p, name, 0x10)) && (p = GuiToken_ReadDword(p, &id)) && readRect(p, r)) {
            u8 *const c = Window_CreateChild(window, nullptr, (u32)(s32)(s16)childKind, flags, id, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d);
            if (c) {
                if (out) *out = c;
                break;
            }
        }
        answer = 0xffff;
        break;
    }
    case 1: {
        char name[0xc];
        if (GuiToken_ReadString(p, name, 0xc)) Window_SetObject98(window, nullptr, reinterpret_cast<u8 *>(GuiWindows_FindByName(name)));
        break;
    }
    case 2: {
        char name[0xc];
        if (GuiToken_ReadString(p, name, 0xc)) Window_SetImpl(window, nullptr, reinterpret_cast<u8 *>(GuiWindows_FindByName(name)));
        break;
    }
    case 3: {
        Rect r;
        if (readRect(p, r)) {
            Fn_659dac(window + 0x2c, nullptr, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d);
            if (at<u32>(window, 0x48) & 0x400) Window_OffsetByParents(window, nullptr, window + 0x2c);
        }
        break;
    }
    case 4:
        hitRegion(window, p);
        break;
    case 6:
        if (kind(window) == 3 && !Window_FindChild(window, nullptr, 1) && !(at<u32>(window, 0x48) & 0x40)) child(window, p, 2, 0x21, 1, answer, out);
        break;
    case 7:
        if (kind(window) == 3 && !Window_FindChild(window, nullptr, 2) && !(at<u32>(window, 0x48) & 0x40)) child(window, p, 2, 0x21, 2, answer, out);
        break;
    case 8:
        if (kind(window) == 3 && !Window_FindChild(window, nullptr, 3)) {
            u8 *const c = child(window, p, 2, 1, 3, answer, out);
            if (c) Window_SetHandler(c, nullptr, 0x0066bb9b);
        }
        break;
    case 9:
        if (kind(window) == 4 && !Window_FindChild(window, nullptr, 1)) {
            child(window, p, 3, 0x11, 1, answer, out);
        } else if (kind(window) == 7 && !Window_FindChild(window, nullptr, 1)) {
            child(window, p, 3, 0x11, 1, answer, out);
        }
        break;
    case 0xa: {
        Rect r;
        if (kind(window) == 4) {
            if (readRect(p, r)) Window705950_SetInner(window, nullptr, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d);
        } else if (kind(window) == 7) {
            if (readRect(p, r)) Window7059f0_SetInner(window, nullptr, (u16)r.a, (u16)r.b, (u16)r.c, (u16)r.d);
        }
        break;
    }
    case 0xb:
        if (kind(window) == 7) {
            u16 word;
            p = GuiToken_ReadWord(p, &word);
            if (p) {
                const s16 count = (s16)word;
                Window7059f0_SetColumns(window, nullptr, (u16)count);
                for (s16 i = 0; i < count; ++i) {
                    s32 left, right;
                    p = GuiToken_ReadPoint(p, &left, &right);
                    if (p) Window7059f0_SetColumn(window, nullptr, (u16)i, (u16)left, (u16)right);
                }
            }
        }
        break;
    case 0xc:
        if (kind(window) == 7) {
            u16 id;
            char name[0x10];
            if ((p = GuiToken_ReadWord(p, &id)) && (p = GuiToken_ReadString(p, name, 0x10))) child(window, p, 2, 1, (u32)((s32)(s16)id + 0x10), answer, out);
        }
        break;
    case 0xd:
        if (kind(window) == 0xb) {
            s32 low, high;
            if (!GuiToken_ReadPoint(p, &low, &high)) {
                low = 0;
                high = 0x1000;
            }
            low = (s16)low;
            high = (s16)high;
            Window705bc8_SetRange(window, nullptr, &low, &high);
        }
        break;
    default:   // 5 and anything past 0xd
        break;
    }
    return answer;
}

u32 __fastcall WindowScriptText_Lines(u8 *window, void *, char **lines, u8 **out, u32 start) {
    s16 index = (s16)start;
    for (;;) {
        u8 *c;
        const s32 answer = (s16)WindowScriptText_Line(window, nullptr, lines[index], &c);
        if (answer == 0) {
            if (out) *out = c;
            index = (s16)WindowScriptText_Lines(c, nullptr, lines, nullptr, (u32)(s32)index + 1);
        } else if (answer == 5) {
            return (u16)index;
        }
        ++index;
    }
}

u8 *__fastcall WindowScriptText_Build(u8 *window, void *, char **lines, u32 handler) {
    u8 *c = nullptr;
    WindowScriptText_Lines(window, nullptr, lines, &c, 0);
    if (c && handler) Window_SetHandler(c, nullptr, handler);
    return c;
}
}
