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

s32 __fastcall Window_CallHandler(u8 *window, void *, u32 message, u32 a, u32 b) {
    using Handler = s32(__cdecl *)(u8 *, u32, u32, u32);
    const Handler handler = at<Handler>(window, 0x114);
    return handler ? handler(window, message, a, b) : -1;
}

s32 __cdecl Window_SendMessage(u8 *window, u32 message, u32 a, u32 b) {
    using Hook = void(__cdecl *)(u8 *, u32, u32, u32);
    if (!window || Fn_659710(window)) return -1;
    if (const Hook hook = *reinterpret_cast<Hook *>((uintptr_t)0x00faa5fc)) hook(window, message, a, b);
    return Window_CallHandler(window, nullptr, message, a, b);
}
}
