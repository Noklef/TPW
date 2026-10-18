#include "windowtimers.h"

#include <cstdint>

#include "../core/accessors.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
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
}
