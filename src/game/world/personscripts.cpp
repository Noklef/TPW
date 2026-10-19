#include "personscripts.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../math/x87.h"
#include "../sprite/script.h"
#include "../sprite/scriptref.h"
#include "../terrain/land.h"
#include "person.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Visitor_PreviousRideTick(u8 *visitor) {
    const u8 *world = *reinterpret_cast<u8 *const *>(0x0080239c);
    if (at<u32>(world, 0x1da70c) % 20 == 0)
        Visitor_PushPreviousRide(visitor, nullptr, (u32)(uintptr_t)visitor & 0xffff0000);
}

void __fastcall Person_ReleaseScripts(u8 *person) {
    Obj_KillSlot(person + 0xc);
    Person_KillScript84(person + 0x30);
    if (at<u8>(person, 2) == 1 && at<u32>(person, 0x210))
        Person_JumpScript210(person);
}

void __fastcall Person_MoveScript(u8 *person, void *, u32 a, u32 b, u32 c, u32 d) {
    if (!at<u32>(person, 0x210))
        return;
    SpriteScript_SetPosition((s32)at<u32>(person, 0x210), at<u32>(person, 0x218), b, at<u32>(person, 0x21c), d);
    at<u32>(person, 0x218) = a;
    at<u32>(person, 0x21c) = c;
}

void __fastcall Person_KillScript84After(u8 *person) {
    const u8 *world = **reinterpret_cast<u8 *const *const *>(0x007cf6ec);
    if (at<u32>(world, 0x1da70c) > at<u32>(person, 0x8c) + 0xc && at<u32>(person, 0x84)) {
        SpriteScript_Kill((s32)at<u32>(person, 0x84));
        at<u32>(person, 0x84) = 0;
    }
}

void __fastcall Person_JumpScript210(u8 *person) {
    if (at<u32>(person, 0x210)) {
        SpriteScript_Jump((s32)at<u32>(person, 0x210), 0x0074f4c0);
        at<u32>(person, 0x210) = 0;
    }
}

void __fastcall Person_KillScript84(u8 *person) {
    if (at<u32>(person, 0x84)) {
        SpriteScript_Kill((s32)at<u32>(person, 0x84));
        at<u32>(person, 0x84) = 0;
    }
}

void __fastcall Person_KillScript84Keep(u8 *person) {
    if (at<u32>(person, 0x84))
        SpriteScript_Kill((s32)at<u32>(person, 0x84));
}

u32 __fastcall Person_PlaceScript(u8 *person, void *, u32 x, float y, u32 z, u32 r15, u32 anim) {
    u8 *const ref = person + 0xc;
    if (!at<u32>(ref, 0)) ScriptRef_Start(ref, nullptr, anim, at<u32>(person, 0x24), at<u32>(person, 0x20));
    float fx, fz;
    std::memcpy(&fx, &x, 4);
    std::memcpy(&fz, &z, 4);
    const float above = (float)X87_SubR(X87Reg(Terrain_HeightAt(fx, fz)), X87Reg(y));  // fsubr y; fstp float
    u32 bits;
    std::memcpy(&bits, &above, 4);
    SpriteScript_SetPosition(at<s32>(ref, 0), x, bits, z, r15);
    if (!ScriptRef_IsAt(ref, nullptr, anim)) Set4_4217f0(ref, nullptr, anim);
    return 0;
}
}
