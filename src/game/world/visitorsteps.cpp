#include "visitorsteps.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/globals.h"
#include "../core/indexed.h"
#include "../core/rings.h"
#include "../math/x87.h"
#include "../sprite/script.h"
#include "../sprite/scriptref.h"
#include "../sprite/sprite.h"
#include "analyser.h"
#include "analysercell.h"
#include "mapcell.h"
#include "money.h"
#include "object.h"
#include "messagecentre.h"
#include "objectmsg.h"
#include "celleffect.h"
#include "person.h"
#include "rideupkeep.h"
#include "personscripts.h"
#include "random.h"
#include "ridelist.h"
#include "staff.h"
#include "thingscan.h"
#include "thingmsg.h"
#include "things.h"
#include "visitoroption.h"
#include "world.h"
#include "visitorsteps.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const VisitorStepsEnv kGameEnv = {&Sprite_WalkNav,       &Sprite_SetDestination, &Sprite_SetDestinationCell,
                                  &Sprite_SpawnParticle, &Thing_Delete,          &Visitor_SetState,
                                  &Object_BackOfQueueTo, &Object_QueuePosition,  &Object_AnyQueueSpot,
                                  &Person_PlaceSprite,   &Sprite_PlaySound,      &Visitor_FindQueueDestination,
                                  &Visitor_ChooseMinorDest, &Visitor_RatingGap,          &Visitor_WontPay,
                                  &Visitor_VisitObject,  &Thing_PlaceAt,         &Object_Reopen,
                                  &MapCell_StartEffect,
                                  reinterpret_cast<u32(__cdecl *)(s32, s32)>((uintptr_t)0x006e7074),
                                  &Things_HeadForNearestFlag40, &Things_NearestFlag80Close,
                                  [](u8 *, void *, u32 kind, u32 cell, s32 radius) -> s32 { return Things_AnyOfKindAround(kind, cell, radius); },
                                  &Staff_NearestInList1da740, &Person_JumpScript210, &Person_SetRandomDest, &Visitor_ChooseMajorDest,
                                  &MessageCentre_Post, reinterpret_cast<u32(__fastcall *)(u8 *)>(&RideUpkeep_Total), &Money_CreditTakings};
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

void __fastcall Visitor_StartEmote(u8 *visitor, void *, u32 mode) {
    u32 x, y, z, r15;   // (read back by nothing)
    SpriteScript_GetPosition(at<s32>(visitor, 0xc), &x, &y, &z, &r15);
    if (mode == 4) {
        u16 handle;
        if ((*Thing_GetHandle(visitor, nullptr, &handle) & 0xf) == 0) g_env->playSound(visitor, nullptr, global<u32>(0x00803a24), 0x7e);
    }
    Set4_4217f0(visitor + 0xc, nullptr, mode);
    at<u32>(visitor, 0x208) = at<u32>(world(), 0x1da70c);
    at<u32>(visitor, 0x224) = at<u32>(visitor, 0x220);
    g_env->setState(visitor, nullptr, 8);
}

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

void __fastcall Visitor_StepWalkToQueue(u8 *visitor) {
    u8 *const object = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(visitor, 0x1dc));
    u8 *const ring = visitor + 0x30;
    const u32 walked = g_env->walkNav(visitor);
    if (walked != 0) {
        if (walked == 2) {
            Ring_PushUnique(ring, nullptr, 3, at<u16>(visitor, 0x1dc));
        } else if (Get1da710_51a280(world()) == 0) {
            if (++at<u8>(visitor, 0x2c) > 0xb) {
                at<u8>(visitor, 0x2c) = 0;
                g_env->chooseMinorDest(visitor);
            }
            return;
        }
        Visitor_Lower19c(visitor, nullptr, 2);
        at<u16>(visitor, 0x1dc) = 0;
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    if (at<u16>(visitor, 0x1dc) == 0) {
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    u16 back;
    if (*g_env->backOfQueueTo(object, nullptr, &back) != ownCell(visitor)) {
        g_env->backOfQueueTo(object, nullptr, &back);
        if (back != 0 && g_env->setDestinationCell(visitor, nullptr, back) != 0) return;
        Ring_PushUnique(ring, nullptr, 0x16, 0);
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    if (Object_QueueHasRoom(object) == 0) {
        Ring_PushUnique(ring, nullptr, 0x15, 0);
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    const s32 gap = g_env->ratingGap(visitor, nullptr, object);
    if (Object_Rating(object, nullptr, 1) != 0 && (gap < 0 ? -gap : gap) >= 0x2d) {
        if (gap < 0) {
            Ring_PushUnique(ring, nullptr, 5, at<u16>(visitor, 0x1dc));
            Thoughts_Show(ring, nullptr, 0xc, 0);
        } else {
            Ring_PushUnique(ring, nullptr, 4, at<u16>(visitor, 0x1dc));
            Thoughts_Show(ring, nullptr, 0xf, 0);
        }
        Visitor_PushPreviousRide(visitor, nullptr, at<u16>(visitor, 0x1dc));
        at<u16>(visitor, 0x1dc) = 0;
        at<u32>(visitor, 0x1fc) = 0;
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    if (Object_QueueFull(object) != 0) {
        Ring_PushUnique(ring, nullptr, 0x15, at<u16>(visitor, 0x1dc));
        Visitor_PushPreviousRide(visitor, nullptr, at<u16>(visitor, 0x1dc));
        Thoughts_Show(ring, nullptr, 0x10, 0);
        at<u16>(visitor, 0x1dc) = 0;
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    at<u32>(visitor, 0x20c) = at<u32>(visitor, 0x19c);
    Object_QueueAddPerson(object, nullptr, handleOf(visitor));
    if (g_env->findQueueDestination(visitor) != 0) return;
    Object_QueueRemovePerson(object, nullptr, handleOf(visitor));
    Visitor_SendAway(visitor);
    g_env->setState(visitor, nullptr, 6);
}

u32 __fastcall Visitor_ArriveAtObject(u8 *visitor) {
    u8 *const object = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(visitor, 0x1dc));
    u16 x, y;
    Object_CellStandPoint(object, nullptr, 1, reinterpret_cast<u8 *>(&x), reinterpret_cast<u8 *>(&y));
    g_env->placeAt(visitor, nullptr, x, y);
    const u16 exit = at<u16>(object, 0x38);
    const u8 *const cell = map() + ((u32)exit - 1) * 0x44;
    const u32 direction = exit == at<u16>(object, 0x36) ? Nibble_Scale(MapCell_Direction(cell)) : MapCell_Direction(cell);
    const u16 next = *reinterpret_cast<const u16 *>(Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction));
    if (!MapCell_IsType3Or9(map() + ((u32)next - 1) * 0x44) && g_env->setDestinationCell(visitor, nullptr, next)) {
        g_env->visitObject(visitor, nullptr, object);
        g_env->setState(visitor, nullptr, 0xf);
        return 1;
    }
    g_env->reopen(object);
    return 0;
}

void __fastcall Visitor_StepInQueue(u8 *visitor) {
    const u32 walked = g_env->walkNav(visitor);
    if (walked != 0 && walked != 2) return;
    u8 *const object = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(visitor, 0x1dc));
    if (g_env->wontPay(visitor, nullptr, object) != 0) {
        u8 *const ring = visitor + 0x30;
        Thoughts_Show(ring, nullptr, 6, 0);
        Ring_PushUnique(ring, nullptr, 0xa, handleOf(object));
        Visitor_Lower19c(visitor, nullptr, 1);
        Object_AddWalkAway(object);
        Obj_ReleaseRideElement0(object, nullptr, handleOf(visitor));
        Object_QueueRemovePerson(object, nullptr, handleOf(visitor));
        Visitor_SendAway(visitor);
        at<u16>(visitor, 0x1dc) = 0;
        g_env->setState(visitor, nullptr, 6);
        return;
    }
    if (Obj_ClaimRideElement0(object, nullptr, handleOf(visitor)) != 0) {
        g_env->setState(visitor, nullptr, 0xe);
        return;
    }
    if (g_env->findQueueDestination(visitor) != 0) return;
    Object_QueueRemovePerson(object, nullptr, handleOf(visitor));
    Visitor_SendAway(visitor);
    g_env->setState(visitor, nullptr, 6);
}

void __fastcall Visitor_StepIdle(u8 *visitor) {
    u8 *const ring = visitor + 0x30;
    const u32 own = ownCell(visitor);
    u8 *const mapCell = global<u8 *>(0x008023a0) + own * 0x44 - 0x44;
    const auto clock = []() -> u32 & { return at<u32>(world(), 0x1da70c); };
    const auto table = [](u16 h) { return global<u8 *>(0x007cfb90 + (u32)h * 20); };
    // The differences of the cells less one, column and row, from the visitor's to the other's.
    const auto delta = [&](const u8 *other, s32 &dx, s32 &dy) {
        const u32 cell = (u32)ownCell(other) - 1, mine = own - 1;
        dx = (s32)(cell & 0x7f) - (s32)(mine & 0x7f);
        dy = (s32)(cell >> 7) - (s32)(mine >> 7);
    };
    const u32 rnd = (u32)World_Rand(world());
    if (clock() - at<u32>(visitor, 0x208) > 0x64 && (u8)(long long)at<float>(visitor, 0x19c) > 0x50) {
        Visitor_StartEmote(visitor, nullptr, 5);
        return;
    }
    if ((u8)(long long)at<float>(visitor, 0x1b0) == 0x64 && rnd % 3 == 0) {
        Visitor_StartEmote(visitor, nullptr, 7);
        g_env->cellEffect(mapCell, nullptr, 7);
        Ring_PushUnique(ring, nullptr, 0x12, 0);
        g_env->playSound(visitor, nullptr, global<u32>(0x00803a24), 0xcc);
        at<u32>(visitor, 0x1b0) = 0;
        return;
    }
    if ((u8)(long long)at<float>(visitor, 0x1b4) >= 0x5a) {
        const u32 state = at<u32>(visitor, 0x220);
        if (state == 6 || state == 7 || state == 0xa) {
            u16 h;
            g_env->headForNearestFlag40(visitor, nullptr, &h, 3);
            if (h != 0) {
                at<u16>(visitor, 0x1dc) = h;
                g_env->setState(visitor, nullptr, 9);
                return;
            }
            const u32 kind = (u32)World_Rand(world()) % (global<u32>(0x0070048c) - 1) + 1;
            g_env->cellEffect(mapCell, nullptr, kind);
            at<u32>(visitor, 0x1b4) = 0;
        }
    }
    if ((u8)(long long)at<float>(visitor, 0x19c) == 0 || at<u32>(visitor, 0x1bc) == 0 || Get1da710_51a280(world()) != 0) {
        Visitor_Lower19c(visitor, nullptr, 2);
        Game_GetRideElement1(world());   // (its answer not read)
        u16 out, cells[2];
        cells[0] = *Map_CrossingParkSidePos(map(), nullptr, &out, 0);
        cells[1] = *Map_CrossingParkSidePos(map(), nullptr, &out, 1);
        if (rnd & 1) {
            const u16 first = cells[0];
            cells[0] = cells[1];
            cells[1] = first;
        }
        for (int round = 0; round < 2; round++) {
            for (int i = 0; i < 2; i++) {
                if (g_env->setDestinationCell(visitor, nullptr, cells[i]) != 0) {
                    g_env->setState(visitor, nullptr, 0x12);
                    return;
                }
            }
            at<u32>(visitor, 0x188) = 1;
        }
        at<u32>(visitor, 0x188) = 0;
    }
    s32 dx, dy;
    {
        u16 h;
        g_env->nearestFlag80Close(visitor, nullptr, &h);
        if (h != 0) {
            delta(table(h), dx, dy);
            if (dx + dy != 0) {
                at<u32>(visitor, 0x1c) = g_env->angle(-dx, dy) & 0xffff;
                return;
            }
        }
    }
    if (g_env->anyOfKindAround(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(world())), nullptr, 6, own, 1) != 0) {
        u16 out;
        u8 *const staff = table(*g_env->nearestStaff(visitor, nullptr, &out));
        if (at<u32>(staff, 0x19c) == 0xe) {
            delta(staff, dx, dy);
            if (dx + dy != 0) {
                Ring_PushUnique(ring, nullptr, 0xe, handleOf(staff));
                at<u32>(visitor, 0x1c) = g_env->angle(-dx, dy) & 0xffff;
                return;
            }
        }
    }
    const u8 chance = at<u8>(visitor, 0x1c0);
    if ((u8)(long long)at<float>(visitor, 0x19c) < 0xf && rnd % 0x65 < chance) {
        u8 *const cell = world() + 0x294 + own * 0x44;
        bool done = false;
        if (chance == 0x64) {
            if (rnd % 100 < global<u32>(0x007850e0)) {
                g_env->cellEffect(cell, nullptr, 8);
                g_env->playSound(visitor, nullptr, global<u32>(0x00803a24), 0xcf);
                done = true;
            }
        } else if (chance == 0x65) {
            g_env->cellEffect(cell, nullptr, rnd % 5 + 1);
            done = true;
        } else if (chance == 0x66) {
            for (u16 h = at<u16>(world(), own * 0x44 + 0x2b8); h != 0;) {
                u8 *const thing = table(h);
                if (h != handleOf(visitor) && at<u8>(thing, 2) == 1 && at<u32>(thing, 0x210) != 0) {
                    g_env->jumpScript(thing);
                    done = true;
                    break;
                }
                h = at<u16>(thing, 0xa);
            }
        }
        if (done) {
            Ring_PushUnique(ring, nullptr, 0xf, 0);
            const u16 me = handleOf(visitor);
            alignas(4) u8 msg[0x10] = {};
            at<u32>(msg, 0) = 0x007007a8;
            at<u16>(msg, 8) = me;
            at<u32>(msg, 0xc) = me % 3u;
            g_env->post(MessageCentre_Instance(), nullptr, msg);
            const X87Reg mood = X87Reg(at<float>(visitor, 0x19c)) - gVisitor_StepIdle_700794;
            if (mood.v > gVisitor_StepIdle_700798) at<u32>(visitor, 0x19c) = 0x42c80000;
            else if (!(mood.v >= gVisitor_StepIdle_70079c)) at<u32>(visitor, 0x19c) = 0;
            else at<float>(visitor, 0x19c) = (float)mood;
        }
    }
    const u32 which = rnd % 3;
    if (which == 1) {
        if (g_env->setRandomDest(visitor) != 0) {
            g_env->setState(visitor, nullptr, 7);
            return;
        }
        at<u32>(visitor, 0x1fc) = clock();
    } else if (which == 0 && clock() > at<u32>(visitor, 0x1fc) + 0x1e) {
        g_env->chooseMajorDest(visitor);
        if (at<u16>(visitor, 0x1dc) != 0) {
            Ring_PushUnique(ring, nullptr, 2, at<u16>(visitor, 0x1dc));
            g_env->setState(visitor, nullptr, 0xa);
            return;
        }
        Ring_PushUnique(ring, nullptr, 1, 0);
        Visitor_StartEmote(visitor, nullptr, 4);
        Visitor_Lower19c(visitor, nullptr, 0);
        at<u32>(visitor, 0x1fc) = clock();
    }
}

void __fastcall Visitor_StepQueueWait(u8 *visitor) {
    u8 *const object = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(visitor, 0x1dc));
    u8 *const ring = visitor + 0x30;
    const auto leave = [visitor] {
        Object_QueueRemovePerson(global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(visitor, 0x1dc)), nullptr, handleOf(visitor));
        Visitor_SendAway(visitor);
        g_env->setState(visitor, nullptr, 6);
    };
    const u32 place = at<u8>(visitor, 0x1f1);
    if (!place && at<u32>(visitor, 0x1f8)) {
        if (!Obj_Word6cEquals(object, nullptr, handleOf(visitor))) return;
        at<u32>(visitor, 0x1f8) = 0;
        u16 x, y;
        Object_CellStandPoint(object, nullptr, 0, reinterpret_cast<u8 *>(&x), reinterpret_cast<u8 *>(&y));
        if (g_env->setDestination(visitor, nullptr, x, y)) {
            g_env->setState(visitor, nullptr, 0xd);
            return;
        }
        Obj_ReleaseRideElement0(object, nullptr, handleOf(visitor));
        leave();
        return;
    }
    if (Object_InPoorCondition(object)) {
        Thoughts_Show(ring, nullptr, 0xe, 0);
        leave();
        return;
    }
    const s32 position = Object_QueuePosition(object, nullptr, handleOf(visitor));
    if (position == -1) {
        leave();
        return;
    }
    if (place == (u32)position) {
        if (place > Object_FillRate(object)) {
            Thoughts_Show(ring, nullptr, 0x10, 0);
            Ring_PushUnique(ring, nullptr, 0x15, at<u16>(visitor, 0x1dc));
            leave();
            return;
        }
        if ((Object_DefIsKind3(object) && !Object_LinkOtherOpen(object)) || (at<u32>(Object_GetDef(object), 0x9c) == 1 && at<u32>(object, 0x2c) == 0)) {
            Thoughts_Show(ring, nullptr, 0xd, 0);
            leave();
            return;
        }
    } else if (at<u32>(visitor, 0x1f4) && place - (u32)position <= 2) {
        --at<u32>(visitor, 0x1f4);
    } else if (!Is19c_Eq1_4e0370(object) && !g_env->findQueueDestination(visitor)) {
        leave();
        return;
    }
    u8 *const world = global<u8 *>(0x0080239c);
    const u32 now = at<u32>(world, 0x1da70c);
    if (now - at<u32>(visitor, 0x208) > 0x1e) {
        const u8 mood = (u8)(long long)at<float>(visitor, 0x19c);   // (__ftol, the low byte)
        if (mood > 0x50) {
            Visitor_StartEmote(visitor, nullptr, 5);
        } else if (mood >= 0x14) {
            if ((u8)(long long)at<float>(visitor, 0x1ac) > 0x50) {
                Thoughts_Show(ring, nullptr, 4, 0);
                if (!(at<u8>(object, 0x32) & 1)) leave();
            }
        } else if (mood >= 0xa) {
            Visitor_StartEmote(visitor, nullptr, 4);
        } else {
            Thoughts_Show(ring, nullptr, 0xb, 0);
            leave();
        }
        return;
    }
    if (now > at<u32>(visitor, 0x1fc) + 0x64) {
        Ring_PushUnique(ring, nullptr, 7, at<u16>(visitor, 0x1dc));
        Thoughts_Show(ring, nullptr, 0xc, 0);
        leave();
        return;
    }
    if ((u32)World_Rand(world) % 10 != 0) return;
    const s32 heading = at<s32>(visitor, 0x1c);
    s32 turned = (s32)((u32)World_Rand(world) % 800) + heading - 400;
    if (turned < 0) turned += 0x800;
    else if (turned >= 0x800) turned -= 0x800;
    at<s32>(visitor, 0x1c) = turned;
}

void __fastcall Visitor_ShowMoodThought(u8 *visitor) {
    const u8 hunger = (u8)(long long)at<float>(visitor, 0x1a8);
    u32 thought;
    if (hunger > 0x5a && (u8)(long long)at<float>(visitor, 0x1a4) > 0x5a) {
        thought = 3;
    } else if (hunger > 99) {
        thought = 1;
    } else if ((u8)(long long)at<float>(visitor, 0x1a4) > 99) {
        thought = 2;
    } else if ((u8)(long long)at<float>(visitor, 0x1ac) > 99) {
        thought = 4;
    } else if ((u8)(long long)at<float>(visitor, 0x1b0) > 99) {
        thought = 0xe;
    } else {
        u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(world()));
        u8 *const cell = ParkAnalyser_Cell(analyser, nullptr, ownCell(visitor));
        const float held = (float)(s32)AnalyserCell_CountWith2c(cell, nullptr, 4);
        const X87Reg share = X87Reg(held) / X87Reg((double)(s32)AnalyserCell_CountType1Or3Or9(cell, nullptr, 4));
        if (share.v > (double)0.2f) {
            thought = 7;
        } else {
            const u8 mood = (u8)(long long)at<float>(visitor, 0x19c);
            if (mood > 99) thought = 9;
            else if (mood < 10) thought = 0xb;
            else return;
        }
    }
    // (the original's table also has 0x7e for 0xc, 0x7f for 0xd and 0x85 for 0xf: not reached)
    if (!(handleOf(visitor) & 0xf) && thought == 4) g_env->playSound(visitor, nullptr, global<u32>(0x00803a24), 0x85);
    Thoughts_Show(visitor + 0x30, nullptr, thought, 0);
}

namespace {
void addClamped(float &stat, X87Reg sum) {
    if (sum.v > 100.0) stat = 100.0f;
    else if (!(sum.v >= 0.0)) stat = 0.0f;
    else stat = (float)sum;
}
}  // namespace

void __fastcall Visitor_Update(u8 *visitor) {
    Person_UpdateSpeed(visitor);
    const u32 now = at<u32>(world(), 0x1da70c);
    if (((handleOf(visitor) ^ now) & 3) == 0) {
        at<u32>(visitor, 0x1bc)--;
        u16 cell;
        Cell_MakeHandle(&cell, visitor[5], visitor[7]);
        const u8 *const effect = Map_RECell(global<u8 *>(0x008023a0), nullptr, cell);
        float &mood = at<float>(visitor, 0x19c), &thirst = at<float>(visitor, 0x1a4), &hunger = at<float>(visitor, 0x1a8), &tired = at<float>(visitor, 0x1ac),
              &sick = at<float>(visitor, 0x1b0);
        addClamped(mood, X87Reg((double)at<s16>(effect, 0)) + X87Reg(mood));
        addClamped(sick, X87Reg((double)at<s16>(effect, 2)) + X87Reg(sick));
        addClamped(hunger, X87Reg((double)at<s16>(effect, 4)) + X87Reg(hunger));
        if (!(at<u8>(world(), 0x1da70c) & 0xf)) {
            addClamped(tired, X87Reg(tired) + X87Reg(1.0));
            addClamped(hunger, X87Reg(hunger) + X87Reg(2.0));
            addClamped(thirst, X87Reg(thirst) + X87Reg(2.0));
        }
        if ((u8)(long long)sick == 0x64) Stat100_Add(&mood, nullptr, -1.0f);
        if ((u8)(long long)hunger == 0x64) Stat100_Add(&mood, nullptr, -1.0f);
        if ((u8)(long long)thirst == 0x64) Stat100_Add(&mood, nullptr, -1.0f);
        if ((u8)(long long)tired == 0x64) Stat100_Add(&mood, nullptr, -1.0f);
        at<u16>(visitor, 0xc2) = global<u16>((u8)(long long)tired > 0x50 ? 0x0075c7f2 : 0x0075c7f0);
        const u32 state = at<u32>(visitor, 0x220);
        if (state != 0x10 && state != 0x11 && AnalyserCell_TypePlain(visitor)) {
            if ((u32)World_Rand(world()) % 10 == 0) Visitor_ShowMoodThought(visitor);
            const u32 left = at<u32>(visitor, 0x214);
            if (left != 0) {
                at<u32>(visitor, 0x214) = left - 1;
                if (left == 1) g_env->jumpScript(visitor);
            }
        }
        Person_KillScript84After(visitor + 0x30);
    }
    u32 doing = at<u32>(visitor, 0x220);
    if (doing == 8) doing = at<u32>(visitor, 0x224);
    if ((s32)doing < 0xb || (s32)doing > 0xe) {   // (and an empty debug print)
        at<u16>(visitor, 0x228) = 0;
        at<u16>(visitor, 0x22a) = 0;
    }
    Visitor_PreviousRideTick(visitor);
}

u32 __fastcall Visitor_JudgeEntryFee() {
    u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(world()));
    const u32 income = g_env->analyserIncome(analyser);
    const u32 base = global<u32>(0x0078507c);
    const u32 divisor = (u32)World_Rand(world()) % ((base >> 1) + 1) + base;
    const u32 price = at<u32>(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(world())), 0x118);
    const u32 least = global<u32>(0x00785080);
    const u32 worth = income / divisor + least;
    if (worth <= least && price <= least) return 3;
    const X87Reg w((double)worth), p((double)price);
    u32 answer;
    u32 value;
    if (!(p.v >= (X87Reg(global<float>(0x00785088)) * w).v)) {
        answer = p.v > (X87Reg(global<float>(0x00785084)) * w).v ? 3 : 2;
        value = global<u8>(answer == 3 ? 0x007850d8 : 0x007850d4);
    } else {
        answer = !(p.v >= (X87Reg(global<float>(0x0078508c)) * w).v) ? 1 : 0;
        value = global<u8>(answer == 1 ? 0x007850d0 : 0x007850cc);
    }
    ParkAnalyser_Record(analyser, nullptr, 0, value);
    return answer;
}

void __fastcall Visitor_StepAtBooth(u8 *visitor) {
    if (at<u32>(visitor, 0x200) != 0) {
        at<u32>(visitor, 0x200)--;
        return;
    }
    const VisitorStepsEnv *const E = g_env;
    u8 *const ring = visitor + 0x30;
    switch (Visitor_JudgeEntryFee()) {
    case 0: {
        at<u32>(visitor, 0x188) = 1;
        const u8 *const m = map();
        u16 cell;
        const u16 *const side = Map_CrossingParkSidePos(m, nullptr, &cell, (u32)World_Rand(world()) & 1);
        E->setDestinationCell(visitor, nullptr, *side);
        Thoughts_Show(ring, nullptr, 6, 0);
        break;
    }
    case 1:
        at<u32>(visitor, 0x200) = (u32)World_Rand(world()) % 0x32 + 0x32;
        Thoughts_Show(ring, nullptr, 6, 0);
        Visitor_Lower19c(visitor, nullptr, 1);
        if ((u8)(long long)at<float>(visitor, 0x19c) != 0) return;
        break;
    case 2:
        Visitor_Raise19c(visitor, nullptr, 1);
        [[fallthrough]];
    case 3:
        E->creditTakings(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(world())));
        at<u32>(visitor, 0x204) = 1;
        E->setState(visitor, nullptr, 3);
        return;
    default:
        return;
    }
    E->setState(visitor, nullptr, 0x12);
    at<u32>(visitor, 0x1bc) = 0;
}

void __fastcall Visitor_StepComingOff(u8 *visitor) {
    const VisitorStepsEnv *const E = g_env;
    const u32 walk = E->walkNav(visitor);
    const u16 object = at<u16>(visitor, 0x1dc);
    if (walk == 0 && object != 0) {
        Rides_SetElement(Rides_FindById(at<s32>(global<u8 *>(0x007cfb90 + 20u * object), 0x24)), 0, 1);
    } else if (walk == 0 || walk == 2) {
        const u32 rnd = (u32)World_Rand(world()) % (global<u32>(0x0070048c) - 1);
        E->cellEffect(const_cast<u8 *>(map()) + ownCell(visitor) * 0x44u - 0x44u, nullptr, rnd + 1);
    } else {
        return;
    }
    at<u32>(visitor, 0x1b4) = 0;
    E->setState(visitor, nullptr, 6);
}

void __fastcall Visitor_PostUpdate(u8 *visitor) {
    const VisitorStepsEnv *const E = g_env;
    switch (at<u32>(visitor, 0x220)) {
    case 0:
        if (E->walkNav(visitor) == 0) {   // (2: a debug print)
            at<u32>(visitor, 0x1c) = 0x400;
            E->setState(visitor, nullptr, 1);
        }
        break;
    case 1: Visitor_GoToTicketBooth(visitor); break;
    case 2: Visitor_StepWalkingIn(visitor); break;
    case 3: Visitor_StepArriving(visitor); break;
    case 4: Visitor_StepAtBooth(visitor); break;
    case 5: Visitor_StepEntering(visitor); break;
    case 6: Visitor_StepIdle(visitor); break;
    case 7: {
        const u32 walk = E->walkNav(visitor);
        if (walk != 0 && walk != 2) break;
        E->setState(visitor, nullptr, (World_Rand(world()) & 3) == 0 && E->setRandomDest(visitor) ? 7 : 6);
        break;
    }
    case 8:
        if (at<u32>(world(), 0x1da70c) > at<u32>(visitor, 0x208) + 0xa) Visitor_ReenterState(visitor);
        break;
    case 9: Visitor_StepComingOff(visitor); break;
    case 0xa: Visitor_StepWalkToQueue(visitor); break;
    case 0xb: Visitor_StepQueueWait(visitor); break;
    case 0xc: {
        const u32 walk = E->walkNav(visitor);
        if (walk == 0 || walk == 2) E->setState(visitor, nullptr, 0xb);
        break;
    }
    case 0xd: Visitor_StepInQueue(visitor); break;
    case 0xe: {
        u8 *const object = Thing_ByHandle(at<u16>(visitor, 0x1dc));
        if (Obj_RideElement0Differs(object)) {
            Object_QueueRemovePerson(object, nullptr, handleOf(visitor));
            E->setState(visitor, nullptr, 0x10);
        }
        break;
    }
    case 0xf: Visitor_StepLeavingRide(visitor); break;
    case 0x10: Thing_ByHandle(at<u16>(visitor, 0x1dc)); break;   // (sic: the answer dropped)
    case 0x11: {
        for (u16 handle = at<u16>(world(), 0x1da744); handle != 0;) {
            u8 *const guard = global<u8 *>(0x007cfb90 + 20u * handle);
            if (at<u16>(guard, 0x212) == handleOf(visitor) && Guard_IsState12(guard)) return;
            handle = at<u16>(guard, 0x210);
        }
        E->thingDelete(visitor);
        break;
    }
    case 0x12: Visitor_StepLeavingPark(visitor); break;
    case 0x13: Visitor_GoToBusStop(visitor); break;
    case 0x14:
        if (E->walkNav(visitor) == 0) {   // (2: a debug print)
            at<u32>(visitor, 0x1c) = World_ArrivalVehicleNotSize1(world()) ? 0x7ff : 0x400;
            E->setState(visitor, nullptr, 0x15);
        }
        break;
    case 0x15: Visitor_StepAtBusStop(visitor); break;
    default: break;
    }
}
