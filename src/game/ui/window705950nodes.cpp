// The nodes of the GUI library's list window: see window705950nodes.h.
#include "window705950nodes.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "window.h"
#include "window705950.h"
#include "window705b20.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u32 kNodeSize = 0x54, kTextUnits = 0x20;

s32 &count(u8 *window) { return at<s32>(window, 0x14c); }
u8 *&firstNode(u8 *window) { return at<u8 *>(window, 0x144); }
s32 rowsShown(u8 *window) { return at<s16>(window, 0x156); }
void redraw(u8 *window) { Window_PostMessage(window, 3, 0, 0); }
bool drawing() { return *reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4) != 0; }

u8 *newNode(const u16 *text, u32 value, u8 *node) {
    at<u32>(node, 0x40) = value;
    Crt().wcsncpy(reinterpret_cast<u16 *>(node), text, kTextUnits);
    at<u32>(node, 0x44) = 0;
    at<u32>(node, 0x48) = 0;
    at<u32>(node, 0x4c) = 0;
    at<u32>(node, 0x50) = 0;
    return node;
}

// The end of both inserts: the bar shown over more rows than fit, the window redrawn.
void inserted(u8 *window) {
    if (count(window) > rowsShown(window)) {
        if (u8 *const bar = Window_FindChild(window, nullptr, 1)) {
            vcall<void>(bar, 0x14, 1);
            Window705b20_SetRange(bar, nullptr, count(window) - rowsShown(window), 0);
            Fn_66b53e(bar, nullptr, (u32)rowsShown(window));
        }
    }
    if (drawing()) redraw(window);
}

bool comesBefore(u8 *node, u8 *other, bool descending) {
    const s32 order = Crt().wcscmp(reinterpret_cast<const u16 *>(node), reinterpret_cast<const u16 *>(other));
    return descending ? order >= 0 : order < 0;
}

// Flags 0x10 and 0x20: in order among the row `current` and its next ones. The answer counts the steps.
u16 insertOrdered(u8 *window, s32 n, u8 *parent, u8 *current, u8 *node, bool descending) {
    u16 steps = 0;
    if (!current || comesBefore(node, current, descending)) {
        at<u8 *>(node, 0x44) = current;
        if (current) at<u8 *>(current, 0x50) = node;
        if (n == -1) firstNode(window) = node;
        else at<u8 *>(parent, 0x48) = node;
        return steps;
    }
    for (;;) {
        u8 *const next = at<u8 *>(current, 0x44);
        ++steps;
        if (!next || comesBefore(node, next, descending)) {
            at<u8 *>(node, 0x44) = next;
            at<u8 *>(current, 0x44) = node;
            at<u8 *>(node, 0x50) = current;
            if (next) at<u8 *>(next, 0x50) = node;
            return steps;
        }
        current = next;
    }
}

}  // namespace

extern "C" {

u16 __fastcall Window705950_InsertSorted(u8 *window, void *, s32 n, const u16 *text, u32 value) {
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(kNodeSize));
    if (!node || n >= count(window)) return 0xffff;
    u16 result = 0;
    ++count(window);
    u8 *current = firstNode(window);
    newNode(text, value, node);
    if (!firstNode(window)) {
        firstNode(window) = node;
    } else {
        u8 *parent = nullptr;
        if (n > -1) {
            parent = Window705950_NthNode(window, nullptr, n);
            current = at<u8 *>(parent, 0x48);
            at<u8 *>(node, 0x4c) = parent;
        }
        const u32 flags = at<u32>(window, 0x48);
        if (flags & 0x10) {
            result = insertOrdered(window, n, parent, current, node, false);
        } else if (flags & 0x20) {
            result = insertOrdered(window, n, parent, current, node, true);
        } else {
            result = 1;
            while (at<u8 *>(current, 0x44)) {
                current = at<u8 *>(current, 0x44);
                ++result;
            }
            at<u8 *>(current, 0x44) = node;
            at<u8 *>(node, 0x50) = current;
        }
    }
    inserted(window);
    return result;
}

u16 __fastcall Window705950_InsertAt(u8 *window, void *, s32 n, s32 position, const u16 *text, u32 value) {
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(kNodeSize));
    if (!node) return 0xffff;
    if (position < 0) position = 0;
    else if (position >= count(window)) position = count(window) - 1;
    if (n >= count(window)) n = count(window) - 1;
    ++count(window);
    newNode(text, value, node);
    u8 *current = firstNode(window);
    u8 *parent = nullptr;
    if (n > -1 && n < count(window)) {
        parent = Window705950_NthNode(window, nullptr, n);
        current = at<u8 *>(parent, 0x48);
        at<u8 *>(node, 0x4c) = parent;
    }
    u16 result = 1;
    if (!firstNode(window)) {
        firstNode(window) = node;
    } else if (position == 0 || !current) {
        at<u8 *>(node, 0x44) = current;
        if (n < 0) firstNode(window) = node;
        if (!current) at<u8 *>(parent, 0x48) = node;
        else if (at<u8 *>(current, 0x4c)) at<u8 *>(at<u8 *>(current, 0x4c), 0x48) = node;
    } else {
        for (--position; position != 0 && at<u8 *>(current, 0x44); --position) {
            current = at<u8 *>(current, 0x44);
            ++result;
        }
        if (at<u8 *>(current, 0x44)) at<u8 *>(at<u8 *>(current, 0x44), 0x50) = node;
        at<u8 *>(node, 0x44) = at<u8 *>(current, 0x44);
        at<u8 *>(current, 0x44) = node;
    }
    inserted(window);
    return result;
}

s32 __cdecl Window705950_FreeNodes(u8 *node) {
    s32 freed = 0;
    while (node) {
        if (at<u8 *>(node, 0x48)) freed += Window705950_FreeNodes(at<u8 *>(node, 0x48));
        u8 *const next = at<u8 *>(node, 0x44);
        GameHeap_Free(node);
        node = next;
        ++freed;
    }
    return freed;
}

u16 __fastcall Window705950_RemoveNode(u8 *window, void *, s32 n) {
    if (n >= count(window) || !firstNode(window) || n < 0) return 0;
    u8 *node;
    if (n == 0) {
        node = firstNode(window);
        firstNode(window) = at<u8 *>(node, 0x44);
    } else {
        node = Window705950_NthNode(window, nullptr, n);
        if (at<u8 *>(node, 0x50)) at<u8 *>(at<u8 *>(node, 0x50), 0x44) = at<u8 *>(node, 0x44);
        else if (at<u8 *>(node, 0x4c)) at<u8 *>(at<u8 *>(node, 0x4c), 0x48) = at<u8 *>(node, 0x44);
    }
    if (at<u8 *>(node, 0x44)) at<u8 *>(at<u8 *>(node, 0x44), 0x50) = at<u8 *>(node, 0x50);
    s32 removed = Window705950_FreeNodes(at<u8 *>(node, 0x48));
    GameHeap_Free(node);
    ++removed;
    if (at<s32>(window, 0x148) == n) at<s32>(window, 0x148) = -1;
    count(window) -= removed;
    if (drawing() && n >= at<s32>(window, 0x150) && n < (s32)(at<u32>(window, 0x150) + (u32)rowsShown(window))) redraw(window);
    if (u8 *const bar = Window_FindChild(window, nullptr, 1)) {
        if (count(window) > rowsShown(window)) {
            vcall<void>(bar, 0x14, 1);
            Window705b20_SetRange(bar, nullptr, count(window) - rowsShown(window), 0);
        } else {
            vcall<void>(bar, 0x14, 0);
            Window705b20_SetRange(bar, nullptr, 0, 0);
        }
    }
    return 1;
}

void __fastcall Window705950_Clear(u8 *window) {
    Window705950_FreeNodes(firstNode(window));
    count(window) = 0;
    at<s32>(window, 0x148) = -1;
    firstNode(window) = nullptr;
    if (drawing()) redraw(window);
    if (u8 *const bar = Window_FindChild(window, nullptr, 1)) {
        vcall<void>(bar, 0x14, 0);
        Window705b20_SetRange(bar, nullptr, 0, 0);
    }
}

u16 __fastcall Window705950_SetNodeText(u8 *window, void *, s32 n, const u16 *text) {
    u8 *const node = Window705950_NthNode(window, nullptr, n);
    if (!node) return 0;
    Crt().wcsncpy(reinterpret_cast<u16 *>(node), text, kTextUnits);
    return 1;
}

u16 __fastcall Window705950_GetNodeText(u8 *window, void *, s32 n, u16 *out) {
    u8 *const node = Window705950_NthNode(window, nullptr, n);
    if (!node) return 0;
    Crt().wcscpy(out, reinterpret_cast<const u16 *>(node));
    return 1;
}
}
