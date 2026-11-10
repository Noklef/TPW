#include "advisorsend.h"

#include <cstdint>

#include "messagecentre.h"
#include "thingmsg.h"

namespace {

AddrCalls g_calls;

const u32 kScoring = 0x00fb3540;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The byte +0x14 taken; gAdvisorScoring made at the first use (the game repeats this guard before each use of it).
bool take(u8 *self) {
    if (!at<u8>(self, 0x14))
        return false;
    at<u8>(self, 0x14) = 0;
    u8 &guard = global<u8>((u32)(uintptr_t)&gAdvisorsend_fb2765 + 0x1);
    if (!(guard & 1)) {
        guard |= 1;
        g_calls.Th(0x0059bea0, kScoring);
        g_calls.Cd(0x0067a7d0, 0x00415e70u);
    }
    return true;
}

u32 say(u8 *self, u32 *out, s32 offset) {
    const u32 id = at<u32>(self, 0xc);
    const s32 count = (s32)g_calls.Th(0x0059c2b0, kScoring, id);
    u32 speech = g_calls.Th(0x0059c240, kScoring, id);
    if (offset < count)
        speech += offset;
    if (speech == 0x266) {
        *out = 0;
        return 0;
    }
    *out = g_calls.Cd(0x00599050, speech);
    const u32 text = g_calls.Cd(0x005997b0, speech);
    u8 msg[0x18];
    ThingMsg18_Ctor(msg, nullptr, text, at<u16>(self, 0), at<u32>(self, 4), 0xffffffffu);
    g_calls.Th(0x0040fb10, (u32)(uintptr_t)MessageCentre_Instance(), msg);
    return 1;
}

}  // namespace

void AdvisorSend_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall AdvisorMsg_Send(u8 *self, void *, u32 *out, s32 offset) {
    return take(self) ? say(self, out, offset) : 0;
}

u32 __fastcall AdvisorMsg_SendScored(u8 *self, void *, u32 *out, s32 offset) {
    if (!take(self))
        return 0;
    u16 word = 0;
    u32 dword = 0;
    const s32 score = (s32)g_calls.Th(0x0059c0a0, kScoring, at<u32>(self, 0xc), &word, &dword);
    if (score < global<s32>(0x00fb3560))
        return 0;
    if (word)
        at<u16>(self, 0) = word;
    if (dword)
        at<u32>(self, 4) = dword;
    return say(self, out, offset);
}
}
