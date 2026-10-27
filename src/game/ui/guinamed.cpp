#include "guinamed.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *&head() { return *reinterpret_cast<u8 **>((uintptr_t)0x00faa624); }

}  // namespace

extern "C" {

u16 __cdecl GuiNamed_Add(u8 *object, const char *name) {
    if (!object || !std::strlen(name)) return 0;
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(0x14));
    if (!node) return 0;
    at<u8 *>(node, 0x10) = nullptr;
    at<u8 *>(node, 0) = object;
    std::strncpy(reinterpret_cast<char *>(node + 4), name, 0xc);
    if (!head()) {
        head() = node;
    } else {
        u8 *last = head();
        while (at<u8 *>(last, 0x10)) last = at<u8 *>(last, 0x10);
        at<u8 *>(last, 0x10) = node;
    }
    return 1;
}

void __cdecl GuiNamed_FreeAll() {
    for (u8 *node = head(); node;) {
        u8 *const next = at<u8 *>(node, 0x10);
        if (u8 *const object = at<u8 *>(node, 0)) vcall<void>(object, 0, 1u);
        GameHeap_Free(node);
        node = next;
    }
}

u8 *__cdecl GuiNamed_Find(const char *name) {
    for (u8 *node = head(); node; node = at<u8 *>(node, 0x10))
        if (!std::strcmp(reinterpret_cast<const char *>(node + 4), name)) return at<u8 *>(node, 0);
    return nullptr;
}
}  // extern "C"
