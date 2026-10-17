#include "lobbyscene.h"

#include <cstdint>

#include "../model/holder.h"
#include "lobbymodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Class702af0_AcquireModels(u8 *object) {
    if (at<u32>(object, 4) != 0)
        return;
    for (u8 *node = at<u8 *>(at<u8 *>(object, 0x58), 4); node;) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (at<u32>(item, 0xc) == 0) {
            RideUser_Acquire(at<u8 *>(item, 8));
            at<u32>(item, 0xc) = 1;
        }
    }
    at<u32>(object, 4) = 1;
}

void __fastcall Class702ca8_AcquireModels(u8 *object) {
    if (at<u32>(object, 0x10) != 0)
        return;
    RideUser_Acquire(at<u8 *>(object, 4));
    RideUser_Acquire(at<u8 *>(object, 8));
    at<u32>(object, 0x10) = 1;
}

u32 __fastcall Class702cb0_StartPendingPlay(u8 *object) {
    const u32 state = at<u32>(object, 0x14);
    if (state == 0)
        return 0;
    if (state == 2)
        LobbyModel_Play(at<u8 *>(at<u8 *>(object, 0xc), 8), nullptr, 1, 0);
    at<u32>(object, 0x14) = 0;
    return 1;
}
}
