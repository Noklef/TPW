#include "small60.h"

#include <cstdint>

#include "../world/objectmsg.h"
#include "small58.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __cdecl Animator_Sleep(u8 *animator) {
    Holder_Sleep(at<u8 *>(animator, 0x24));
    at<u32>(animator, 0) = at<u32>(animator, 0) & 0xffffbfff;
    return 1;
}

u16 *__fastcall Object_BackOfQueueTo(u8 *object, void *, u16 *out) {
    Object_BackOfQueue(object, nullptr, out);
    return out;
}

s32 __fastcall Object_QueuePositionOfNone(u8 *object) {
    return Object_QueuePosition(object, nullptr, (u32)(uintptr_t)object & 0xffff0000u);
}

}  // extern "C"
