#include "window.h"

#include <cstdint>

#include "../core/accessors.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

u8 *__fastcall Window_FindChild(u8 *window, void *, u32 id) {
    u8 *found = nullptr;
    for (u8 *child = at<u8 *>(window, 0x5c); child && !found; child = at<u8 *>(child, 0x54))
        if (Fn_65d7bc(child) == id) found = child;
    return found;
}
}
