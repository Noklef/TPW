#include "guardescort.h"

#include <cstdint>

#include "../core/accessors.h"
#include "analyser.h"
#include "mapcell.h"
#include "person.h"
#include "staff.h"
#include "visitorsteps.h"
#include "world.h"
#include "../core/soundset.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const GuardEscortEnv kGameEnv = {&Staff_IsTooTired, &Staff_TireB, &Sprite_WalkNav, &Sprite_SetDestinationCell, &Visitor_IsState6To10, &ParkAnalyser_Tally, &Sprite_PlaySound, &Visitor_KillScripts, &Map_TicketBoothPos, &Map_EntrancePos, &Guard_SetState, &Visitor_Arrive};
const GuardEscortEnv *g_env = &kGameEnv;

u8 *thingOf(u32 handle) { return global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20); }
u32 cellOf(const u8 *sprite) { return ((u32)sprite[7] << 7) + sprite[5] + 1; }
u8 *record() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x0080239c))); }
u32 parkSide() { return at<u32>(global<u8 *>(0x0080239c), 0x1da70c) & 1; }

void givenUp(u8 *guard) {
    g_env->tally(record(), nullptr, 0);
    at<u16>(guard, 0xc2) = global<u16>((u32)(uintptr_t)&gStaff_75c7f0);
    g_env->setState(guard, nullptr, 0);
    at<u16>(guard, 0x212) = 0;
}

u32 toBooth(u8 *guard) {
    u16 out;
    return g_env->setDestination(guard, nullptr, *g_env->boothPos(global<u8 *>(0x008023a0), nullptr, &out, parkSide()));
}
}  // namespace

void GuardEscort_SetEnv(const GuardEscortEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Guard_Send(u8 *guard, void *, u32 handle) {
    if (g_env->tooTired(guard)) return;
    g_env->playSound(guard, nullptr, gSoundCategories.staff, 0x88);
    at<u16>(guard, 0x212) = (u16)handle;
    if (g_env->setDestination(guard, nullptr, cellOf(thingOf(handle)))) g_env->setState(guard, nullptr, 0x10);
}

void __fastcall Guard_Chase(u8 *guard) {
    g_env->tire(guard);
    const u16 handle = at<u16>(guard, 0x212);
    if (!handle || !at<u32>(guard, 0x214)) {
        givenUp(guard);
        return;
    }
    --at<u32>(guard, 0x214);
    u8 *const visitor = thingOf(handle);
    const u32 step = g_env->walkNav(guard);
    const bool here = cellOf(guard) == cellOf(visitor);
    if (step == 0 && !here) {
        if (!g_env->setDestination(guard, nullptr, cellOf(visitor))) givenUp(guard);
        return;
    }
    if (step == 2) {
        givenUp(guard);
        return;
    }
    if (!here) return;
    u8 *const again = thingOf(at<u16>(guard, 0x212));
    if (!g_env->isCaught(again)) {
        givenUp(guard);
        return;
    }
    at<u16>(guard, 0xc2) = global<u16>((u32)(uintptr_t)&gStaff_75c7f0);
    Set4_4217f0(guard + 0xc, nullptr, 0x16);
    g_env->tally(record(), nullptr, 2);
    g_env->playSound(guard, nullptr, gSoundCategories.staff, 0x89);
    g_env->killScripts(visitor);
    if (toBooth(guard)) {
        g_env->setState(guard, nullptr, 0x12);
        return;
    }
    at<u32>(guard, 0x188) = 1;
    if (toBooth(guard)) {
        g_env->setState(guard, nullptr, 0x12);
        return;
    }
    g_env->arrive(visitor, nullptr, cellOf(guard));
    givenUp(guard);
}

void __fastcall Guard_LeaveWithVisitor(u8 *guard) {
    const u32 step = g_env->walkNav(guard);
    if (step != 0 && step != 2) return;
    Set4_4217f0(guard + 0xc, nullptr, 0x18);
    g_env->arrive(thingOf(at<u16>(guard, 0x212)), nullptr, cellOf(guard));
    at<u32>(guard, 0x188) = 0;
    u16 out;
    const u32 found = g_env->setDestination(guard, nullptr, *g_env->entrancePos(global<u8 *>(0x008023a0), nullptr, &out, parkSide()));
    g_env->setState(guard, nullptr, found ? 0x13 : 0);
}
}
