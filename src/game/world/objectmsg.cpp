#include "objectmsg.h"

#include <cstdint>

#include "../core/vcall.h"
#include "person.h"

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

}  // extern "C"
