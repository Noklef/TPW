// The handyman's, mechanic's, entertainer's and guard's constructors: see staffctors.h.
#include "staffctors.h"

#include <cstdint>

#include "../files/charsets.h"
#include "../sprite/scriptref.h"
#include "../sprite/sprite.h"
#include "mapcell.h"
#include "messagecentre.h"
#include "person.h"
#include "staff.h"
#include "staffwork.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

const StaffCtorsEnv kGameEnv = {&Staff_CtorTyped, &Staff_Ctor, &Charset_FormatWide, &World_AddHandyman, &World_AddMechanic, &World_AddEntertainer, &Mechanic_SetState, &Map_SpreadAdd, &MessageCentre_AddThing, &Handyman_PickWork, &Mechanic_PickWork, &Entertainer_PickWork, &World_AddGuard, &SpriteFolder_RandomSprite,
    GameAddress<u32(__fastcall *)(u8 *)>(0x00506a40), &Person_SetRandomDest, &Guard_SetState};
const StaffCtorsEnv *g_env = &kGameEnv;

template <typename T>
T &at(u8 *base, u32 offset) {
    return *reinterpret_cast<T *>(base + offset);
}

template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

u8 *world() { return global<u8 *>(0x0080239c); }

void named(u8 *staff, u32 charset, u32 name) {
    g_env->format(global<u8 *>(charset), reinterpret_cast<u16 *>(staff + 0x1a0), 0x20, name);
}

void started(u8 *staff, u32 folder, u32 sprite) {
    const u32 low = sprite & 0xff;
    at<u32>(staff, 0x24) = folder;
    at<u32>(staff, 0x20) = low;
    ScriptRef_Start(staff + 0xc, nullptr, 3, folder, low);
}

void mechanicRest(u8 *mechanic, u32 sprite) {
    at<u16>(mechanic, 0x210) = 0;
    g_env->addMechanic(world(), nullptr, mechanic);
    started(mechanic, 6, sprite);
    at<u16>(mechanic, 0x218) = 0;
    at<u32>(mechanic, 0x214) = 0;
    g_env->mechanicState(mechanic, nullptr, 0);
    g_env->addThing(MessageCentre_Instance(), nullptr, mechanic, 0xa);
    g_env->mechanicWork(mechanic);
}

u32 cellOf(u8 *staff) { return (u16)((at<u8>(staff, 7) << 7) + at<u8>(staff, 5) + 1); }

void entertainerRest(u8 *entertainer, u32 sprite) {
    at<u16>(entertainer, 0x210) = 0;
    g_env->addEntertainer(world(), nullptr, entertainer);
    started(entertainer, 4, sprite);
    at<u16>(entertainer, 0xc0) = global<u16>((u32)(uintptr_t)&gStaff_75c7f0 + 0x8);
    g_env->spread(global<u8 *>(0x008023a0), nullptr, 0, cellOf(entertainer));
    at<u32>(entertainer, 0x214) = 0;
    g_env->entertainerWork(entertainer);
}

}  // namespace

void StaffCtors_SetEnv(const StaffCtorsEnv *env) { g_env = env ? env : &kGameEnv; }

u8 *__fastcall Handyman_Ctor(u8 *handyman, void *, u32 cell, u32 type, u32 sprite, u32 name) {
    g_env->typed(handyman, nullptr, cell, 5, type, sprite);
    named(handyman, (u32)(uintptr_t)&gLanguage_78b530 + 0x578, name);
    at<u16>(handyman, 0x210) = 0;
    g_env->addHandyman(world(), nullptr, handyman);
    at<u32>(handyman, 0x214) = 0;
    at<u16>(handyman, 0x218) = 0;
    at<u16>(handyman, 0x21a) = 0;
    started(handyman, 5, sprite);
    g_env->addThing(MessageCentre_Instance(), nullptr, handyman, 0xa);
    g_env->handymanWork(handyman);
    return handyman;
}

u8 *__fastcall Mechanic_Ctor(u8 *mechanic, void *, u32 cell, u32 type, u32 sprite, u32 name) {
    g_env->typed(mechanic, nullptr, cell, 4, type, sprite);
    named(mechanic, (u32)(uintptr_t)&gLanguage_78b530 + 0x57c, name);
    mechanicRest(mechanic, sprite);
    return mechanic;
}

u8 *__fastcall Mechanic_CtorLoad(u8 *mechanic, void *, u32 cell) {
    g_env->plain(mechanic, nullptr, cell, 4);
    mechanicRest(mechanic, gMechanic_Sprite_7cdbec);
    return mechanic;
}

u8 *__fastcall Entertainer_Ctor(u8 *entertainer, void *, u32 cell, u32 type, u32 sprite, u32 name) {
    g_env->typed(entertainer, nullptr, cell, 6, type, sprite);
    named(entertainer, (u32)(uintptr_t)&gLanguage_78b530 + 0x580, name);
    entertainerRest(entertainer, sprite);
    return entertainer;
}

u8 *__fastcall Entertainer_CtorLoad(u8 *entertainer, void *, u32 cell) {
    g_env->plain(entertainer, nullptr, cell, 6);
    entertainerRest(entertainer, (u8)gEntertainer_NextSprite_7cdb60);
    gEntertainer_NextSprite_7cdb60 = (gEntertainer_NextSprite_7cdb60 + 1) % 3;
    return entertainer;
}

u8 *__fastcall Guard_Ctor(u8 *guard, void *, u32 cell, u32 type, u32 sprite, u32 name) {
    g_env->typed(guard, nullptr, cell, 7, type, sprite);
    named(guard, (u32)(uintptr_t)&gLanguage_78b530 + 0x584, name);
    Guard_Init(guard);
    return guard;
}

void __fastcall Guard_Init(u8 *guard) {
    at<u16>(guard, 0x210) = 0;
    g_env->addGuard(world(), nullptr, guard);
    at<u32>(guard, 0x24) = 7;
    const u32 sprite = g_env->randomSprite(7);
    at<u32>(guard, 0x20) = sprite;
    ScriptRef_Start(guard + 0xc, nullptr, 3, 7, sprite);
    g_env->spread(global<u8 *>(0x008023a0), nullptr, 3, cellOf(guard));
    at<u16>(guard, 0x212) = 0;
    at<u32>(guard, 0x214) = 0;
    g_env->addThing(MessageCentre_Instance(), nullptr, guard, 0x1b);
    if (g_env->busy(guard)) return;
    g_env->guardState(guard, nullptr, (global<u8>((u32)(uintptr_t)world() + 0x1da70c) & 3) && g_env->randomDest(guard) ? 1 : 0);
}
