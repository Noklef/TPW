#include "visitorsteps.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../sprite/script.h"
#include "../sprite/scriptref.h"
#include "../sprite/sprite.h"
#include "analysercell.h"
#include "mapcell.h"
#include "object.h"
#include "objectmsg.h"
#include "person.h"
#include "random.h"
#include "ridelist.h"
#include "thingmsg.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const VisitorStepsEnv kGameEnv = {&Sprite_WalkNav,       &Sprite_SetDestination, &Sprite_SetDestinationCell,
                                  &Sprite_SpawnParticle, &Thing_Delete,          &Visitor_SetState,
                                  &Object_BackOfQueueTo, &Object_QueuePosition,  &Object_AnyQueueSpot,
                                  &Person_PlaceSprite};
const VisitorStepsEnv *g_env = &kGameEnv;

u8 *world() { return global<u8 *>(0x0080239c); }
const u8 *map() { return global<const u8 *>(0x008023a0); }

// The visitor's own cell: (row << 7) + column + 1.
u16 ownCell(const u8 *visitor) { return (u16)(((u32)visitor[7] << 7) + visitor[5] + 1); }

// The word -0x20 of the cell's 0x44-byte record in [0x8023a0] (a thing's handle: the one standing at the gate, at the stop).
u16 cellMark(u16 cell) { return at<u16>(map(), (u32)cell * 0x44 - 0x20); }

u16 handleOf(const u8 *visitor) {
    u16 h;
    return *Thing_GetHandle(visitor, nullptr, &h);
}

// The bus stop's four cells: the stop, the one after, the one a row before and that one's next.
void busStops(u16 stops[4]) {
    u16 p;
    stops[0] = *Map_BusStopPos(map(), nullptr, &p, 0);
    stops[1] = (u16)(*Map_BusStopPos(map(), nullptr, &p, 0) + 1);
    stops[2] = (u16)(*Map_BusStopPos(map(), nullptr, &p, 0) - 0x100);
    stops[3] = (u16)(*Map_BusStopPos(map(), nullptr, &p, 0) - 0xff);
}

// The walk to a random spot of the cell (two rolls for the low bytes), then the state. (The original builds x and y in stack dwords whose
// high halves it never sets; Sprite_SetDestination stores and uses only the low words.)
void walkToSpot(u8 *visitor, u16 cell, u32 state) {
    const u32 rx = (u32)World_Rand(world());
    const u32 ry = (u32)World_Rand(world());
    const u32 index = (u32)cell - 1;
    const u32 x = (index & 0x7f) << 8 | (rx & 0xff);
    const u32 y = (index >> 7 & 0xff) << 8 | (ry & 0xff);
    g_env->setDestination(visitor, nullptr, (s32)x, (s32)y);
    g_env->setState(visitor, nullptr, state);
}

}  // namespace

void VisitorSteps_SetEnv(const VisitorStepsEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Visitor_StepArriving(u8 *visitor) {
    if (Get1da710_51a280(world()) != 0 || Game_GetRideElement1(world()) != 1) {
        if (at<u32>(visitor, 0x200)) {
            --at<u32>(visitor, 0x200);
            return;
        }
        at<u32>(visitor, 0x188) = 1;
        const u32 side = (u32)World_Rand(world()) & 1;
        u16 p;
        g_env->setDestinationCell(visitor, nullptr, *Map_CrossingParkSidePos(map(), nullptr, &p, side));
        g_env->setState(visitor, nullptr, 0x12);
        at<u32>(visitor, 0x1bc) = 0;
        return;
    }
    u16 p;
    if (at<u32>(visitor, 0x204)) {
        if (cellMark(ownCell(visitor)) != handleOf(visitor)) return;
        const u32 side = (u32)World_Rand(world()) & 1;
        g_env->setDestinationCell(visitor, nullptr, *Map_EntrancePos(map(), nullptr, &p, side));
        g_env->setState(visitor, nullptr, 5);
        return;
    }
    u16 booth;
    if (ownCell(visitor) == *Map_TicketBoothPos(map(), nullptr, &p, 0))
        booth = *Map_TicketBoothPos(map(), nullptr, &p, 0);
    else if (ownCell(visitor) == *Map_TicketBoothPos(map(), nullptr, &p, 1))
        booth = *Map_TicketBoothPos(map(), nullptr, &p, 1);
    else
        booth = *Map_TicketBoothPos(map(), nullptr, &p, (u32)World_Rand(world()) & 1);
    g_env->setDestinationCell(visitor, nullptr, booth);
    at<u32>(visitor, 0x200) = 0;
    g_env->setState(visitor, nullptr, 2);
}

void __fastcall Visitor_StepLeavingRide(u8 *visitor) {
    const u32 walked = g_env->walkNav(visitor);
    if (walked) {
        if (walked == 2) {
            at<u16>(visitor, 0x1dc) = 0;
            g_env->setState(visitor, nullptr, 6);
        }
        return;
    }
    const u16 pending = at<u16>(visitor, 0x1de);
    if (pending) {
        at<u16>(visitor, 0x1dc) = pending;
        at<u16>(visitor, 0x1de) = 0;
        if (Thing_ByHandle(pending)[2] == 3) {
            u16 back = 0;
            g_env->backOfQueueTo(Thing_ByHandle(at<u16>(visitor, 0x1dc)), nullptr, &back);
            if (back != 0 && g_env->setDestinationCell(visitor, nullptr, back)) {
                // Debug_Printf(2, "Left minor destination, found ol...")
                g_env->setState(visitor, nullptr, 10);
                return;
            }
        } else {
            at<u16>(visitor, 0x1dc) = 0;   // Debug_Printf(2, "Deleted major dest while I was d...")
        }
    }
    // Debug_Printf(2, "Person %d successfully left ride ...", its handle, +0x1dc)
    g_env->setState(visitor, nullptr, 6);
}

void __fastcall Visitor_GoToBusStop(u8 *visitor) {
    u16 stops[4];
    busStops(stops);
    if (Game_PollRideElement1(world()) == 0) return;
    const u16 cell = stops[World_Rand(world()) & 3];
    walkToSpot(visitor, cell, 0x14);
}

void __fastcall Visitor_StepAtBusStop(u8 *visitor) {
    const s32 element = Game_TakeRideElement1(world());
    if (element == 4) {
        u16 cell;
        Cell_MakeHandle(&cell, visitor[5], visitor[7]);
        if (cellMark(cell) != handleOf(visitor)) return;
        g_env->spawnParticle(visitor, nullptr, 0x13);
        g_env->thingDelete(visitor);
        return;
    }
    if (element != 1 && element != 2 && element != 3) return;
    u16 stops[4];
    busStops(stops);
    const u16 *const pair = World_ArrivalVehicleNotSize1(world()) ? stops + 2 : stops;
    if (ownCell(visitor) == pair[0] || ownCell(visitor) == pair[1]) return;
    const u16 cell = pair[World_Rand(world()) & 1];
    walkToSpot(visitor, cell, 0x14);
}

u32 __fastcall Visitor_FindQueueDestination(u8 *visitor) {
    const s32 place = g_env->queuePosition(Thing_ByHandle(at<u16>(visitor, 0x1dc)), nullptr, handleOf(visitor));
    if (place == -1) return 0;
    at<u8>(visitor, 0x1f1) = (u8)place;
    u16 cell = 0;
    u8 x = 0, y = 0;
    g_env->anyQueueSpot(Thing_ByHandle(at<u16>(visitor, 0x1dc)), nullptr, (u32)place, &cell, &x, &y);
    const u32 index = (u32)cell - 1;   // (the high halves of the dwords built are the stack's in the original; only the low words are used)
    const u32 dx = (index & 0x7f) << 8 | x;
    const u32 dy = (index >> 7 & 0xff) << 8 | y;
    if (g_env->setDestination(visitor, nullptr, (s32)dx, (s32)dy)) {
        g_env->setState(visitor, nullptr, 0xc);
        return 1;
    }
    // Debug_Printf(2, "QQQ - FindQueueDestination SetDe...")
    return 0;
}

void __fastcall Visitor_KillScripts(u8 *visitor) {
    if (at<u32>(visitor, 0x210)) {
        SpriteScript_Kill((s32)at<u32>(visitor, 0x210));
        at<u32>(visitor, 0x210) = 0;
    }
    Obj_KillSlot(visitor + 0xc);
    g_env->setState(visitor, nullptr, 0x11);
}

void __fastcall Visitor_GoToTicketBooth(u8 *visitor) {
    if (Game_PollRideElement1(world()) == 0) return;
    const u32 side = (u32)World_Rand(world()) & 1;
    u16 p;
    const u16 cell = *Map_TicketBoothPos(map(), nullptr, &p, side);
    walkToSpot(visitor, cell, 2);
}

void __fastcall Visitor_StepWalkingIn(u8 *visitor) {
    u16 speed;
    if (World_ArrivalVehicleNotSize1(world()) == 0 && Game_TakeRideElement1(world()) == 3)
        speed = global<u16>(0x0075c7f4);   // Debug_Printf(2, "The bus is coming, RUUUUUUUUUUN...")
    else
        speed = (handleOf(visitor) & 3) == 0 ? global<u16>(0x0075c7f2) : global<u16>(0x0075c7f0);
    at<u16>(visitor, 0xc2) = speed;
    if (g_env->walkNav(visitor) != 0) return;   // (2: Debug_Printf(0, "Big problem ..."))
    g_env->setState(visitor, nullptr, Get1da710_51a280(world()) ? 3 : 4);
}

void __fastcall Visitor_StepEntering(u8 *visitor) {
    if (Get1da710_51a280(world()) != 0 || Game_GetRideElement1(world()) != 1) {
        const u32 side = (u32)World_Rand(world()) & 1;
        u16 p;
        g_env->setDestinationCell(visitor, nullptr, *Map_TicketBoothPos(map(), nullptr, &p, side));
        g_env->setState(visitor, nullptr, 2);
        return;
    }
    const u32 walked = g_env->walkNav(visitor);
    if (walked != 0) return;   // (2: Debug_Printf(0, "Big problem ..."))
    at<u32>(visitor, 0x1d8) = World_AddVisitorToDate(world());
    g_env->setState(visitor, nullptr, 6);
}

void __fastcall Visitor_StepLeavingPark(u8 *visitor) {
    if (at<s32>(visitor, 0x1bc) > 0 && (u8)(long long)at<float>(visitor, 0x19c) != 0 && Get1da710_51a280(world()) == 0 &&
        AnalyserCell_TypePlain(visitor) != 0) {
        // Debug_Printf(2, "Make up your mind!")
    } else {
        const u32 walked = g_env->walkNav(visitor);
        if (walked == 0) {
            g_env->setState(visitor, nullptr, 0x13);
            return;
        }
        if (walked != 2) return;
        // Debug_Printf(2, "I'm stuck in the park, even thou...")
    }
    at<u32>(visitor, 0x188) = 0;
    g_env->setState(visitor, nullptr, 6);
}

void __fastcall Visitor_ReenterState(u8 *visitor) { g_env->setState(visitor, nullptr, at<u32>(visitor, 0x224)); }

u8 *__fastcall Visitor_Ctor(u8 *visitor, void *, u32 cell) {
    MovingPerson_Ctor(visitor, nullptr, cell, 1);
    for (u32 offset = 0x19c; offset <= 0x1bc; offset += 4) at<u32>(visitor, offset) = 0;
    at<u8>(visitor, 0x1c0) = 0;
    for (u32 offset = 0x1c4; offset <= 0x1d4; offset += 4) at<u32>(visitor, offset) = 0;
    at<u16>(visitor, 0x1dc) = 0;
    at<u16>(visitor, 0x1de) = 0;
    at<u8>(visitor, 0x1f0) = 0;
    at<u8>(visitor, 0x1f1) = 0;
    for (u32 offset = 0x1f4; offset <= 0x21c; offset += 4) at<u32>(visitor, offset) = 0;
    at<u32>(visitor, 0x220) = 6;
    at<u32>(visitor, 0x224) = 6;
    at<u16>(visitor, 0x228) = 0;
    at<u16>(visitor, 0x22a) = 0;
    at<u32>(visitor, 0x1d8) = 0;
    at<u32>(visitor, 0x1d4) = at<u32>(world(), 0x1da70c);
    // (gBalanceLevel: the exit level and its variance, the peep types' count and table, the cash percentage, the prankery likelihood)
    at<s32>(visitor, 0x1bc) = global<s32>(0x0078504c);
    const s32 variance = global<s32>(0x00785050);
    at<s32>(visitor, 0x1bc) += World_Rand(world()) % (variance * 2) - variance;
    const s32 type = World_Rand(world()) % global<s32>(0x007851d4);
    at<u8>(visitor, 0x1f0) = (u8)type;
    const u32 base = global<u32>(0x007850e8 + 12 * (u32)type);
    const s32 percent = global<s32>(0x00785054);
    const s32 roll = World_Rand(world()) % (percent * 2 + 1);
    const s32 cash = (s32)((u32)(roll - global<s32>(0x00785054) + 100) * base) / 100;   // (imul, wrapping; signed / 100)
    at<s32>(visitor, 0x1a0) = cash < 0 ? 0 : cash;
    at<u32>(visitor, 0x19c) = 0x42480000u;   // 50.0f
    at<float>(visitor, 0x1a4) = (float)(long long)((u32)World_Rand(world()) % 50u);   // (fild qword: the high dword 0)
    at<float>(visitor, 0x1a8) = (float)(long long)((u32)World_Rand(world()) % 50u);
    const u32 r30 = (u32)World_Rand(world()) % 30u;
    at<u32>(visitor, 0x1b4) = 0;
    at<u32>(visitor, 0x1b0) = 0;
    at<u32>(visitor, 0x1b8) = 0;
    at<float>(visitor, 0x1ac) = (float)(long long)r30;
    World_Rand(world());
    const u8 prank = (u32)World_Rand(world()) % 100u < global<u32>(0x007850dc) ? 100 : 0;
    at<u8>(visitor, 0x1c0) = prank;
    if (prank == 100) at<u8>(visitor, 0x1c0) += (u8)(handleOf(visitor) % 3u);
    at<u16>(visitor, 0x1dc) = 0;
    at<u16>(visitor, 0x1de) = 0;
    at<u16>(visitor, 0x228) = 0;
    at<u16>(visitor, 0x22a) = 0;
    at<u8>(visitor, 0x1f1) = 0;
    at<u32>(visitor, 0x1f8) = 0;
    at<u32>(visitor, 0x208) = 0;
    World_SetSeed(world(), nullptr, handleOf(visitor));
    at<u32>(visitor, 0x24) = 0;
    at<u32>(visitor, 0x20) = SpriteFolder_RandomSprite(0);
    ScriptRef_Start(visitor + 0xc, nullptr, 1, at<u32>(visitor, 0x24), at<u32>(visitor, 0x20));
    at<u16>(visitor, 0xc2) = global<u16>(0x0075c7f2);
    if (AnalyserCell_TypePlain(visitor) != 0) {
        at<u32>(visitor, 0x1d8) = World_AddVisitorToDate(world());
        g_env->setState(visitor, nullptr, 6);
    } else {
        const u32 side = (u32)World_Rand(world()) & 1;
        u16 p;
        const u16 crossing = *Map_CrossingBSSidePos(map(), nullptr, &p, side);
        const u32 rx = (u32)World_Rand(world());
        const u32 index = (u32)crossing - 1;
        // (x's high half is the stack's in the original; y is built in the cell argument's own slot, so its high half is the argument's)
        const u32 x = (index & 0x7f) << 8 | (rx & 0xff);
        const u32 y = (cell & 0xffff0000u) | (index >> 7 & 0xff) << 8 | 0xc8;
        const u32 walking = g_env->setDestination(visitor, nullptr, (s32)x, (s32)y);
        g_env->setState(visitor, nullptr, walking ? 0 : 6);
        g_env->placeSprite(visitor, nullptr, 0.0f);
        g_env->spawnParticle(visitor, nullptr, 0x13);
    }
    at<u32>(visitor, 0x204) = 0;
    for (u32 i = 0; i < 4; ++i) {
        at<u16>(visitor, 0x1e0 + 2 * i) = 0;
        at<u16>(visitor, 0x1e8 + 2 * i) = 0;
    }
    u32 msg[3];
    Object_PostMessage(ThingMsg1c_Ctor(reinterpret_cast<u8 *>(msg), nullptr, handleOf(visitor)));
    return visitor;
}
