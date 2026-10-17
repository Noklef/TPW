#include "person.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "nav.h"

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

u32 __fastcall Visitor_IsState11AtQueuePos0(const u8 *visitor) {
    return at<s32>(visitor, 0x220) == 11 && at<u8>(visitor, 0x1f1) == 0;
}

u8 *__fastcall Thoughts_Ctor(u8 *thoughts) {
    at<u32>(thoughts, 0) = 0;
    at<u32>(thoughts, 0x84) = 0;
    at<u32>(thoughts, 0x8c) = 0;
    for (u32 i = 0; i != 32; ++i) {
        at<u16>(thoughts, 4 + i * 4) = 0;
        at<u16>(thoughts, 6 + i * 4) = 0;
    }
    at<u32>(thoughts, 0x88) = 0;
    return thoughts;
}

u32 __fastcall Visitor_IsState11To14(const u8 *visitor) {
    s32 state = at<s32>(visitor, 0x220);
    if (state == 8) state = at<s32>(visitor, 0x224);
    return state >= 11 && state <= 14;
}

u32 __fastcall Person_HasWordInState(const u8 *object, void *, u32 word) {
    if (!Person_HasWord(object, nullptr, word))
        return 0;
    const u8 kind = at<u8>(object, 2);
    if (kind == 5 && at<u32>(object, 0x19c) == 0xb)
        return 1;
    if (kind == 4 && at<u32>(object, 0x19c) == 0xd)
        return 1;
    return 0;
}

void __fastcall Visitor_Set1f8One(u8 *object) { at<u32>(object, 0x1f8) = 1; }

u32 __fastcall Person_HasWord(const u8 *object, void *, u32 word) {
    const u8 kind = at<u8>(object, 2);
    if (kind == 4)
        return at<u16>(object, 0x218) == (u16)word;
    if (kind == 5)
        return at<u16>(object, 0x21a) == (u16)word;
    return 0;
}

u8 *__fastcall Person_BaseCtor(u8 *object, void *, u32) {
    Thoughts_Ctor(object + 0x30);
    Nav_Ctor(object + 0xd4);
    at<u32>(object, 0x1f0) = 0;
    at<u32>(object, 0x1f4) = 0;
    at<u32>(object, 0x1f8) = 0;
    at<u32>(object, 0x1fc) = 0;
    return object;
}

u8 *__fastcall Person_PartsCtor(u8 *object, void *, u32) {
    Thoughts_Ctor(object + 0x30);
    Nav_Ctor(object + 0xd4);
    return object;
}
}
