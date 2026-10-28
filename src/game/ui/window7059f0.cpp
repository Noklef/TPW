#include "window7059f0.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/selectors.h"
#include "../core/stdwstring.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "window705b20.h"
#include "windowfactory.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Window7059f0_Layout(u8 *window) {
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    GuiRect_Assign(window + 0x138, nullptr, window + 0x2c);
}

u8 *__fastcall Window7059f0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    GuiRect_Ctor(window + 0x138);
    at<u32>(window, 0) = 0x7059f0;
    u32 bar_flags = 0x10;
    at<u32>(window, 0x14c) = 0xffffffff;
    at<u32>(window, 0x144) = 0;
    at<u32>(window, 0x148) = 0;
    at<u32>(window, 0x174) = 0;
    at<u16>(window, 0x170) = 0;
    at<u16>(window, 0x172) = 0;
    at<u32>(window, 0x154) = 0;
    at<u32>(window, 0x150) = 0;
    at<u32>(window, 0x158) = 0;
    at<u32>(window, 0x160) = 0xffffffff;
    at<u16>(window, 0x164) = 0;
    at<u16>(window, 0x166) = 0;
    at<u32>(window, 0x16c) = 0;
    at<u16>(window, 0x168) = 0;
    at<u16>(window, 0x16a) = 0;
    Window7059f0_Layout(window);
    if (flags & 0x1000) {
        if (flags & 0x40) bar_flags |= 0x40;
        u8 *const bar = Window_CreateChild(window, nullptr, 3, bar_flags, 1, (u32)(at<s16>(window, 0x34) + 4), at<u16>(window, 0xe),
                                           at<u16>(window, 0x10), at<u16>(window, 0x12));
        if (bar) {
            vcall<void>(bar, 0x14, 0u);
            Window_SetShown(bar, nullptr, 1);
            Window705b20_SetRange(bar, nullptr, 0, 0);
        }
    }
    at<u32>(window, 0x114) = 0x665c35;
    at<u32>(window, 0x118) = at<u32>(window, 0x114);
    return window;
}

u32 __fastcall Window7059f0_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x7059f0;
    Window7059f0_Clear(window);
    if (u8 *const columns = at<u8 *>(window, 0x16c)) vcall<u32>(columns, 0x10, 3u);
    if (at<u32>(window, 0x148)) {
        for (s32 i = 0; i < at<s16>(window, 0x172); ++i) {
            if (u8 *const row = at<u8 **>(window, 0x148)[i]) vcall<u32>(row, 8, 1u);
        }
        GameHeap_Free(at<void *>(window, 0x148));
    }
    if (at<u32>(window, 0x144)) GameHeap_Free(at<void *>(window, 0x144));
    if (u8 *const highlight = at<u8 *>(window, 0x174)) {
        if ((s16)WindowImpl_Release(highlight) == -1) vcall<u32>(highlight, 0, 1u);
    }
    GuiRect_Dtor(window + 0x138);
    return Window_Dtor(window);
}

void __fastcall Window7059f0_SetColumns(u8 *window, void *, u32 count) {
    if (at<s16>(window, 0x164) != 0) return;
    const s32 n = (s16)count;
    u8 *columns = nullptr;
    if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc((u32)(n * 0xc + 4)))) {
        at<s32>(block, 0) = n;
        CRT_VectorCtorIterator(block + 4, 0xc, n, reinterpret_cast<void(__fastcall *)(void *)>(&GuiRect_Ctor), reinterpret_cast<void *>(&GuiRect_Dtor));
        columns = block + 4;
    }
    at<u8 *>(window, 0x16c) = columns;
    for (s32 i = 0; i < n; ++i) Fn_659dac(at<u8 *>(window, 0x16c) + i * 0xc, nullptr, (u32)(i << 5), 0, (u32)((i + 1) << 5), 1);
    at<u16>(window, 0x164) = (u16)count;
}

u32 __fastcall Window7059f0_SetColumn(u8 *window, void *, u32 index, u32 left, u32 right) {
    const s32 i = (s16)index;
    if (i >= at<s16>(window, 0x164)) return (u32)i;
    if (!at<u8 *>(window, 0x16c)) return (u32)(uintptr_t)window;
    return Fn_659dac(at<u8 *>(window, 0x16c) + i * 0xc, nullptr, (u16)left, 0, (u16)right, 1);
}

u32 __fastcall Window7059f0_SetInner(u8 *window, void *, u32 left, u32 top, u32 right, u32 bottom) {
    return Fn_659dac(window + 0x138, nullptr, (u16)left, (u16)top, (u16)right, (u16)bottom);
}

u32 __fastcall Window7059f0_Item148Depth(const u8 *window) {
    u8 *const *const cell = at<u8 *const *>(const_cast<u8 *>(window), 0x148);
    return cell && *cell ? at<u16>(*cell, 0xe0) : 0;
}

s32 __fastcall Window7059f0_ColumnBottom(u8 *window, void *, u32 column) {
    return at<s16>(at<u8 *>(window, 0x16c) + (s16)column * 0xc, 0xa);
}

void __fastcall Window7059f0_SetColumnBottom(u8 *window, void *, u32 column, u32 value) {
    if ((s16)column < at<s16>(window, 0x164) && at<u8 *>(window, 0x16c))
        at<u16>(at<u8 *>(window, 0x16c) + (s16)column * 0xc, 0xa) = (u16)value;   // (a negative column writes before the array, sic)
}

void __fastcall Window7059f0_OffsetColumns(u8 *window, void *, u32 dx, u32 dy) {
    if (!at<u8 *>(window, 0x16c)) return;
    for (s32 i = at<s16>(window, 0x164); i != 0; --i)   // (last first; a negative count runs on, sic)
        vcall<void>(at<u8 *>(window, 0x16c) + (i - 1) * 0xc, 8, dx, dy);
}

s32 __fastcall Window7059f0_NextItem(u8 *window, void *, s32 item, s32 *level) {
    const s32 *const links = at<s32 *>(window, 0x144);
    s32 next = -2, depth = 0;
    if (links[item + 2] >= 0) {   // the first child
        next = links[item + 2];
        ++depth;
    } else if (links[item] >= 0) {   // the next sibling
        next = links[item];
    } else if (links[item + 3] >= 0) {   // up to the first parent with a next sibling (or the last parent's next: -1 or less)
        next = links[item + 3];
        --depth;
        while (links[next] < 0 && links[next + 3] >= 0) {
            next = links[next + 3];
            --depth;
        }
        next = links[next];
    }
    if (level) *level = depth;
    return next;
}

s32 __fastcall Window7059f0_NthItem(u8 *window, void *, s32 n) {
    s32 item = at<s32>(window, 0x160);
    for (s32 i = 0; n >= 0 && n < at<s32>(window, 0x150) && at<u32>(window, 0x144) && i < n && item != -1; ++i)
        item = Window7059f0_NextItem(window, nullptr, item, nullptr);
    return item;
}

u32 __fastcall Window7059f0_SetNthItemData(u8 *window, void *, s32 n, u32 data) {
    const s32 item = Window7059f0_NthItem(window, nullptr, n);
    if (item == -1) return 0;
    at<s32 *>(window, 0x144)[item + 1] = (s32)data;
    return 1;
}

void __fastcall Window7059f0_SetRowHeight(u8 *window, void *, u32 height) {
    at<u16>(window, 0x170) = (u16)height;
    at<u16>(window, 0x172) = (u16)((at<s16>(window, 0x142) - at<s16>(window, 0x13e)) / at<s16>(window, 0x170));
}

u16 __fastcall Window7059f0_SetRowSize(u8 *window, void *, u8 *row) {
    if (row) {
        s16 width, height;
        Window_GetSize(row, nullptr, &width, &height, 1);
        at<s16>(window, 0x170) = height;
        at<u16>(window, 0x172) = (u16)((at<s16>(window, 0x142) - at<s16>(window, 0x13e)) / at<s16>(window, 0x170));
        if (!at<u32>(window, 0x148)) at<void *>(window, 0x148) = GameHeap_Alloc((u32)((s32)at<s16>(window, 0x172) << 2));
        if (u8 *const highlight = at<u8 *>(window, 0x174)) {
            u8 rect[12];
            GuiRect_CtorWith(rect, nullptr, at<u16>(window, 0x13c), at<u16>(window, 0x13e), at<u16>(window, 0x140),
                             (u32)((s32)at<s16>(window, 0x13e) + at<s16>(window, 0x170)));
            u8 event[0x28];  // (only +0x14, the rectangle, and +0x24 set)
            at<u8 *>(event, 0x14) = rect;
            at<u32>(event, 0x24) = 0x10;
            vcall<void>(highlight, 0xc, 0x10u, event);
            GuiRect_Dtor(rect);
        }
    }
    return at<u16>(window, 0x172);
}

void __fastcall Window7059f0_SetRow(u8 *window, void *, s32 index, u8 *row) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    if (row && at<u32>(window, 0x148) && at<u32>(window, 0x16c) && index < at<s16>(window, 0x172)) {
        at<u8 **>(window, 0x148)[index] = row;
        u8 *w = row;
        const s16 top = (s16)(at<s16>(window, 0x13e) + (s32)at<s16>(window, 0x170) * index);
        for (s32 i = 0; i < at<s16>(window, 0x164); ++i) {
            u8 *const column = at<u8 *>(window, 0x16c) + 0xc * i;
            Window_SetRectTo(w, nullptr, (u32)(s32)at<s16>(column, 4), (u32)(s32)top, (u32)(s32)at<s16>(column, 8),
                             (u32)((s32)top + at<s16>(window, 0x170)));
            Window_SetDepth(w, nullptr, (u32)(at<s16>(window, 0xe0) + 3));
            w = reinterpret_cast<u8 *>((uintptr_t)(i == 0 ? Fn_65d547(w, nullptr, 0) : Fn_65d56b(w)));
        }
    }
    GuiRect_Dtor(rect);
}

void __fastcall Window7059f0_SetHighlight(u8 *window, void *, u8 *highlight) {
    if (highlight == at<u8 *>(window, 0x174)) return;
    if (u8 *const old = at<u8 *>(window, 0x174)) {
        if ((s16)WindowImpl_Release(old) == -1) vcall<u32>(old, 0, 1u);
    }
    at<u8 *>(window, 0x174) = highlight;
    if (!highlight) return;
    Fn_6617f0(highlight);
    u8 event[0x28];  // (only the words +4-+9, +0x14 (the rectangle) and +0x24 set)
    at<u16>(event, 4) = 0;
    at<u16>(event, 6) = (u16)(at<s16>(window, 0xe0) + 2);
    at<u16>(event, 8) = at<u16>(window, 0xe2);
    u8 rect[12];
    GuiRect_CtorWith(rect, nullptr, at<u16>(window, 0x13c), at<u16>(window, 0x13e), at<u16>(window, 0x140),
                     (u32)((s32)at<s16>(window, 0x13e) + at<s16>(window, 0x170)));
    at<u8 *>(event, 0x14) = rect;
    at<u32>(event, 0x24) = 0x17;
    vcall<void>(highlight, 0xc, 0x17u, event);
    if ((s16)Window_IsShown(window) != 0 && at<s32>(window, 0x14c) != -1) vcall<void>(at<u8 *>(window, 0x174), 0x14, 1u);
    GuiRect_Dtor(rect);
}

void __fastcall Window7059f0_ShowRows(u8 *window, void *, u32 shown) {
    const bool on = (s16)shown != 0;
    if (at<u32>(window, 0x148)) {
        for (s32 i = 0; i < at<s16>(window, 0x172); ++i) {
            const u32 show = ((s32)((u32)i + at<u32>(window, 0x154)) < at<s32>(window, 0x150) && on) ? 1 : 0;
            if (u8 *const row = at<u8 **>(window, 0x148)[i]) Window_SetShown(row, nullptr, show);
        }
    }
    if (u8 *const highlight = at<u8 *>(window, 0x174)) vcall<void>(highlight, 0x14, (at<s32>(window, 0x14c) != -1 && on) ? 1u : 0u);
}

void __fastcall Window7059f0_SendToRows(u8 *window, void *, u32 message, u32 a, u32 b) {
    if (!at<u32>(window, 0x148)) return;
    s32 item = Window7059f0_NthItem(window, nullptr, 0);
    for (s32 i = 0; i < at<s16>(window, 0x172); ++i) {
        u8 *w = at<u8 **>(window, 0x148)[i];
        if (!w) continue;
        if (message == 0x1e && item != -1) {
            b = (u32)at<s32 *>(window, 0x144)[item + 1];
            item = Window7059f0_NextItem(window, nullptr, item, nullptr);
        }
        for (s32 column = 0; column < at<s16>(window, 0x164); ++column) {
            if (message == 0x1e) a = (u32)i << 16 | (u32)column;
            Window_SendMessage(w, message, a, b);
            w = reinterpret_cast<u8 *>((uintptr_t)(column == 0 ? Fn_65d547(w, nullptr, 0) : Fn_65d56b(w)));
        }
    }
}

s32 __fastcall Window7059f0_FindItemData(u8 *window, void *, u32 data) {
    s32 item = Window7059f0_NthItem(window, nullptr, 0), position = -1;
    bool found = false;
    while (item != -1 && !found && position < at<s32>(window, 0x150) - 1) {
        ++position;
        if ((u32)at<s32 *>(window, 0x144)[item + 1] == data) found = true;
        item = Window7059f0_NextItem(window, nullptr, item, nullptr);
    }
    return found ? position : -1;
}

void __fastcall Window7059f0_ResetItems(u8 *window) {
    const s32 count = at<s32>(window, 0x158) / ((s32)at<s16>(window, 0x164) + 4);
    s32 item = 0;
    for (s32 i = 0; i < count - 1; ++i) {
        at<s32 *>(window, 0x144)[item] = item + at<s16>(window, 0x164) + 4;
        at<s32 *>(window, 0x144)[item + 2] = -1;
        at<s32 *>(window, 0x144)[item + 3] = -2;
        item += at<s16>(window, 0x164) + 4;
    }
    at<s32 *>(window, 0x144)[item] = -1;
    at<s32 *>(window, 0x144)[item + 2] = -1;
    at<s32 *>(window, 0x144)[item + 3] = -2;
}

void __fastcall Window7059f0_RootEnd(u8 *window, void *, s32 *end, s32 *last) {
    s32 item = at<s32>(window, 0x160), before = -1;
    if (!at<s32 *>(window, 0x144)) return;
    for (s32 next = Window7059f0_NthItem(window, nullptr, 0); next >= 0 && at<s32 *>(window, 0x144)[next + 3] != -2;) {
        before = item;
        next = at<s32 *>(window, 0x144)[next];
        item = next;
    }
    *end = item;
    *last = before;
}

s32 __fastcall Window7059f0_NthChild(u8 *window, void *, s32 n, s32 parent) {
    s32 item;
    if (parent > -1) {
        item = -1;
        const s32 k = Window7059f0_NthItem(window, nullptr, parent);
        if (k > -1) item = at<s32 *>(window, 0x144)[k + 2];
    } else {
        item = at<s32>(window, 0x160);
    }
    if (n >= 0 && n < at<s32>(window, 0x150) && at<s32 *>(window, 0x144))
        for (s32 i = 0; i < n && item > -1; ++i) item = at<s32 *>(window, 0x144)[item];
    return item;
}

namespace {
// Window7059f0_Sort's order: whether the cell a comes before the cell b (numbers or the strings they point to; the mode +0x16a).
// The cells are read only for the modes 0 and 1.
bool sorts_first(u8 *window, bool numeric, const s32 *a, const s32 *b) {
    const s16 mode = at<s16>(window, 0x16a);
    if (mode != 0 && mode != 1) return false;
    if (numeric) return mode == 0 ? *a < *b : *a > *b;
    const s32 order = WString_Compare(reinterpret_cast<const u8 *>((uintptr_t)(u32)*a), nullptr, reinterpret_cast<const u8 *>((uintptr_t)(u32)*b));
    return mode == 0 ? order < 0 : order > 0;
}

const s32 *cell(u8 *window, s32 item) { return &at<s32 *>(window, 0x144)[item + at<s16>(window, 0x168) + 4]; }
}  // namespace

void __fastcall Window7059f0_Sort(u8 *window, void *, s32) {
    if (!at<u8 *>(window, 0x16c) || at<s32>(window, 0x150) < 2) return;
    const bool numeric = at<s16>(at<u8 *>(window, 0x16c), at<s16>(window, 0x168) * 12 + 0xa) == 1;
    s32 *items = at<s32 *>(window, 0x144);
    s32 cur = at<s32>(window, 0x160), prev = -1, best = cur, best_prev = -1;
    const s32 *best_cell = &items[cur + at<s16>(window, 0x168) + 4];
    for (s32 i = 0; i < at<s32>(window, 0x150) && at<s32 *>(window, 0x144)[cur + 3] != -2 && cur != -1; ++i) {
        if (sorts_first(window, numeric, &items[cur + at<s16>(window, 0x168) + 4], best_cell)) {
            best_cell = &items[cur + at<s16>(window, 0x168) + 4];
            best = cur;
            best_prev = prev;
        }
        prev = cur;
        cur = items[cur];
    }
    if (best < 0) return;
    if (best != at<s32>(window, 0x160)) {
        const s32 old = at<s32>(window, 0x160);
        if (best_prev != -1) at<s32 *>(window, 0x144)[best_prev] = at<s32 *>(window, 0x144)[best];
        at<s32 *>(window, 0x144)[best] = old;
        at<s32>(window, 0x160) = best;
    }
    if (at<s32>(window, 0x150) <= 2) return;
    Window7059f0_NthChild(window, nullptr, 0, -1);
    Window7059f0_NthChild(window, nullptr, 1, -1);
    cur = Window7059f0_NthChild(window, nullptr, 2, -1);
    for (s32 i = 2; i < at<s32>(window, 0x150) && at<s32 *>(window, 0x144)[cur + 3] != -2 && cur >= 0; ++i) {
        bool found = false;
        const s32 p = Window7059f0_NthChild(window, nullptr, i - 1, -1);
        const s32 x = at<s32 *>(window, 0x144)[p];
        cur = at<s32 *>(window, 0x144)[x];
        s32 q = Window7059f0_NthChild(window, nullptr, 0, -1);
        s32 s = at<s32 *>(window, 0x144)[q];
        s32 t = at<s32 *>(window, 0x144)[s];
        s32 parent = at<s32 *>(window, 0x144)[s + 3];
        while (!found && parent > -2 && s != x) {
            found = sorts_first(window, numeric, cell(window, x), cell(window, s));
            if (!found) {
                q = s;
                s = t;
                t = at<s32 *>(window, 0x144)[s];
                parent = at<s32 *>(window, 0x144)[s + 3];
            }
        }
        if (found && q != x) {
            at<s32 *>(window, 0x144)[p] = cur;
            at<s32 *>(window, 0x144)[q] = x;
            at<s32 *>(window, 0x144)[x] = s;
        }
    }
}

void __cdecl Window7059f0_FitBar(u8 *bar, s32 count, s32 rows, u32 first) {
    if (!bar) return;
    if (count > rows) {
        Window_SetShown(bar, nullptr, 1);
        Window705b20_SetRange(bar, nullptr, (s32)((u32)count - (u32)rows), 0);
        vcall<void>(bar, 0x1c, first);
    } else {
        Window_SetShown(bar, nullptr, 0);
        Window705b20_SetRange(bar, nullptr, 0, 0);
    }
}

void __fastcall Window7059f0_FreeItem(u8 *window, void *, u32 unused, s32 item, s32 previous, u32 siblings) {
    const s32 child = at<s32 *>(window, 0x144)[item + 2];
    if (child >= 0) Window7059f0_FreeItem(window, nullptr, unused, child, -1, 1);
    if (siblings) {
        const s32 next = at<s32 *>(window, 0x144)[item];
        if (next >= 0) Window7059f0_FreeItem(window, nullptr, unused, next, item, 1);
    }
    for (s32 column = 0; column < at<s16>(window, 0x164); ++column) {
        if (Window7059f0_ColumnBottom(window, nullptr, (u32)column) != 0) continue;
        if (u8 *const object = reinterpret_cast<u8 *>((uintptr_t)(u32)at<s32 *>(window, 0x144)[item + 4 + column]))
            CountedBuf_Deleter(object, nullptr, 1);
    }
    s32 end, last;   // (both unset without the array, sic)
    Window7059f0_RootEnd(window, nullptr, &end, &last);
    if (last != item) {   // (the item moved to the end of the first shown one's siblings)
        if (previous >= 0) {
            at<s32 *>(window, 0x144)[previous] = at<s32 *>(window, 0x144)[item];
        } else {
            const s32 parent = at<s32 *>(window, 0x144)[item + 3];
            if (parent >= 0) at<s32 *>(window, 0x144)[parent + 2] = -1;   // (its other children dropped, sic)
        }
        at<s32 *>(window, 0x144)[item] = end;
        at<s32 *>(window, 0x144)[last] = item;
    }
    at<s32 *>(window, 0x144)[item + 2] = -1;
    at<s32 *>(window, 0x144)[item + 3] = -2;
    if (--at<s32>(window, 0x150) == 0) at<s32>(window, 0x160) = -1;
}

void __fastcall Window7059f0_SelectAt(u8 *window, void *, u32 x, u32 y) {
    if (!at<s16>(window, 0x170)) return;
    if (!(s16)vcall<u32>(window + 0x138, 0, x, y)) return;   // (the inner rectangle's slot 0: the point in it)
    const s32 row = ((s32)(s16)y - at<s16>(window, 0x13e)) / at<s16>(window, 0x170);
    if (row >= 0) Window7059f0_Select(window, nullptr, row);
}

void __fastcall Window7059f0_Select(u8 *window, void *, s32 row) {
    if (row < 0) {
        row = -1;
    } else if (row < at<s16>(window, 0x172) && row < at<s32>(window, 0x150)) {
        row += at<s32>(window, 0x154);
    } else if (at<u32>(window, 0x48) & 0x100) {
        row = -1;
    } else {
        return;
    }
    if (row != at<s32>(window, 0x14c)) {
        if (at<u8 *>(window, 0x174)) {
            if (row == -1 && at<s32>(window, 0x14c) != -1)
                vcall<void>(at<u8 *>(window, 0x174), 0x14, 0u);
            else if (row != -1 && at<s32>(window, 0x14c) == -1 && (s16)Window_IsShown(window) != 0)
                vcall<void>(at<u8 *>(window, 0x174), 0x14, 1u);
        }
        at<s32>(window, 0x14c) = row;
        Window_PostMessage(at<u8 *>(window, 0x60), 0x401, at<u32>(window, 0x8c), at<u32>(window, 0x14c));
        if (*reinterpret_cast<const u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
    }
    if (!at<u8 *>(window, 0x174) || at<s32>(window, 0x14c) == -1) return;
    const u32 top = (u32)(s32)at<s16>(window, 0x13e), height = (u32)(s32)at<s16>(window, 0x170);
    const u32 bottom = top + height * ((u32)at<s32>(window, 0x14c) + 1 - (u32)at<s32>(window, 0x154));
    u8 rect[0xc], event[0x18];   // (only the event's pointer +0x14 set)
    GuiRect_CtorWith(rect, nullptr, at<u16>(window, 0x13c), top + height * ((u32)at<s32>(window, 0x14c) - (u32)at<s32>(window, 0x154)),
                     at<u16>(window, 0x140), bottom);
    at<u8 *>(event, 0x14) = rect;
    vcall<void>(at<u8 *>(window, 0x174), 0xc, 0x10u, event);
    GuiRect_Dtor(rect);
}

s32 __fastcall Window7059f0_FindText(u8 *window, void *, const u16 *text, s32 column) {
    s32 item = Window7059f0_NthItem(window, nullptr, 0), position = -1, found = -1;
    bool done = false;
    if (column < 1) column = 1;
    if (column > at<s16>(window, 0x164)) column = at<s16>(window, 0x164);
    --column;
    if (!at<u8 *>(window, 0x16c) || Window7059f0_ColumnBottom(window, nullptr, (u32)column) != 0) return found;
    while (item != -1 && !done && position < at<s32>(window, 0x150) - 1) {
        ++position;
        const u8 *const cell = reinterpret_cast<const u8 *>((uintptr_t)(u32)at<s32 *>(window, 0x144)[item + column + 4]);
        if (WString_CompareText(cell, nullptr, text) == 0) {
            done = true;
            found = position;
        }
        item = Window7059f0_NextItem(window, nullptr, item, nullptr);
    }
    return found;
}

void __fastcall Window7059f0_RefreshRows(u8 *window) {
    s32 *cells = at<s32 *>(window, 0x144);
    if (!at<u8 **>(window, 0x148)) return;
    s32 item = Window7059f0_NthItem(window, nullptr, at<s32>(window, 0x154));
    const s32 enabled = (s16)Window_IsShown(window);
    s32 level = 0;
    if (item >= 0)
        for (s32 p = at<s32 *>(window, 0x144)[item + 3]; p >= 0; p = at<s32 *>(window, 0x144)[p + 3]) ++level;
    for (s32 row = 0; row < at<s16>(window, 0x172); ++row) {
        u8 *child = at<u8 **>(window, 0x148)[row];
        const bool visible = row + at<s32>(window, 0x154) < at<s32>(window, 0x150);
        Window_SetShown(child, nullptr, enabled && visible ? 1 : 0);
        if (visible) cells = at<s32 *>(window, 0x144) + item + 4;
        for (s32 column = 0; column < at<s16>(window, 0x164); ++column) {
            if (visible) {
                Window_SendMessage(child, 0x403, (u32)(uintptr_t)(cells + 4), (u32)at<s32 *>(window, 0x144)[item + 1]);
                if (Window7059f0_ColumnBottom(window, nullptr, (u32)column) == 1) {
                    vcall<void>(child, 0x1c, (u32)cells[column]);
                } else {
                    if (column == 0) Window_NotifyC000(child, nullptr, (u32)(level * 0x14), 0);
                    vcall<void>(child, 0x24, Obj_Get4Or6fd6c4(reinterpret_cast<const u8 *>((uintptr_t)(u32)cells[column])));
                }
            }
            child = reinterpret_cast<u8 *>((uintptr_t)(column == 0 ? Fn_65d547(child, nullptr, 0) : Fn_65d56b(child)));
        }
        if (visible) {
            s32 delta;
            item = Window7059f0_NextItem(window, nullptr, item, &delta);
            level += delta;
        }
    }
}

void __fastcall Window7059f0_BuildRows(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *)) {
    if (!make) return;
    u8 *row = make();
    s16 count = (s16)Window7059f0_SetRowSize(window, nullptr, row);
    if (u8 *const bar = Window_FindChild(window, nullptr, 1)) Fn_66b53e(bar, nullptr, (u32)(s32)count);
    if (count) {
        Window7059f0_SetRow(window, nullptr, 0, row);   // (the first row made kept at 0)
        if (release) release(row);
        for (--count; count; --count) {   // (then rows count-1 down to 1, each made anew)
            row = make();
            Window7059f0_SetRow(window, nullptr, count, row);
            if (release) release(row);
        }
    }
    Window7059f0_RefreshRows(window);
}

s32 *__fastcall Window7059f0_Grow(u8 *window, void *, s32 n) {
    if (!n || at<s16>(window, 0x164) == 0) return nullptr;
    const s32 added = n * (at<s16>(window, 0x164) + 4);
    s32 *const old = at<s32 *>(window, 0x144);
    s32 end, last;   // (unset without the array; then unused)
    if (old) Window7059f0_RootEnd(window, nullptr, &end, &last);
    s32 *const links = static_cast<s32 *>(GameHeap_Alloc((u32)(at<s32>(window, 0x158) + added) << 2));
    if (!links) return nullptr;
    s32 item = at<s32>(window, 0x158);
    std::memset(links, 0, (u32)(at<s32>(window, 0x158) + added) << 2);
    for (s32 i = 0; i < n - 1; ++i) {   // (the new items free, each the next one's previous)
        links[item] = item + at<s16>(window, 0x164) + 4;
        links[item + 2] = -1;
        links[item + 3] = -2;
        item += at<s16>(window, 0x164) + 4;
    }
    links[item] = -1;
    links[item + 2] = -1;
    links[item + 3] = -2;
    if (old) {
        std::memcpy(links, old, (u32)at<s32>(window, 0x158) << 2);
        links[last] = at<s32>(window, 0x158);   // (last -1: the dword before the block, sic)
        GameHeap_Free(old);
    }
    at<s32 *>(window, 0x144) = links;
    at<s32>(window, 0x158) += added;
    return links;
}

void __fastcall Window7059f0_CopyCells(u8 *window, void *, const u32 *texts, u32 *cells) {
    u8 allocator;   // (never set: its byte goes into each new string, sic)
    for (s32 column = 0; column < at<s16>(window, 0x164); ++column, ++texts, ++cells) {
        if (Window7059f0_ColumnBottom(window, nullptr, (u32)column) != 0) {
            *cells = *texts;
        } else if (*texts) {
            void *const block = GameHeap_Alloc(0x10);
            *cells = block ? (u32)(uintptr_t)StdWString_CtorChars(static_cast<StdWString *>(block), nullptr,
                                                                  reinterpret_cast<const u16 *>((uintptr_t)*texts), &allocator)
                           : 0;
        }
    }
}

u32 __fastcall Window7059f0_SortsBefore(u8 *window, void *, const u32 *texts, const u32 *cells) {
    const s32 column = at<s16>(window, 0x168);
    const bool numbers = Window7059f0_ColumnBottom(window, nullptr, at<u16>(window, 0x168)) == 1;
    if (at<s16>(window, 0x16a) == 0) {
        if (numbers) return (s32)texts[column] < (s32)cells[column] ? 1 : 0;
        if (cells[column] == 0) return 1;
        const u32 cell = Obj_Get4Or6fd6c4(reinterpret_cast<const u8 *>((uintptr_t)cells[column]));
        return Crt().wcscmp(reinterpret_cast<const u16 *>((uintptr_t)texts[column]), reinterpret_cast<const u16 *>((uintptr_t)cell)) < 0 ? 1 : 0;
    }
    if (numbers) return (s32)texts[column] > (s32)cells[column] ? 1 : 0;
    if (cells[column] == 0) return 1;
    const u32 cell = Obj_Get4Or6fd6c4(reinterpret_cast<const u8 *>((uintptr_t)cells[column]));
    return Crt().wcscmp(reinterpret_cast<const u16 *>((uintptr_t)cell), reinterpret_cast<const u16 *>((uintptr_t)texts[column])) < 0 ? 1 : 0;
}

s32 __fastcall Window7059f0_AddItem(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh) {
    auto links = [window]() { return at<s32 *>(window, 0x144); };
    s32 shown = at<s32>(window, 0x160);
    bool child = false;
    if (!texts) return -1;
    if (at<s32>(window, 0x150) * (at<s16>(window, 0x164) + 4) >= at<s32>(window, 0x158))
        Window7059f0_Grow(window, nullptr, at<s32>(window, 0x158) == 0 ? 0x14 : 0xa);
    s32 item;
    if (parent != -1 && parent < at<s32>(window, 0x150)) {
        parent = Window7059f0_NthItem(window, nullptr, parent);
        item = links()[parent + 2];
        child = true;
    } else {
        item = at<s32>(window, 0x160);
    }
    // (the first free item and the one before it, Window7059f0_RootEnd; without the array it stores neither and the original goes on
    // with its two stack slots unset, sic: 0 here, as the emulator's stack gives them, so that the read of the item's parent faults)
    s32 added = 0, last = 0;
    if (item == -1) {
        if (child) {   // (the parent's first child)
            Window7059f0_RootEnd(window, nullptr, &added, &last);
            Window7059f0_CopyCells(window, nullptr, texts, reinterpret_cast<u32 *>(links() + added + 4));
            links()[parent + 2] = added;
            links()[last] = links()[added];
            links()[added] = -1;
            links()[added + 3] = parent;
        } else {   // (no items: the first, sic)
            Window7059f0_CopyCells(window, nullptr, texts, reinterpret_cast<u32 *>(links() + 4));
            shown = 0;
            added = 0;
        }
    } else {
        Window7059f0_RootEnd(window, nullptr, &added, &last);
        Window7059f0_CopyCells(window, nullptr, texts, reinterpret_cast<u32 *>(links() + added + 4));
        if (at<u32>(window, 0x48) & 0x10) {   // (sorted: before the first sibling it sorts before; else it stays after the roots, sic)
            if (Window7059f0_SortsBefore(window, nullptr, texts, reinterpret_cast<const u32 *>(links() + item + 4))) {
                links()[last] = links()[added];
                links()[added] = item;
                if (child) {
                    links()[parent + 2] = added;
                    links()[added + 3] = parent;
                } else {
                    shown = added;
                }
            } else {
                bool done = false;
                for (s32 i = 1; !done && i < at<s32>(window, 0x150);) {
                    const s32 previous = item;
                    item = links()[item];
                    ++i;
                    if (item == -1 || Window7059f0_SortsBefore(window, nullptr, texts, reinterpret_cast<const u32 *>(links() + item + 4))) {
                        links()[last] = links()[added];
                        links()[added] = item;
                        links()[previous] = added;
                        if (child) links()[added + 3] = parent;
                        done = true;
                    }
                }
            }
        }
    }
    if (links()[added + 3] < 0) links()[added + 3] = -1;
    links()[added + 1] = (s32)data;
    at<s32>(window, 0x160) = shown;
    if (++at<s32>(window, 0x150) == 1) Window7059f0_Select(window, nullptr, 0);
    if (refresh) {
        Window7059f0_RefreshRows(window);
        if (*reinterpret_cast<const u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
    }
    const u32 first = at<u32>(window, 0x154);
    const s32 rows = at<s16>(window, 0x172), count = at<s32>(window, 0x150);
    Window7059f0_FitBar(Window_FindChild(window, nullptr, 1), count, rows, first);
    s32 end = at<s32>(window, 0x154) + at<s16>(window, 0x172);
    if (end >= at<s32>(window, 0x150)) end = at<s32>(window, 0x150) - 1;
    Window_SendMessage(at<u8 *>(window, 0x60), 0x405, at<u32>(window, 0x154), (u32)end);
    return 0;
}

u32 __fastcall Window7059f0_RemoveItem(u8 *window, void *, s32 n) {
    if (n >= at<s32>(window, 0x150) || !at<s32 *>(window, 0x144) || n < 0) return 0;
    s32 shown = at<s32>(window, 0x160);
    const s32 item = Window7059f0_NthItem(window, nullptr, n);
    s32 previous = -1;
    if (n > 0) {
        const s32 parent = at<s32 *>(window, 0x144)[item + 3];
        if (parent < 0 || at<s32 *>(window, 0x144)[parent + 2] != item)
            previous = Window7059f0_NthItem(window, nullptr, n - 1);   // (the one before in the walk, not always a sibling, sic)
    } else {
        shown = at<s32 *>(window, 0x144)[item];
    }
    Window7059f0_FreeItem(window, nullptr, (u32)n, item, previous, 0);
    at<s32>(window, 0x160) = shown;
    if (at<s32>(window, 0x14c) == n) {
        s32 row = at<s32>(window, 0x14c);
        if (at<s32>(window, 0x150) > 0) {
            if (row >= at<s32>(window, 0x150)) --row;
        } else {
            row = -1;
        }
        Window7059f0_Select(window, nullptr, row);
    }
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x00faa5d4) && n >= at<s32>(window, 0x154) &&
        n < at<s32>(window, 0x154) + at<s16>(window, 0x172))
        Window_PostMessage(window, 3, 0, 0);
    const u32 first = at<u32>(window, 0x154);
    const s32 rows = at<s16>(window, 0x172), count = at<s32>(window, 0x150);
    Window7059f0_FitBar(Window_FindChild(window, nullptr, 1), count, rows, first);
    s32 last = at<s32>(window, 0x154) + at<s16>(window, 0x172);
    if (last >= at<s32>(window, 0x150)) last = at<s32>(window, 0x150) - 1;
    Window_SendMessage(at<u8 *>(window, 0x60), 0x405, at<u32>(window, 0x154), (u32)last);
    Window7059f0_RefreshRows(window);
    return 1;
}

void __fastcall Window7059f0_Clear(u8 *window) {
    if (at<s32>(window, 0x150) == 0) return;
    Window7059f0_NthItem(window, nullptr, 0);   // (unused)
    for (s32 i = 0; i < at<s32>(window, 0x150); ++i) {
        const s32 *const cells = at<s32 *>(window, 0x144) + Window7059f0_NthItem(window, nullptr, i) + 4;
        for (s32 column = 0; column < at<s16>(window, 0x164); ++column) {
            if (Window7059f0_ColumnBottom(window, nullptr, (u32)column) != 0) continue;
            if (u8 *const object = reinterpret_cast<u8 *>((uintptr_t)(u32)cells[column])) CountedBuf_Deleter(object, nullptr, 1);
        }
    }
    Window7059f0_ResetItems(window);
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
    at<s32>(window, 0x150) = 0;
    at<s32>(window, 0x14c) = -1;
    at<s32>(window, 0x160) = -1;
    at<s32>(window, 0x154) = 0;
    const u32 first = at<u32>(window, 0x154);
    const s32 rows = at<s16>(window, 0x172), count = at<s32>(window, 0x150);
    Window7059f0_FitBar(Window_FindChild(window, nullptr, 1), count, rows, first);
    if (at<u8 *>(window, 0x174)) vcall<void>(at<u8 *>(window, 0x174), 0x14, 0u);
    Window7059f0_RefreshRows(window);
}

void __fastcall Window7059f0_ScrollTo(u8 *window, void *, s32 position) {
    if (!at<s32 *>(window, 0x144)) return;
    const s32 old = at<s32>(window, 0x154);
    if (position < 0) {
        Window7059f0_Select(window, nullptr, -1);
    } else {
        if (position >= at<s32>(window, 0x150)) position = at<s32>(window, 0x150) - 1;
        if (position < at<s32>(window, 0x154) || position >= at<s32>(window, 0x154) + at<s16>(window, 0x172)) {
            at<s32>(window, 0x154) = position;
            if (at<s32>(window, 0x154) + at<s16>(window, 0x172) > at<s32>(window, 0x150))
                at<s32>(window, 0x154) = at<s32>(window, 0x150) - at<s16>(window, 0x172);
        }
        Window7059f0_Select(window, nullptr, position - at<s32>(window, 0x154));
    }
    Window7059f0_RefreshRows(window);
    const u32 first = at<u32>(window, 0x154);
    vcall<void>(Window_FindChild(window, nullptr, 1), 0x1c, first);
    if (old == at<s32>(window, 0x154)) return;
    s32 last = at<s32>(window, 0x154) + at<s16>(window, 0x172);
    if (last >= at<s32>(window, 0x150)) last = at<s32>(window, 0x150) - 1;
    Window_SendMessage(at<u8 *>(window, 0x60), 0x405, at<u32>(window, 0x154), (u32)last);
}

void __fastcall Window7059f0_ScrollFromBar(u8 *window) {
    u8 *const bar = Window_FindChild(window, nullptr, 1);
    if (!bar) return;
    const s32 first = vcall<s32>(bar, 0x20);
    if (first == at<s32>(window, 0x154)) return;
    at<s32>(window, 0x154) = first;
    Window7059f0_RefreshRows(window);
    s32 last = at<s32>(window, 0x154) + at<s16>(window, 0x172);
    if (last >= at<s32>(window, 0x150)) last = at<s32>(window, 0x150) - 1;
    Window_SendMessage(at<u8 *>(window, 0x60), 0x405, at<u32>(window, 0x154), (u32)last);
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}

u8 *__fastcall Window7059f0_Cell(u8 *window, void *, s32 row, u32 column) {
    s16 col = (s16)column;
    if (col >= at<s16>(window, 0x164) || row >= at<s16>(window, 0x172)) return nullptr;
    u8 **const slot = reinterpret_cast<u8 **>(at<uintptr_t>(window, 0x148) + (uintptr_t)row * 4);
    if (!slot) return nullptr;
    if (col == 0) return *slot;
    if (!*slot) return nullptr;
    u8 *cell = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(*slot, nullptr, 0));
    for (; col != 0; --col) {
        if (cell) cell = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(cell));
    }
    return cell;
}

void __fastcall Window7059f0_PostColumnAt(u8 *window, void *, u32 x, u32) {
    if (at<s32>(window, 0x14c) < 0) return;
    for (s32 column = 0; column < at<s16>(window, 0x164); ++column) {
        const u8 *const col = at<u8 *>(window, 0x16c) + column * 0xc;
        if ((s16)x > at<s16>(const_cast<u8 *>(col), 4) && (s16)x < at<s16>(const_cast<u8 *>(col), 8)) {
            Window_PostMessage(at<u8 *>(window, 0x60), 0x404, at<u32>(window, 0x8c), (u32)column);
            return;
        }
    }
}

u16 __fastcall Window7059f0_NthItemCell(u8 *window, void *, s32 n, s32 column, u32 *out) {
    const s32 item = Window7059f0_NthItem(window, nullptr, n);
    if (item < 0 || column >= at<s16>(window, 0x164)) return 0;
    const u32 *const cell = &at<u32 *>(window, 0x144)[item + 4 + column];
    if (Window7059f0_ColumnBottom(window, nullptr, (u32)column) == 0) *out = Obj_Get4Or6fd6c4(reinterpret_cast<const u8 *>((uintptr_t)*cell));
    else *out = *cell;
    return 1;
}

u32 __fastcall Window7059f0_NthItemData(u8 *window, void *, s32 n, u32 *out) {
    const s32 item = Window7059f0_NthItem(window, nullptr, n);
    if (item == -1) return 0xffff0000u;
    if (!out) return ((u32)item & 0xffff0000u) | 1;
    *out = at<u32 *>(window, 0x144)[item + 1];
    return ((u32)(uintptr_t)out & 0xffff0000u) | 1;
}

void __fastcall Window7059f0_OnKey(u8 *window, void *, u32 key, u32) {
    if (!(at<u32>(window, 0x48) & 0x200)) return;
    switch ((s16)key) {
    case 0x2400: Window7059f0_ScrollTo(window, nullptr, 0); break;
    case 0x2300: Window7059f0_ScrollTo(window, nullptr, at<s32>(window, 0x150) - 1); break;
    case 0x2600: Window7059f0_ScrollTo(window, nullptr, at<s32>(window, 0x14c) - 1); break;
    case 0x2800: Window7059f0_ScrollTo(window, nullptr, at<s32>(window, 0x14c) + 1); break;
    }
}

u32 __fastcall Window7059f0_PostSelection(u8 *window) {
    const s32 selection = at<s32>(window, 0x14c);
    if (selection < 0) return (u32)(uintptr_t)window;   // (EAX still the window the compare went through)
    return Window_PostMessage(at<u8 *>(window, 0x60), 0x400, at<u32>(window, 0x8c), (u32)selection);
}

s32 __fastcall Window7059f0_NotifySort(u8 *window) {
    s32 value = at<s16>(window, 0x168) + 1;
    if (at<s16>(window, 0x16a) == 1) value = -value;
    return Window_SendMessage(at<u8 *>(window, 0x60), 0x406, at<u32>(window, 0x8c), (u32)value);
}

u32 __fastcall Window7059f0_ColumnSpan(u8 *window, void *, s32 column, u16 *first, u16 *second) {
    if (!at<u8 *>(window, 0x16c) || !first || !second || column >= at<s16>(window, 0x164)) return (u32)(uintptr_t)window & 0xffff0000u;
    *first = at<u16>(at<u8 *>(window, 0x16c) + column * 12, 4);
    *second = at<u16>(at<u8 *>(window, 0x16c) + column * 12, 8);
    return ((u32)column * 12 & 0xffff0000u) | 1;
}

u32 __fastcall Window7059f0_SetSortColumn(u8 *window, void *, u32 column) {
    if ((s16)column == at<s16>(window, 0x168)) return (u32)(s32)(s16)column & 0xffff0000u;
    at<s16>(window, 0x168) = (s16)column;
    return ((u32)Window7059f0_NotifySort(window) & 0xffff0000u) | 1;
}

u32 __fastcall Window7059f0_SetSortOrder(u8 *window, void *, u32 order) {
    if ((s16)order == at<s16>(window, 0x16a)) return (u32)(s32)(s16)order & 0xffff0000u;
    at<s16>(window, 0x16a) = (s16)order == -1 ? (s16)(at<s16>(window, 0x16a) == 0) : (s16)order;
    return ((u32)Window7059f0_NotifySort(window) & 0xffff0000u) | 1;
}

void __fastcall Window7059f0_SortBy(u8 *window, void *, s32 by, u32 sort) {
    const u32 magnitude = by < 0 ? 0u - (u32)by : (u32)by;   // (CRT abs 0x680050)
    const bool column = (s16)Window7059f0_SetSortColumn(window, nullptr, magnitude - 1) == 1;
    const bool order = (s16)Window7059f0_SetSortOrder(window, nullptr, by < 0 ? 1u : 0u) == 1;
    if ((order || column) && (u8)sort) Window7059f0_Sort(window, nullptr, -1);
}

void __fastcall Window7059f0_OnHeader(u8 *window, void *, u32 id) {
    const s16 column = (s16)(id - 0x10);
    if (!(at<u32>(window, 0x48) & 0x10) || column < 0 || column >= at<s16>(window, 0x164)) return;
    if (column == at<s16>(window, 0x168)) {
        Window7059f0_SetSortOrder(window, nullptr, 0xffffffffu);
    } else {
        Window7059f0_SetSortOrder(window, nullptr, 0);
        Window7059f0_SetSortColumn(window, nullptr, (u16)column);
    }
    Window7059f0_Sort(window, nullptr, -1);
    Window7059f0_RefreshRows(window);
    if (at<s32>(window, 0x14c) >= 0) Window_PostMessage(at<u8 *>(window, 0x60), 0x401, at<u32>(window, 0x8c), at<u32>(window, 0x14c));
}

void __fastcall Window7059f0_OnMouseDown(u8 *window, void *, u32 x, u32 y) {
    Window7059f0_SelectAt(window, nullptr, x & 0xffff, y & 0xffff);
    Window7059f0_PostColumnAt(window, nullptr, x & 0xffff, y & 0xffff);
    if ((at<u32>(window, 0x48) & 0x80) && at<s32>(window, 0x14c) >= 0)
        Window_PostMessage(at<u8 *>(window, 0x60), 0x400, at<u32>(window, 0x8c), at<u32>(window, 0x14c));
}

void __fastcall Window7059f0_OnDouble(u8 *window, void *, u32 x, u32 y) {
    if (!(at<u32>(window, 0x48) & 0x80)) return;
    Window7059f0_SelectAt(window, nullptr, x & 0xffff, y & 0xffff);
    if (at<s32>(window, 0x14c) >= 0) Window_PostMessage(at<u8 *>(window, 0x60), 0x402, at<u32>(window, 0x8c), at<u32>(window, 0x14c));
}

void __fastcall Window7059f0_OnDrag(u8 *window, void *, u32 x, u32 y) {
    if (!(at<u32>(window, 0x48) & 0x80) || at<u32>(window, 0x11c) != 0) return;
    if ((s16)vcall<u32>(window + 0x2c, 0, x & 0xffff, y & 0xffff)) Window7059f0_SelectAt(window, nullptr, x & 0xffff, y & 0xffff);
}

void __fastcall Window7059f0_OnMoved(u8 *window, void *, u32 dx, u32 dy) {
    vcall<void>(window + 0x138, 8, dx & 0xffff, dy & 0xffff);   // (the inner rectangle's slot 2: moved)
    Window7059f0_OffsetColumns(window, nullptr, dx & 0xffff, dy & 0xffff);
}

u8 *__fastcall Window7059f0_DeletingDtor(u8 *window, void *, u32 flags) {
    Window7059f0_Dtor(window);
    if (flags & 1) GameHeap_Free(window);
    return window;
}
}
