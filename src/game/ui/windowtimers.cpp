#include "windowtimers.h"

#include <cstdint>

#include "../core/accessors.h"
#include "guimanager.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *&timers() { return *reinterpret_cast<u8 **>((uintptr_t)0x00faa634); }
}

extern "C" {

u8 *__fastcall WindowTimer_Unlink(u8 *node) {
    if (at<u8 *>(node, 0x1c)) at<u8 *>(at<u8 *>(node, 0x1c), 0x18) = at<u8 *>(node, 0x18);
    if (!at<u8 *>(node, 0x18)) return node;
    at<u8 *>(at<u8 *>(node, 0x18), 0x1c) = at<u8 *>(node, 0x1c);
    return at<u8 *>(node, 0x1c);
}

u8 *__fastcall WindowTimer_Find(u8 *node, void *, u32 window, u32 id) {
    u8 *found = nullptr;
    if (at<u8 *>(node, 0x18)) found = WindowTimer_Find(at<u8 *>(node, 0x18), nullptr, window, id);
    if (found) return found;
    return id == at<u32>(node, 0xc) && window == at<u32>(node, 0) ? node : nullptr;
}

u8 *__fastcall WindowTimer_Delete(u8 *node, void *, u32 flags) {
    WindowTimer_Unlink(node);
    if (flags & 1) Fn_661f9b((u32)(uintptr_t)node);
    return node;
}

u32 __cdecl WindowTimers_Kill(u32 window, u32 id) {
    if (!timers()) return 0;
    u8 *const node = WindowTimer_Find(timers(), nullptr, window, id);
    if (!node) return 0;
    if (node == timers()) timers() = at<u8 *>(node, 0x18);
    WindowTimer_Delete(node, nullptr, 1);
    return 1;
}

u8 *__cdecl WindowTimer_New(u32) {
    u8 *found = nullptr;
    for (s32 i = 0; i < *reinterpret_cast<s32 *>((uintptr_t)0x00faa62c) && !found; ++i) {
        u8 *const node = *reinterpret_cast<u8 **>((uintptr_t)0x00faa630) + 0x20 * i;
        if (at<u32>(node, 0x14)) {
            found = node;
            at<u32>(node, 0x14) = 0;
        }
    }
    return found;
}

u8 *__fastcall WindowTimer_Ctor(u8 *node, void *, u32 window, u32 interval, u32 id) {
    at<u32>(node, 0) = window;
    at<u32>(node, 8) = interval;
    at<u32>(node, 0xc) = id;
    at<u32>(node, 0x18) = 0;
    at<u32>(node, 0x1c) = 0;
    at<u32>(node, 4) = GuiClock_Now();
    at<u32>(node, 0x10) = 0;
    at<u32>(node, 0x14) = 0;
    return node;
}

u8 *__cdecl WindowTimers_Add(u32 window, u32 interval, u32 id) {
    u8 *node = WindowTimer_New(0x20);
    if (node) node = WindowTimer_Ctor(node, nullptr, window, interval, id);
    if (!node) return nullptr;
    if (!timers()) {
        timers() = node;
    } else {
        u8 *last = timers();
        while (at<u8 *>(last, 0x18)) last = at<u8 *>(last, 0x18);
        at<u8 *>(last, 0x18) = node;
        at<u8 *>(node, 0x1c) = last;
    }
    return node;
}

u32 __fastcall WindowTimer_Start(u8 *node) {
    at<u32>(node, 0x10) = 1;
    const u32 now = GuiClock_Now();
    at<u32>(node, 4) = now;
    return now;
}

u32 __fastcall Window_SetTimer(u8 *window, void *, u32 id, u32 interval) {
    if ((id & 0x80080) == 0x80080) id -= 0x80080;
    u8 *&slot = at<u8 *>(window, 0x120 + 4 * id);
    if (slot) {
        Fn_6620cc(slot, nullptr, interval);
        return 1;
    }
    slot = WindowTimers_Add((u32)(uintptr_t)window, interval, id + 0x80080);
    return slot ? 1 : 0;
}

u32 __fastcall Window_StartTimer(u8 *window, void *, u32 id) {
    if ((id & 0x80080) == 0x80080) id -= 0x80080;
    u8 *const node = at<u8 *>(window, 0x120 + 4 * id);
    if (!node) return 0;
    WindowTimer_Start(node);
    return 1;
}

u32 __fastcall Window_StopTimer(u8 *window, void *, u32 id) {
    if ((id & 0x80080) == 0x80080) id -= 0x80080;
    u8 *const node = at<u8 *>(window, 0x120 + 4 * id);
    if (!node) return 0;
    Fn_662097(node);
    return 1;
}
}
