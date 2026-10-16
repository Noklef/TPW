#include "person.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 map_id(const u8 *thing) { return (u16)((at<u8>(thing, 7) << 7) + at<u8>(thing, 5) + 1); }

}  // namespace

extern "C" {

u16 *__fastcall Thing_MapId(const u8 *thing, void *, u16 *out) {
    *out = map_id(thing);
    return out;
}

u16 *__fastcall Sprite_RecordMapId(u8 *sprite, void *, u16 *out) {
    const u16 last = at<u16>(sprite, 0xcc);
    at<u16>(sprite, 0xcc) = map_id(sprite);
    *out = last;
    return out;
}

u32 __fastcall Visitor_TimeInPark(const u8 *visitor) {
    const u8 *world = *reinterpret_cast<u8 *const *>(0x0080239c);
    return (at<u32>(world, 0x1da70c) - at<u32>(visitor, 0x1d4)) >> 2;
}

void __fastcall Visitor_PushPreviousRide(u8 *visitor, void *, u32 ride) {
    u16 *const rides = &at<u16>(visitor, 0x1e8);
    for (int i = 3; i != 0; --i) rides[i] = rides[i - 1];
    rides[0] = (u16)ride;
}
}
