#include "parkopen.h"

#include <cstdint>

#include "object.h"
#include "objectmsg.h"
#include "../sprite/draw.h"
#include "ridelist.h"
#include "thingmsg.h"
#include "things.h"
#include "thingscan.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ParkOpenEnv kGameEnv = {&Object_Close,
                              reinterpret_cast<void(__fastcall *)(void *, void *, u8 *)>((uintptr_t)0x00481b50)};
const ParkOpenEnv *g_env = &kGameEnv;

u8 *thing(u32 handle) { return reinterpret_cast<u8 *const *>((uintptr_t)0x007cfb90)[(handle & 0xffff) * 5]; }

// The gate's ride record's element 1 read, then its element 0 set.
void setGate(const u8 *game, u32 value) {
    const u8 *const gate = Thing_ByHandle(at<u16>(game, 0x1da732));
    Rides_GetElement(Rides_FindById(at<s32>(gate, 0x24)), 1);
    Rides_SetElement(Rides_FindById(at<s32>(gate, 0x24)), 0, value);
}

void post(u32 a) {
    u32 msg[3];
    Object_PostMessage(ThingMsg13_Ctor(reinterpret_cast<u8 *>(msg), nullptr, a));
}

}  // namespace

extern "C" {

void ParkOpen_SetEnv(const ParkOpenEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Game_OpenPark(u8 *game, void *, u32 open, u32 gateOnly) {
    if (gateOnly) {
        setGate(game, 2);
        return;
    }
    if (open) {
        if (at<u32>(game, 0x1da710)) {
            at<u32>(game, 0x1da710) = 0;
            setGate(game, 1);
            for (u16 h = at<u16>(game, 0x1da746); h;) {
                u8 *const object = thing(h);
                if ((object[0x32] & 4) && Object_CanOpen(object)) g_env->openObject(object);
                h = at<u16>(object, 0xc);
            }
        }
        post(3);
        return;
    }
    if (!at<u32>(game, 0x1da710)) {
        at<u32>(game, 0x1da710) = 1;
        if (Things_CountKind1OnPlainCell() == 0) {
            const u16 gate = at<u16>(game, 0x1da732);
            const s32 state = gate ? (s32)Rides_GetElement(Rides_FindById(at<s32>(Thing_ByHandle(gate), 0x24)), 1) : -1;
            if (state == 1) setGate(game, 0);
        }
        for (u16 h = at<u16>(game, 0x1da746); h;) {
            u8 *const object = thing(h);
            if (object[0x32] & 4) Object_RestartRide(object);
            h = at<u16>(object, 0xc);
        }
    }
    post(4);
}

void __fastcall Game_EndPark(u8 *game) {
    at<u32>(game, 0x1da738) = 4;
    g_env->endGate(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, thing(at<u16>(game, 0x1da732)));
    SpriteDraw_Disable();
    const ThingSlot *const slots = reinterpret_cast<const ThingSlot *>((uintptr_t)0x007cfb90);
    const ThingSlot *const first = *reinterpret_cast<ThingSlot *const *>((uintptr_t)0x007cf56c);
    for (u16 h = first ? (u16)first->id : 0; h;) {
        u8 *const t = thing(h);
        if (t && !t[3] && t[2] == 3 && (t[0x32] & 4)) Obj_SetRideElement4One(t);
        u16 index;
        Thing_GetHandle(t, nullptr, &index);
        const ThingSlot *const next = slots[index].next;
        h = next ? (u16)next->id : 0;
    }
    Things_ReleaseDeleted();
    Game_OpenPark(game, nullptr, 0, 1);
}
}
