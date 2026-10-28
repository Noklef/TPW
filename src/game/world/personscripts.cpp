#include "personscripts.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/options.h"
#include "../math/x87.h"
#include "../sprite/script.h"
#include "../sprite/scriptref.h"
#include "../terrain/land.h"
#include "person.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ThoughtsEnv kThoughtsGame = {&Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &SpriteScript_Kill,
                                   reinterpret_cast<s32(__cdecl *)(u32, u32, u32, u32, u32, u32, u32)>(&SpriteScript_Start)};
const ThoughtsEnv *g_thoughts = &kThoughtsGame;

// Each thought's bubble (an entry point) and wait (times 20 ticks), thoughts 1 to 0x16.
struct Bubble {
    u32 code, wait;
};
const Bubble kBubbles[0x16] = {
    {0x0074f250, 1}, {0x0074f280, 1}, {0x0074f2b0, 1}, {0x0074f2c8, 1}, {0x0074f310, 0}, {0x0074f268, 0}, {0x0074f2e0, 1}, {0x0074f208, 3},
    {0x0074f190, 3}, {0x0074f1a8, 3}, {0x0074f1d8, 3}, {0x0074f1f0, 0}, {0x0074f1c0, 0}, {0x0074f220, 1}, {0x0074f238, 0}, {0x0074f298, 0},
    {0x0074f2f8, 0}, {0x0074f328, 3}, {0x0074f340, 2}, {0x0074f358, 0}, {0x0074f370, 0}, {0x0074f388, 0},
};

}  // namespace

void Thoughts_SetEnv(const ThoughtsEnv *env) { g_thoughts = env ? env : &kThoughtsGame; }

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

void __fastcall Thoughts_Show(u8 *thoughts, void *, u32 thought, u8 force) {
    const ThoughtsEnv *const E = g_thoughts;
    at<u32>(thoughts, 0) = thought;
    if (global<u8>(0x00790ab0) & 0x16) return;
    u8 &guard = global<u8>(0x00fb34aa);   // (the mode setting 0xfb3b7c: a function-local static)
    if (!(guard & 1)) {
        guard |= 1;
        E->readMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        E->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (global<u32>(0x00fb3b7c) == 1 && force == 0) return;
    if (at<u32>(thoughts, 0x84) != 0) {
        E->kill(at<s32>(thoughts, 0x84));
        at<u32>(thoughts, 0x84) = 0;
    }
    if (thought - 1 >= 0x16) return;   // (the empty Debug_Printf)
    const Bubble &bubble = kBubbles[thought - 1];
    const u32 clock = at<u32>(*global<u8 **>(0x007cf6ec), 0x1da70c);
    if (clock < at<u32>(thoughts, 0x8c) + bubble.wait * 20) return;
    at<u32>(thoughts, 0x8c) = clock;
    at<s32>(thoughts, 0x84) = E->start(bubble.code, 9, 0, 0, 0, 0, 0);
}
}
