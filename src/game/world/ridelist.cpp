#include "ridelist.h"

#include <cstdint>

#include "objectdefs.h"
#include "random.h"
#include "things.h"
#include "../coaster/ridebounds.h"
#include "../core/soundplay.h"
#include "../math/x87.h"
#include "../render/particles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

bool system() { return global<u32>(0x008791a0) != 0; }

u8 *find(s32 id) {
    for (u8 *node = global<u8 *>(0x008791b0); node; node = at<u8 *>(node, 0))
        if (at<s32>(node, 8) == id)
            return node;
    return nullptr;
}

}  // namespace

extern "C" {

u8 *__cdecl Rides_FindById(s32 id) {
    if (!system())
        return nullptr;
    return find(id);
}

void __cdecl Rides_SetElement(u8 *record, s32 index, u32 value) {
    if (!system() || !record)
        return;
    if (index >= 0 && index < at<s32>(record, 0x8c))
        at<u32 *>(record, 0x1c)[index] = value;
}

u32 __cdecl Rides_GetElement(const u8 *record, s32 index) {
    if (!system() || !record)
        return 0;
    if (index >= 0 && index < at<s32>(record, 0x8c))
        return at<const u32 *>(record, 0x1c)[index];
    return 0;
}

void __cdecl Rides_SetWordC0(u8 *record, u32 value) {
    if (!system() || !record)
        return;
    at<u16>(record, 0xc0) = (u16)value;
}

s32 __cdecl Rides_GetWordC0(const u8 *record) {
    if (!system() || !record)
        return 0;
    return at<s16>(record, 0xc0);
}

u32 __cdecl Rides_Get9c(const u8 *record) {
    if (!system() || !record)
        return 0;
    return at<u32>(record, 0x9c);
}

u32 __cdecl Rides_GetE0ById(s32 id) {
    if (!system())
        return 0;
    const u8 *record = find(id);
    return record ? at<u32>(record, 0xe0) : 0;
}

u32 __cdecl Rides_GetLinkedElement(s32 id, s32 index) {
    if (!system())
        return 0;
    const u8 *first = find(id);
    if (!first)
        return 0;
    const u8 *second = find(at<s32>(first, 0x14));
    if (!second)
        return 0;
    if (index >= 0 && index < at<s32>(second, 0x8c))
        return at<const u32 *>(second, 0x1c)[index];
    return 0;
}

void __fastcall Obj_ClearRideElement4(const u8 *object) { Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 4, 0); }

u32 __fastcall Obj_RideElement7Positive(const u8 *object) { return (s32)Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 7) > 0; }

u32 __fastcall Obj_RideElement0Differs(const u8 *object) {
    return Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 0) != at<u16>(object, 0x3c);
}

void __fastcall Obj_SetSpeed(u8 *object, void *, u32 speed) {
    Rides_SetWordC0(Rides_FindById(at<s32>(object, 0x24)), speed);
    at<u32>(object, 0x58) = speed;
}

void __fastcall Obj_SetDuration(u8 *object, void *, u32 duration) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 value = (s32)(duration & 0xff);
    if (value < at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x12c))
        value = (u8)at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x12c);
    else if (value > at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x130))
        value = (u8)at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x130);
    Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 3, (u8)value);
    at<u8>(object, 0x5c) = (u8)value;
}

s32 __fastcall Game_TakeRideElement1(u8 *game) {
    u32 handle = at<u16>(game, 0x1da72a);
    if (handle == 0)
        return -1;
    const s32 element = (s32)Rides_GetElement(Rides_FindById(at<s32>(*reinterpret_cast<u8 **>(0x007cfb90 + 20 * handle), 0x24)), 1);
    if (element != 6)
        return element;
    handle = at<u16>(game, 0x1da72a);
    if (handle == 0)
        return -1;
    Rides_SetElement(Rides_FindById(at<s32>(*reinterpret_cast<u8 **>(0x007cfb90 + 20 * handle), 0x24)), 1, 0);
    at<u16>(game, 0x1da72a) = 0;
    return -1;
}

s32 __fastcall Game_PollRideElement1(u8 *game) {
    const u16 handle = at<u16>(game, 0x1da72a);
    if (handle == 0 || handle != at<u16>(game, 0x1da72c))
        return 1;
    if (Game_TakeRideElement1(game) == 3)  // (each take inlined in the original)
        return 0;
    if (at<s32>(game, 0x2d0) >= 10)
        return 1;
    if (Game_TakeRideElement1(game) == 1)
        return 0;
    s32 element = -1;  // the third take finds the thing with Thing_ByHandle when it clears the element
    if (at<u16>(game, 0x1da72a) != 0) {
        element = (s32)Rides_GetElement(Rides_FindById(at<s32>(*reinterpret_cast<u8 **>(0x007cfb90 + 20 * (u32)at<u16>(game, 0x1da72a)), 0x24)), 1);
        if (element == 6) {
            element = -1;
            if (at<u16>(game, 0x1da72a) != 0) {
                Rides_SetElement(Rides_FindById(at<s32>(Thing_ByHandle(at<u16>(game, 0x1da72a)), 0x24)), 1, 0);
                at<u16>(game, 0x1da72a) = 0;
            }
        }
    }
    return element == 2 ? 0 : 1;
}

void __cdecl Obj_FillLinkedElements(u8 *object) {
    if (!object)
        return;
    u8 *slot = at<u8 *>(object, 0x128) + 0x58;
    for (u32 i = 0; i < at<u32>(object, 0xf4); ++i, slot += 0x80) {
        if (!at<u32>(slot, (u32)-4))
            at<u32>(slot, (u32)-4) = Rides_GetLinkedElement(at<s32>(object, 0x24), 5);
        if (!at<u32>(slot, 0))
            at<u32>(slot, 0) = Rides_GetLinkedElement(at<s32>(object, 0x24), 6);
        if (!at<u32>(slot, 4))
            at<u32>(slot, 4) = Rides_GetLinkedElement(at<s32>(object, 0x24), 7);
        if (!at<u32>(slot, 8))
            at<u32>(slot, 8) = Rides_GetLinkedElement(at<s32>(object, 0x24), 8);
    }
}

void __cdecl Objects_SetRideId(u32 id, u32 value) {
    u8 *object = *reinterpret_cast<u8 **>(0x00790fe0);
    while (object && at<u32>(object, 0x10) != id)
        object = at<u8 *>(object, 0x18);
    if (!object)
        return;
    at<u32>(object, 0x24) = value;
    Obj_FillLinkedElements(object);
}

u32 __fastcall Obj_RollChance(const u8 *object, void *, u32) {
    // The original finds the object's definition (which makes it the cache's) and reads a dword of it for its debug print.
    const volatile u32 forPrint = at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe)), 0x4ac);
    (void)forPrint;
    const u32 roll = (u32)World_Rand(*reinterpret_cast<u8 **>(0x007cf83c)) % 100;
    const u32 hit = at<u8>(object, 0x190) >= roll ? 1 : 0;
    Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 0xb, hit);
    return hit;
}

void __fastcall Obj_SetRideElement4One(const u8 *object) { Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 4, 1); }

void __fastcall Obj_ReleaseRideElement0(u8 *object, void *, u32 handle) {
    at<u16>(object, 0x6c) = 0;
    if (Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 0) == (handle & 0xffff))
        Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 0, 0);
}

u32 __fastcall Obj_ClaimRideElement0(u8 *object, void *, u32 handle) {
    const u32 state = at<u32>(object, 0x19c);
    if (state == 1 || at<u32>(object, 0x68) == 0 || state == 4)
        return 0;
    at<u16>(object, 0x6c) = 0;  // sic
    u8 *const ride = Rides_FindById(at<s32>(object, 0x24));
    if (Rides_GetElement(ride, 0) != 0)
        return 0;
    Rides_SetElement(ride, 0, handle & 0xffff);
    return 1;
}

s32 __fastcall Game_GetRideElement1(const u8 *game) {
    u32 handle = at<u16>(game, 0x1da732);
    if (handle == 0)
        return -1;
    u8 *thing = *reinterpret_cast<u8 **>(0x007cfb90 + 20 * handle);
    return (s32)Rides_GetElement(Rides_FindById(at<s32>(thing, 0x24)), 1);
}

void __cdecl Ride_SetRecordId(u32 index, u32 value) {
    if (!index)
        return;
    u8 *const ride = reinterpret_cast<u8 *const *>(0x007a4610)[index];
    at<u32>(ride, 0xd4) = value;
    if (at<u32>(ride, 4) & 0x40000)
        Objects_SetRideId(index, value);
}

void __cdecl Ride_PlayEffect(s32 id) {
    const u8 *const node = Rides_FindById(id);
    const u16 handle = at<u16>(node, 0xac);   // (no record: read through null, sic)
    if (handle == 0)
        return;
    const u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe));
    const s32 effect = at<s32>(def, 0x6c);
    if (effect == 0 || at<s32>(node, 0xc8) == 0)
        return;
    float box[6];
    if (Ride_Bounds(at<s32>(node, 0xc8), box) == 0)
        return;
    const float &half = global<float>(0x00700fd0), &scale = global<float>(0x00700fdc);
    const X87Reg zc = (X87Reg(box[2]) + box[5]) * half;
    const X87Reg xc = (X87Reg(box[0]) + box[3]) * half;
    const u8 *const ride = global<u8 *>(0x007a4610 + 4 * at<u32>(node, 0xc8));
    const float z = (float)zc;
    const float y = at<float>(at<u8 *>(at<u8 *>(at<u8 *>(ride, 8), 4), 0x78), 0x44);
    const float x = (float)xc;
    const s32 iz = Ftol(zc * scale);   // (the centre's z unrounded, still in the register)
    const s32 iy = Ftol(X87Reg(y) * scale);
    const s32 ix = Ftol(X87Reg(x) * scale);
    const s32 system = Particle_Spawn(effect, ix, iy, iz);
    const s32 index = at<s32>(node, 0xc8);
    if (index != 0 && Ride_Bounds(index, box) != 0) {
        const float &across = global<float>(0x00701050), &up = global<float>(0x00701054);
        const X87Reg dz = X87Reg(box[5]) - box[2];
        const X87Reg dy = X87Reg(box[4]) - box[1];
        const float dx = (float)(X87Reg(box[3]) - box[0]);
        const s32 vz = Ftol(dz * across);
        const s32 vy = Ftol(dy * up);
        const s32 vx = Ftol(X87Reg(dx) * across);
        ParticleSystem_SetVectorB((u32)system, vx, vy, vz);
    }
    const s32 sz = Ftol(X87Reg(z));
    const s32 sy = Ftol(X87Reg(y));
    const s32 sx = Ftol(X87Reg(x));
    Sound_PlayAt(0, global<u32>(0x00803a28), 0xb8, sx, sy, sz);
}
}  // extern "C"
