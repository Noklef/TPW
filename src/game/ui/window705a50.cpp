#include "window705a50.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "textpool.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

u16 __fastcall Window705a50_TextLength(const u8 *window) {
    const u16 *const text = *reinterpret_cast<const u16 *const *>(window + 0x138);
    return text ? (u16)Crt().wcslen(text) : 0;
}

u32 __fastcall Window705a50_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705a50;
    if (at<u8 *>(window, 0x148)) TextPool_Release(at<u8 *>(window, 0x148));
    return Window_Dtor(window);
}

u8 *__fastcall Window705a50_DeletingDtor(u8 *window, void *, u32 flags) {
    Window705a50_Dtor(window);
    if (flags & 1) GameHeap_Free(window);
    return window;
}
}
