#include "visitorsteps.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "mapcell.h"
#include "objectmsg.h"
#include "person.h"
#include "random.h"
#include "ridelist.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const VisitorStepsEnv kGameEnv = {&Sprite_WalkNav,       &Sprite_SetDestination, &Sprite_SetDestinationCell,
                                  &Sprite_SpawnParticle, &Thing_Delete,          &Visitor_SetState,
                                  &Object_BackOfQueueTo, &Object_QueuePosition,  &Object_AnyQueueSpot};
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

// The walk to a random spot of the cell (two rolls for the low bytes), state 0x14. (The original builds x and y in stack dwords whose high
// halves it never sets; Sprite_SetDestination stores and uses only the low words.)
void walkToStop(u8 *visitor, u16 cell) {
    const u32 rx = (u32)World_Rand(world());
    const u32 ry = (u32)World_Rand(world());
    const u32 index = (u32)cell - 1;
    const u32 x = (index & 0x7f) << 8 | (rx & 0xff);
    const u32 y = (index >> 7 & 0xff) << 8 | (ry & 0xff);
    g_env->setDestination(visitor, nullptr, (s32)x, (s32)y);
    g_env->setState(visitor, nullptr, 0x14);
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
    walkToStop(visitor, cell);
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
    walkToStop(visitor, cell);
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
