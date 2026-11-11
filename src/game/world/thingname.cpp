// A thing's two names as text: see thingname.h.
#include "thingname.h"

#include <cstdint>

#include "../core/crt.h"
#include "../ui/window7cc8c0handler.h"
#include "signtext.h"

namespace {

u16 *global_text(u32 address) { return reinterpret_cast<u16 *>((uintptr_t)address); }

const ThingNameEnv kGameEnv = {CRT_snwprintf};
const ThingNameEnv *g_env = &kGameEnv;

}  // namespace

void ThingName_SetEnv(const ThingNameEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Thing_NameA(u8 *thing, void *, u16 *to, u32 count) {
    u16 *const text = reinterpret_cast<u16 *>(thing + 0x3c8);
    const u16 *const format = gText_Signtext_75c0cc;
    s32 length = 0;
    while (text[length]) ++length;
    if (length == 0) {
        g_env->snwprintf(to, count, global_text(reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374)));
        return;
    }
    if (length > 1 && text[length - 1] == '-' && text[length - 2] == '-') {
        u16 *const copy = gThingName_Copy_7cdc58;
        copy[0x20] = 0;
        g_env->snwprintf(copy, 0x21, format, text);
        copy[length - 1] = 0;
        g_env->snwprintf(to, count, format, copy);
        return;
    }
    g_env->snwprintf(to, count, format, text);
}

void __fastcall Thing_NameB(u8 *thing, void *, u16 *to, u32 count) {
    g_env->snwprintf(to, count, gText_Signtext_75c0cc, reinterpret_cast<u16 *>(thing + 0x40a));
}
