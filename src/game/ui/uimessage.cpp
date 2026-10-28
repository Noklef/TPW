#include "uimessage.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vwrap.h"
#include "../world/advisorscore.h"
#include "dialog7ca13c.h"
#include "levelmeter.h"
#include "panel7cc328.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const UiMessageEnv kGameEnv = {&Obj7b51f0_CheckOnline, &Obj7b51f0_RefreshResearch, &Obj7b51f0_PassThingTo7cc218,
                               &Obj7b51f0_OnGuestRemoved, &Obj7b51f0_OnGuestAdded, &Panel7cc328_OnStaff};
const UiMessageEnv *g_env = &kGameEnv;

u8 *object() { return reinterpret_cast<u8 *>((uintptr_t)0x007b51f0); }

}  // namespace

extern "C" {

void UiMessage_SetEnv(const UiMessageEnv *env) { g_env = env ? env : &kGameEnv; }

void __stdcall UiObject_HandleMessage(u8 *message) {
    using Method = u32(__fastcall *)(u8 *);
    const u32 type = (*reinterpret_cast<const Method *>(at<uintptr_t>(message, 0)))(message);
    // (the word +8 in the argument's slot: its high half the message pointer's)
    const u32 word = ((u32)(uintptr_t)message & 0xffff0000u) | at<u16>(message, 8);
    switch (type) {
    case 0x13: {
        const u32 value = at<u32>(message, 8);
        if (value == 5) g_env->checkOnline(object(), nullptr, 1);
        else if (value == 6) g_env->checkOnline(object(), nullptr, 0);
        break;
    }
    case 0x12: g_env->refreshResearch(object(), nullptr, at<u32>(message, 8)); break;
    case 0xa: Stub_Ret4b(word); break;
    case 7:
    case 8: g_env->passThing(object(), nullptr, word); break;
    case 0x1b: g_env->guestRemoved(object(), nullptr, word); break;
    case 0x1c: g_env->guestAdded(object(), nullptr, word); break;
    case 0xf: g_env->staff(object(), nullptr, at<u16>(message, 8), at<u32>(message, 0xc)); break;
    default: break;
    }
}
}
