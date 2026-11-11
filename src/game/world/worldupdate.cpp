// The world's tick and a thing's: see worldupdate.h.
#include "worldupdate.h"

#include <cstdint>

#include "ridelist.h"
#include "things.h"
#include "../ui/dialog74fb20.h"
#include "worldstart.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *address(u32 a) { return reinterpret_cast<u8 *>((uintptr_t)a); }

using Method = WorldUpdateEnv::Method;
Method method(u32 a) { return reinterpret_cast<Method>((uintptr_t)a); }

const WorldUpdateEnv kGameEnv = {
    {method(0x00501650), method(0x005019f0)},
    {method(0x004e0b90), method(0x004e0e00)},
    {method(0x004da5a0), method(0x004da490)},
    {method(0x004d7060), method(0x004d73c0)},
    {method(0x004d4660), method(0x004d4810)},
    {method(0x004d6360), method(0x004d6410)},
    {method(0x00502960), method(0x005029f0)},
    {method(0x004f7c60), method(0x004f7c90)},
    method(0x0059a550),
    method(0x00512880),
    method(0x004d1e90),
    method(0x00550ca0),
    reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)0x0067a7d0),
    Rides_FindById,
    Rides_GetElement,
    Rides_SetElement,
    reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)0x004c9130),
    reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)0x004fa990),
    method(0x004d7b20),
    method(0x004041d0),
};
const WorldUpdateEnv *g_env = &kGameEnv;

// The setting, read once under its guard (each caller's own).
u32 setting(u32 guard_at) {
    u8 &guard = global<u8>(guard_at);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->settingReadMode(address(reinterpret_cast<uintptr_t>(&gDialog74fb20_fb3b7c)));
        g_env->atexit(address(0x005d1750));
    }
    return gDialog74fb20_fb3b7c;
}

ThingSlot &slot_of(u16 handle) { return reinterpret_cast<ThingSlot *>((uintptr_t)0x007cfb90)[handle]; }
u16 handle_of_slot(const ThingSlot *slot) { return slot ? static_cast<u16>(slot->id) : 0; }
u8 *thing_of(u16 handle) { return static_cast<u8 *>(slot_of(handle).thing); }
u16 first_handle() { return handle_of_slot(global<const ThingSlot *>(0x007cf56c)); }

u16 next_handle(const u8 *thing) {
    u16 handle;
    return handle_of_slot(slot_of(*Thing_GetHandle(thing, nullptr, &handle)).next);
}

void both(const Method *calls, u8 *thing) {
    calls[0](thing);
    calls[1](thing);
}

bool some_staff_off_plain_cell() {
    for (u16 handle = first_handle(); handle; ) {
        u8 *const thing = thing_of(handle);
        if (!thing[3] && thing[2] >= 4 && thing[2] <= 8 && g_env->typePlain(thing) == 0) return true;
        handle = next_handle(thing);
    }
    return false;
}

void check_gates(u8 *world) {
    const u16 gates = at<u16>(world, 0x1da732);
    if (!gates || at<u32>(world, 0x1da70c) % 30 != 0 || !at<u32>(world, 0x1da710)) return;
    if (g_env->getElement(g_env->findRide(at<s32>(thing_of(gates), 0x24)), 1) != 1) return;
    if (g_env->countVisitors(thing_of(at<u16>(world, 0x1da720))) != 0 || some_staff_off_plain_cell()) return;
    const u8 *const thing = thing_of(at<u16>(world, 0x1da732));
    g_env->getElement(g_env->findRide(at<s32>(thing, 0x24)), 1);
    g_env->setElement(g_env->findRide(at<s32>(thing, 0x24)), 0, 0);
}

}  // namespace

void WorldUpdate_SetEnv(const WorldUpdateEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Thing_Update(u8 *thing) {
    const WorldUpdateEnv *const e = g_env;
    switch (thing[2]) {
    case 1: both(e->visitor, thing); break;
    case 3: both(e->object, thing); break;
    case 4: both(e->mechanic, thing); break;
    case 5: both(e->handyman, thing); break;
    case 6: both(e->entertainer, thing); break;
    case 7: both(e->guard, thing); break;
    case 8: both(e->researcher, thing); break;
    case 11: if (setting((u32)(uintptr_t)&gWorldUpdate_fb34ab) != 1) e->kind11(thing); break;
    case 15: e->weather(thing); break;
    case 18: if (setting((u32)(uintptr_t)&gWorldUpdate_fb34ab) == 1) both(e->chat, thing); break;
    case 19: if (setting((u32)(uintptr_t)&gWorldUpdate_fb34ab) != 1) e->kind19(thing); break;
    default: break;
    }
}

void __fastcall World_Update(u8 *world) {
    ++at<u32>(world, 0x1da70c);
    if (at<u32>(world, 0x1da738) != 4) {
        for (u16 handle = first_handle(); handle; ) {
            u8 *const thing = thing_of(handle);
            if (thing && !thing[3]) Thing_Update(thing);
            handle = next_handle(thing);
        }
    }
    check_gates(world);
    Things_ReleaseDeleted();
    g_env->tick(world);
    g_env->actionTick(address((u32)(uintptr_t)&gActionRec_785a08));
}

void __fastcall World_UpdateInMode1(u8 *world) {
    if (setting((u32)(uintptr_t)&gWorldStart_fb34a9) == 1) World_Update(world);
}
}
