#include "objectctor.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/funnytime.h"
#include "../core/globalsave.h"
#include "../math/intpair.h"
#include "../model/stripanim.h"
#include "analyser.h"
#include "mapcell.h"
#include "messagecentre.h"
#include "money.h"
#include "object.h"
#include "objectdefs.h"
#include "objectride.h"
#include "objectunlock.h"
#include "ridelist.h"
#include "staff.h"
#include "thingmsg.h"
#include "things.h"
#include "world.h"
#include "rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *World() { return global<u8 *>(0x0080239c); }
u8 *Defs() { return reinterpret_cast<u8 *>((uintptr_t)0x007890a0); }

u32 __fastcall HasKeyGame(u8 *, void *, u32 key) { return GlobalSave_HasKey(key); }
u8 __fastcall UnlockGame(u8 *, void *, u32 id) { return ObjectDef_Unlock(id); }

const ObjectCtorEnv kGameEnv = {
    &Thing_Construct,
    &World_AddObject,
    &MessageCentre_AddThing,
    &MessageCentre_Post,
    &Map_SpreadAdd,
    &HasKeyGame,
    &UnlockGame,
    &GameTable7cfb90_Get1da726_5195d0,
    &Money_Debit,
    &Object_SetState,
    &Object_ClaimRide,
    reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x004dcf90),
    &Rides_FindById,
    &Rides_SetWordC0,
    &Object_SetCapacity,
    &Rides_SetElement,
    &FunnyTime_Now,
    &ObjectControls_Find,
    &ObjectDefs_GetName,
    &ObjectDefs_GetSuffix,
    &GameTable7cfb90_Get1da720_519510,
    &ParkAnalyser_StampTime21c30,
    &StripAnim_StartGrow,
};
const ObjectCtorEnv *g_env = &kGameEnv;

// The word for a definition's pair (at +offset) turned by the angle: row << 7 plus column.
u16 Turned(const u8 *def, u32 offset, s32 angle) {
    u32 out[2];
    const u32 *const pair = IntPair_Rotated(reinterpret_cast<const u32 *>(def + offset), nullptr, out, angle);
    return static_cast<u16>((pair[1] << 7) + pair[0]);
}

// The six timers' start: -1, 30, a byte 0.
void TimerStart(u8 *object, u32 offset) {
    at<s32>(object, offset) = -1;
    at<u32>(object, offset + 4) = 30;
    at<u8>(object, offset + 8) = 0;
}

// The object's cell word + 1 in the low half of the cell argument's slot.
u32 SpreadCell(const u8 *object, u32 cell) {
    return (cell & 0xffff0000u) | static_cast<u16>((at<u8>(object, 7) << 7) + at<u8>(object, 5) + 1);
}

}  // namespace

extern "C" {

void ObjectCtor_SetEnv(const ObjectCtorEnv *env) { g_env = env ? env : &kGameEnv; }

u8 *__fastcall Object_Construct(u8 *object, void *, u32 cell, u32 id, s32 angle, u32 ride) {
    const ObjectCtorEnv *const E = g_env;
    E->thingConstruct(object, nullptr, cell, 3);
    at<u16>(object, 0xe) = static_cast<u16>(id);
    at<s32>(object, 0x10) = angle;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = ride;
    at<u32>(object, 0x2c) = 1;
    for (u32 offset = 0x30; offset <= 0x3c; offset += 2) at<u16>(object, offset) = 0;
    for (u32 offset = 0x40; offset <= 0x4c; offset += 4) at<u32>(object, offset) = 0;
    at<u8>(object, 0x50) = 0;
    at<u32>(object, 0x54) = 0;
    at<u32>(object, 0x58) = 0;
    at<u8>(object, 0x5c) = 0;
    at<u8>(object, 0x5d) = 0;
    at<u16>(object, 0x5e) = 0;
    at<u32>(object, 0x60) = 0;
    at<u32>(object, 0x64) = 0;
    at<u32>(object, 0x68) = 0;
    at<u16>(object, 0x6c) = 0;
    at<u32>(object, 0x70) = 0;
    TimerStart(object, 0xec);
    at<u32>(object, 0xf8) = 0;
    TimerStart(object, 0x174);
    at<u32>(object, 0x180) = 0;
    at<u32>(object, 0x184) = 0;
    at<u32>(object, 0x19c) = 0;
    at<u32>(object, 0x1a0) = 0;
    at<u32>(object, 0x1a4) = 0;
    at<u32>(object, 0x1a8) = 0;
    TimerStart(object, 0x224);
    at<u32>(object, 0x230) = 0;
    TimerStart(object, 0x2ac);
    at<u32>(object, 0x2b8) = 0;
    TimerStart(object, 0x334);
    at<u32>(object, 0x340) = 0;
    TimerStart(object, 0x3bc);
    at<u16>(object, 0xc) = 0;

    E->addObject(World(), nullptr, object);
    E->addThing(MessageCentre_Instance(), nullptr, object, 0xb);
    u16 handle;
    Thing_GetHandle(object, nullptr, &handle);
    u8 msg[0xc];
    ThingMsg06_Ctor(msg, nullptr, handle);
    E->post(MessageCentre_Instance(), nullptr, msg);

    const u8 *const def = ObjectDefs_Find(Defs(), nullptr, at<u16>(object, 0xe));
    const u16 first = static_cast<u16>(cell + Turned(def, 0x4b0, angle + 0xb4));
    at<u16>(object, 0x34) = first;
    at<u16>(object, 0x36) = static_cast<u16>(first + Turned(def, 0x494, angle));
    at<u16>(object, 0x38) = static_cast<u16>(first + Turned(def, 0x4a0, angle));
    at<u32>(object, 0x44) = 0x42c80000;
    at<u32>(object, 0x4c) = 0x42c80000;
    at<u32>(object, 0x48) = 0x42c80000;
    at<u32>(object, 0x64) = 0;
    at<u8>(object, 0x50) = 0;
    at<u32>(object, 0x54) = 0;
    at<u32>(object, 0x188) = at<u32>(def, 0x140);
    at<u32>(object, 0x18c) = 0x32;
    at<u32>(object, 0x190) = 100 - at<u32>(def, 0xec);
    at<u32>(object, 0x194) = at<u32>(def, 0xe4);
    at<u32>(object, 0x198) = 0x32;
    at<u32>(object, 0x68) = 1;
    at<u16>(object, 0x6c) = 0;
    if (at<u32>(def, 0xf4)) {
        at<u16>(object, 0x32) |= 1;
        E->spread(global<u8 *>(0x008023a0), nullptr, 1, SpreadCell(object, cell));
    }
    if (at<u32>(def, 0xf8)) at<u8>(object, 0x32) |= 0x40;
    if (at<u32>(def, 0xfc)) at<u8>(object, 0x32) |= 2;
    if (at<u32>(def, 0x3c)) at<u8>(object, 0x32) |= 4;
    if (at<u32>(def, 0x104)) at<u8>(object, 0x32) |= 0x20;
    if (at<u32>(def, 0x40)) at<u8>(object, 0x32) |= 8;
    if (at<u32>(def, 0xf0)) {
        at<u8>(object, 0x32) |= 0x10;
        E->spread(global<u8 *>(0x008023a0), nullptr, 4, SpreadCell(object, cell));
    }
    if (at<u32>(def, 0x110)) {
        at<u8>(object, 0x32) |= 0x80;
        E->spread(global<u8 *>(0x008023a0), nullptr, 7, SpreadCell(object, cell));
    }
    if (at<u32>(def, 0x48)) at<u8>(object, 0x33) |= 1;

    if (at<s32>(def, 0xc4) > 0 && static_cast<u8>(E->hasKey(World() + 0x2c0, nullptr, at<u16>(def, 0))) == 0) {
        E->unlock(World() + 0x2c0, nullptr, at<u16>(def, 0));
    } else {
        const s32 price = at<s32>(def, 0x1b8);
        E->debit(reinterpret_cast<u8 *>((uintptr_t)E->money(World())), nullptr, price);
    }
    E->setState(object, nullptr, (at<u8>(object, 0x32) & 4) ? 0 : 3);
    E->claimRide(object, nullptr, 0);
    E->fn4dcf90(object);

    const s32 word = at<s32>(def, 0x1a8);
    if (word > 0) {
        E->setWordC0(E->findRide(at<s32>(object, 0x24)), static_cast<u32>(word));
        at<s32>(object, 0x58) = word;
    }
    if (at<s32>(def, 0x198) > 0) E->setCapacity(object, nullptr, at<u8>(def, 0x198));
    if (at<s32>(def, 0x1a0) > 0) {
        const u8 value = at<u8>(def, 0x1a0);
        const u8 *const limits = ObjectDefs_Find(Defs(), nullptr, at<u16>(object, 0xe));
        u8 clamped = value;
        if (static_cast<s32>(value) < at<s32>(limits, 0x12c))
            clamped = at<u8>(limits, 0x12c);
        else if (static_cast<s32>(value) > at<s32>(limits, 0x130))
            clamped = at<u8>(limits, 0x130);
        E->setElement(E->findRide(at<s32>(object, 0x24)), 3, clamped);
        at<u8>(object, 0x5c) = clamped;
    }

    FileTime time;
    const FileTime *const started = E->now(World() + 0x2a0, nullptr, &time, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    at<u32>(object, 0x18) = started->low;
    at<u32>(object, 0x1c) = started->high;

    if (at<u32>(E->findControls(World() + 0x1d943c, nullptr, id, 0), 0x1c) == 0) {
        const u32 stamp = at<u32>(World(), 0x1da70c);
        at<u32>(E->findControls(World() + 0x1d943c, nullptr, id, 0), 0x1c) = stamp;
    }
    at<s32>(E->findControls(World() + 0x1d943c, nullptr, id, 0), 0x18) += 1;

    char copy[0x20];
    Crt().strncpy(copy, at<const char *>(def, 0x14), 0x20);
    E->getName(Defs(), nullptr, id, reinterpret_cast<u16 *>(object + 0x3c8), 0x20);
    E->getSuffix(Defs(), nullptr, id, reinterpret_cast<u16 *>(object + 0x40a), 0x20);

    if (at<u8>(object, 0x32) & 8) {
        at<u32>(object, 0x68) = 0;
        at<u16>(object, 0x6c) = 0;
        E->setElement(E->findRide(at<s32>(object, 0x24)), 6, 1);
        E->startGrow(gRides_7a4610[at<u32>(object, 0x20)], 1);
    }
    E->stampTime(reinterpret_cast<u8 *>((uintptr_t)E->analyser(World())));
    return object;
}
}
