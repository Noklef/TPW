#include "window7059f0.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
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
}
