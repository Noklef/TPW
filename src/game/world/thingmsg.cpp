#include "thingmsg.h"

#include <cstdint>

#include "../core/initializers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *init(u8 *msg, u32 vtable) {
    at<u32>(msg, 4) = 0;
    at<u32>(msg, 0) = vtable;
    return msg;
}

}  // namespace

extern "C" {

u8 *__fastcall ThingMsg01_Ctor(u8 *msg, void *, u32 a) {
    at<u32>(init(msg, 0x006fd7a8), 8) = a;
    return msg;
}

u8 *__fastcall ThingMsg02_Ctor(u8 *msg, void *, u32 c, u32 b) {
    init(msg, 0x006fd7a0);
    at<u32>(msg, 8) = b;
    at<u32>(msg, 0xc) = c;
    return msg;
}

#define THINGMSG_U16(n, vtable)                               \
    u8 *__fastcall ThingMsg##n##_Ctor(u8 *msg, void *, u16 a) { \
        at<u16>(init(msg, vtable), 8) = a;                    \
        return msg;                                           \
    }
THINGMSG_U16(06, 0x006fd7c0)
THINGMSG_U16(07, 0x006fd7c8)
THINGMSG_U16(08, 0x006fd7d0)
THINGMSG_U16(09, 0x006fd7d8)
THINGMSG_U16(0a, 0x006fd7e0)
THINGMSG_U16(14, 0x006fd818)
THINGMSG_U16(1b, 0x006fd848)
THINGMSG_U16(1c, 0x006fd840)
#undef THINGMSG_U16

u8 *__fastcall ThingMsg0b_Ctor(u8 *msg) { return init(msg, 0x006fd7e8); }
u8 *__fastcall ThingMsg0c_Ctor(u8 *msg) { return init(msg, 0x006fd7f0); }
u8 *__fastcall ThingMsg0d_Ctor(u8 *msg) { return init(msg, 0x006fd7f8); }

u8 *__fastcall ThingMsg11_Ctor(u8 *msg, void *, u16 a, u32 b) {
    init(msg, 0x006fd800);
    at<u16>(msg, 8) = a;
    at<u32>(msg, 0xc) = b;
    return msg;
}

u8 *__fastcall ThingMsg12_Ctor(u8 *msg, void *, u32 a) {
    at<u32>(init(msg, 0x006fd808), 8) = a;
    return msg;
}

u8 *__fastcall ThingMsg13_Ctor(u8 *msg, void *, u32 a) {
    at<u32>(init(msg, 0x006fd810), 8) = a;
    return msg;
}

u8 *__fastcall ThingMsg15_Ctor(u8 *msg, void *, u8 a) {
    at<u8>(init(msg, 0x006fd6c8), 8) = a;
    return msg;
}

u8 *__fastcall ThingMsg17_Ctor(u8 *msg, void *, u32 a, u32 b) {
    init(msg, 0x006fd820);
    at<u32>(msg, 8) = a;
    at<u32>(msg, 0xc) = b;
    return msg;
}

u8 *__fastcall ThingMsg18_Ctor(u8 *msg, void *, u32 a, u16 b, u32 c, u32 d) {
    init(msg, 0x006fd828);
    at<u32>(msg, 8) = a;
    at<u16>(msg, 0xc) = b;
    at<u32>(msg, 0x10) = d;
    at<u32>(msg, 0x14) = c;
    return msg;
}

u8 *__fastcall ThingMsg19_Ctor(u8 *msg, void *, u32 a, u32 b, u8 c, u32 d, u32 e, u32 f, u32 g, u32 h, u16 i,
                               u16 j, u32 k) {
    init(msg, 0x006fd830);
    at<u32>(msg, 8) = a;
    at<u32>(msg, 0xc) = b;
    at<u8>(msg, 0x10) = c;
    at<u32>(msg, 0x14) = d;
    at<u32>(msg, 0x18) = e;
    at<u32>(msg, 0x1c) = f;
    at<u32>(msg, 0x20) = g;
    at<u32>(msg, 0x24) = h;
    at<u16>(msg, 0x28) = i;
    at<u16>(msg, 0x2a) = j;
    at<u32>(msg, 0x2c) = k;
    return msg;
}

u8 *__fastcall ThingMsg1a_Ctor(u8 *msg, void *, u32 a, u32 b, u16 c, u16 d) {
    init(msg, 0x006fd838);
    at<u32>(msg, 8) = a;
    at<u32>(msg, 0xc) = b;
    at<u16>(msg, 0x10) = c;
    at<u16>(msg, 0x12) = d;
    return msg;
}

#define THINGMSG_TYPE(n) \
    u32 __fastcall ThingMsg##n##_Type(const u8 *) { return 0x##n; }
THINGMSG_TYPE(02)
THINGMSG_TYPE(05)
THINGMSG_TYPE(06)
THINGMSG_TYPE(07)
THINGMSG_TYPE(08)
THINGMSG_TYPE(09)
THINGMSG_TYPE(0a)
THINGMSG_TYPE(0b)
THINGMSG_TYPE(0c)
THINGMSG_TYPE(0d)
THINGMSG_TYPE(11)
THINGMSG_TYPE(12)
THINGMSG_TYPE(13)
THINGMSG_TYPE(14)
THINGMSG_TYPE(17)
THINGMSG_TYPE(18)
THINGMSG_TYPE(19)
THINGMSG_TYPE(1a)
THINGMSG_TYPE(1b)
THINGMSG_TYPE(1c)
#undef THINGMSG_TYPE

u8 *__fastcall Obj_CtorPart8(u8 *object, void *, u32 value) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 0) = 0x006fd7b8;
    Fn_59b550(object + 8, nullptr, value);
    at<u32>(object, 0) = 0x006fd7b0;
    return object;
}
}
