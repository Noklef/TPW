#include "objectmsg.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "analysercell.h"
#include "mapcell.h"
#include "objectdefs.h"
#include "person.h"
#include "ridelist.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The thing a handle names (the debug assert on the table is left out).
u8 *ThingOf(u32 handle) { return *reinterpret_cast<u8 **>(0x007cfb90 + 20 * (handle & 0xffff)); }

// One ring: the temp dword at base, the data after it, then the index, the length and the wrapped byte.
void Advance(u8 *object, u32 base) {
    const s32 length = at<s32>(object, base + 0x80);
    const u32 temp = at<u32>(object, base);
    const s32 next = at<s32>(object, base + 0x7c) + 1;
    at<s32>(object, base + 0x7c) = next;
    if (next >= length) {
        at<u32>(object, base + 0x7c) = 0;
        at<u8>(object, base + 0x84) = 1;
    }
    at<u32>(object, base + 4 * at<u32>(object, base + 0x7c) + 4) = temp;
    at<u32>(object, base) = 0;
}

}  // namespace

extern "C" {

u32 __fastcall Object_HandleDayMessage(u8 *object, void *, u8 *message) {
    const u32 type = vcall<u32>(message, 0);
    if (type == 0xb) {
        static const u32 bases[6] = {0x1a8, 0x230, 0x2b8, 0x70, 0xf8, 0x340};
        for (u32 base : bases)
            Advance(object, base);
        return 0;
    }
    if (type == 0x1b && at<u16>(message, 8) == at<u16>(object, 0x5e)) {  // (the Debug_Printf is left out)
        at<u16>(object, 0x5e) = 0;
        at<u32>(object, 0x60) = 0;
        return 0;
    }
    return type;
}

void __fastcall Object_QueueAddPerson(u8 *object, void *, u32 person) {
    u16 tail = at<u16>(object, 0x3c);
    if (tail == 0) {
        at<u16>(object, 0x3c) = (u16)person;
    } else {
        u16 current = tail;
        while (at<u16>(ThingOf(current), 0x228) != 0) {
            tail = at<u16>(ThingOf(current), 0x228);
            current = tail;
        }
        at<u16>(ThingOf(current), 0x228) = (u16)person;
    }
    at<u16>(ThingOf(person), 0x22a) = tail;
    at<u16>(ThingOf(person), 0x228) = 0;
}

void __fastcall Object_QueueRemovePerson(u8 *object, void *, u32 person) {
    const u32 handle = person & 0xffff;
    if (Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 0) == handle)
        Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 0, 0);
    const u16 ahead = at<u16>(ThingOf(handle), 0x22a);
    const u16 behind = at<u16>(ThingOf(handle), 0x228);
    if (ahead == 0)
        at<u16>(object, 0x3c) = behind;
    else
        at<u16>(ThingOf(ahead), 0x228) = behind;
    if (behind != 0)
        at<u16>(ThingOf(behind), 0x22a) = ahead;
    at<u16>(ThingOf(handle), 0x22a) = 0;
    at<u16>(ThingOf(handle), 0x228) = 0;
}

s32 __fastcall Object_QueuePosition(u8 *object, void *, u32 person) {
    u16 current = at<u16>(object, 0x3c);
    const u16 wanted = (u16)person;
    s32 position = 0;
    while (current != wanted) {
        if (current == 0) return -1;  // (the original prints that the person could not be found)
        ++position;
        const u8 *thing = ThingOf(current);
        if (thing)
            current = Visitor_IsState11To14(thing) ? at<u16>(thing, 0x228) : (u16)0;
    }
    return position;
}

void __fastcall Object_BackOfQueue(u8 *object, void *, u16 *out) {
    if (at<u16>(object, 0x3a) != 0) {
        *out = at<u16>(object, 0x3a);
        return;
    }
    u16 cell = 0;
    Object_NeighbourCellId(object, nullptr, &cell);
    at<u32>(object, 0x40) = 0;
    at<u16>(object, 0x3a) = cell;
    s32 budget = 1000, left;
    for (;;) {
        left = budget;
        if (cell == 0)
            break;
        left = budget - 1;
        if (!(0 < budget))
            break;
        at<u16>(object, 0x3a) = cell;
        u16 next;
        cell = *Map_FindFacingNeighbour(&next, cell);
        at<u32>(object, 0x40) = at<u32>(object, 0x40) + 1;
        budget = left;
    }
    if (0 < left) {
        *out = at<u16>(object, 0x3a);
        return;
    }
    *out = 0;  // (the message "GetBackOfQueue crashed" is left out)
}

u16 *__fastcall Object_BackOfQueueTo(u8 *object, void *, u16 *out) {
    Object_BackOfQueue(object, nullptr, out);
    return out;
}

s32 __fastcall Object_QueuePositionOfNone(u8 *object) {
    return Object_QueuePosition(object, nullptr, (u32)(uintptr_t)object & 0xffff0000u);
}

u32 __fastcall Object_QueueHasRoom(u8 *object) {
    const u32 position = (u32)Object_QueuePosition(object, nullptr, 0);
    return position < (at<u32>(object, 0x40) << 2) ? 1 : 0;
}

u32 __fastcall Object_QueueOpen(u8 *object) {
    if (!(at<u8>(object, 0x32) & 4)) return 0;
    const s32 state = at<s32>(object, 0x19c);
    if (state == 1 || at<u32>(object, 0x68) == 0 || state == 4) return 0;
    u16 back;
    Object_BackOfQueue(object, nullptr, &back);
    if (back == 0) return 0;
    const u32 position = (u32)Object_QueuePosition(object, nullptr, 0);
    if (!(position < (at<u32>(object, 0x40) << 2))) return 0;
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    if (at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 3 && !Object_LinkOtherOpen(object)) return 0;
    if (at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 1 && at<u32>(object, 0x2c) == 0) return 0;
    if (at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 2 && at<u32>(object, 0x2c) == 0) return 0;
    return 1;
}

u32 __fastcall Object_EntranceOnPath(u8 *object) {
    u8 *const cells = *reinterpret_cast<u8 **>(0x008023a0);
    if (at<u8>(object, 0x32) & 8) {
        u16 back;
        Object_BackOfQueue(object, nullptr, &back);
        if (back == 0) return 0;
        const u8 *cell = cells + (u32)back * 0x44 - 0x44;
        const u8 neighbours = MapCell_Neighbours(cell);
        return neighbours != MapCell_Direction(cell) ? 1 : 0;
    }
    u8 *const cell = cells + (u32)at<u16>(object, 0x36) * 0x44 - 0x44;
    u8 bit = 0;  // the original's is a stale stack byte for an angle that is not a quarter turn
    switch (at<s32>(object, 0x10)) {
    case 0: bit = 0x10; break;
    case 0x5a: bit = 4; break;
    case 0xb4: bit = 1; break;
    case 0x10e: bit = 0x40; break;
    default: break;
    }
    const u8 opposite = (u8)Nibble_Scale(bit);
    const u8 *next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, opposite);
    if (!MapCell_IsType1(next)) return 0;  // (sic: a step off the map gives null, which is read)
    if (!(MapCell_Neighbours(next) & bit)) return 0;
    if (!(MapCell_Neighbours(cell) & opposite)) return 0;
    return 1;
}

u32 __fastcall Object_CanOpen(u8 *object) {
    const s32 state = at<s32>(object, 0x19c);
    if (state == 1 || state == 4 || at<u32>(object, 0x64) != 0 || state == 2) return 0;
    if (!Object_EntranceOnPath(object)) return 0;
    const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
    if (at<s32>(def, 0x9c) == 3 && !Object_LinkOtherOpen(object)) return 0;
    return 1;
}
}  // extern "C"
