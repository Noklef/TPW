// The tag system's CTagTable: reimplementation of 0x50af40.
#include "tagsystem.h"

#include <cstdint>

#include "messagecentre.h"
#include "things.h"

namespace {

constexpr u32 kTable = 0x75eca8, kTableEnd = 0x761c68;

u32 *Record(u32 address) { return reinterpret_cast<u32 *>((uintptr_t)address); }

}  // namespace

extern "C" u32 *__fastcall TagTable_Find(void *, void *, u32 id) {
    if (id == 0x17f || id == 0x17e) return nullptr;
    u32 *direct = Record(kTable + (id << 5));
    if (*direct == id) return direct;
    for (u32 address = kTable; (s32)address < (s32)kTableEnd; address += 0x20)
        if (*Record(address) == id) return Record(address);
    return nullptr;
}

extern "C" u8 *__fastcall TagSystem_Ctor(u8 *tags) {
    Thing_Construct(tags, nullptr, 1, 0xc);
    static const s32 kSlots[] = {7, 0xc, 0x14, 0x18, 0x19, 0x13};
    for (s32 slot : kSlots)
        MessageCentre_AddThing(MessageCentre_Instance(), nullptr, tags, slot);
    return tags;
}
