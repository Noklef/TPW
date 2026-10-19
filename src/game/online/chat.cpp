#include "chat.h"

#include <cstdint>

#include "../core/sync.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall OnlineChat_Set29And2a(u8 *chat) {
    CritSec_Enter(chat + 0x48);
    at<u8>(chat, 0x29) = 1;
    at<u8>(chat, 0x2a) = 1;
    CritSec_Leave(chat + 0x48);
}

u8 __fastcall OnlineChat_Get2a(u8 *chat) {
    CritSec_Enter(chat + 0x48);
    const u8 value = at<u8>(chat, 0x2a);
    CritSec_Leave(chat + 0x48);
    return value;
}
}
