#include "small61.h"

#include <cstdint>

#include "../sprite/script.h"
#include "../sprite/sprite.h"
#include "../world/nav.h"
#include "../world/random.h"
#include "../world/things.h"
#include "small58.h"
#include "vwrap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Thing_PlaceAt(u8 *thing, void *, u32 x, u32 y) {
    Nav_SetPosVel(thing + 0xd4, nullptr, (u32)((s32)(s16)x << 8), (u32)((s32)(s16)y << 8), 0, 0);
    Nav_GetPos(thing + 0xd4, nullptr, &at<u32>(thing, 0x190), &at<u32>(thing, 0x194));
    Thing_SetPos(thing, nullptr, x, y);
}

s32 __cdecl Slot_ReadIntoFree(u8 *stream) {
    if (global<u32>(0x00816d40) == 0)
        return -1;
    s32 index = 0;
    for (u32 slot = 0x00804488; slot != 0 && global<u8>(slot) != 0;) {
        slot += 0x140;
        index++;
        if ((s32)slot > 0x80c7c7)
            return -1;
    }
    Object_CallSlot8Product(0x00804370 + (u32)index * 0x140, 0x140, 1, stream);
    return index;
}

u32 __fastcall Thing_SpriteChoice(u8 *thing) {
    if (SpriteFolder_Count(3) != 0) {
        const u32 flags = at<u32>(thing, 0x20);
        return at<u32>(thing, 0x24) == 2 ? flags | 0x80000000u : flags;
    }
    u16 handle;
    World_SetSeed(global<u8 *>(0x0080239c), nullptr, *Thing_GetHandle(thing, nullptr, &handle));
    return SpriteFolder_RandomSprite(1);
}

}  // extern "C"
