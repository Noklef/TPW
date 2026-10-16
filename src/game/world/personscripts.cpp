#include "personscripts.h"

#include <cstdint>

#include "../sprite/script.h"
#include "person.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Visitor_PreviousRideTick(u8 *visitor) {
    const u8 *world = *reinterpret_cast<u8 *const *>(0x0080239c);
    if (at<u32>(world, 0x1da70c) % 20 == 0)
        Visitor_PushPreviousRide(visitor, nullptr, (u32)(uintptr_t)visitor & 0xffff0000);
}

void __fastcall Person_JumpScript210(u8 *person) {
    if (at<u32>(person, 0x210)) {
        SpriteScript_Jump((s32)at<u32>(person, 0x210), 0x0074f4c0);
        at<u32>(person, 0x210) = 0;
    }
}

void __fastcall Person_KillScript84(u8 *person) {
    if (at<u32>(person, 0x84)) {
        SpriteScript_Kill((s32)at<u32>(person, 0x84));
        at<u32>(person, 0x84) = 0;
    }
}
}
