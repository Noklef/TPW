#include "keyactions.h"

#include <cstdint>

#include "../world/money.h"
#include "../world/world.h"
#include "actionrec.h"
#include "gameclock.h"
#include "globals.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *gameClock() { return reinterpret_cast<u8 *>((uintptr_t)0x00785970); }

u32 turn(u32 by) {
    Global81d7a4_SetRecorded(static_cast<u32>(static_cast<s32>(Global81d7a4_Get_52f1f0() + by) % 360), 1);
    return 1;
}

}  // namespace

extern "C" {

u32 __cdecl KeyAction_Turn90() { return turn(0x5a); }
u32 __cdecl KeyAction_Turn270() { return turn(0x10e); }

u32 __cdecl KeyAction_ClockSlower() {
    GameClock_Slower(gameClock());
    return 1;
}

u32 __cdecl KeyAction_ClockFaster() {
    GameClock_Faster(gameClock());
    return 1;
}

u32 __cdecl KeyAction_ClockNormal() {
    GameClock_SetRate(gameClock(), nullptr, 0, 0x3ff00000);
    return 1;
}

u32 __cdecl KeyAction_Credit10000() {
    Money_Credit(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c))), nullptr, 10000);
    return 1;
}

u32 __cdecl KeyAction_Toggle787d1c() {
    const u32 v = 1 - global<u32>(0x00787d1c);
    global<u32>(0x00787d1c) = v;
    Global878128_Set_550510(v);
    return 1;
}

u32 __cdecl KeyAction_ViewFillStep() {
    u32 &flags = global<u32>(0x008bcbc8);
    switch (flags & 0xc) {
    case 4: flags &= ~0xcu; break;
    case 0xc: flags = (flags & ~8u) | 4; break;
    default: flags |= 0xc; break;
    }
    return 1;
}
}
