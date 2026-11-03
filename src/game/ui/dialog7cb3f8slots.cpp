// The dialog [0x7cb3f8]'s four user slots shown: see dialog7cb3f8slots.h.
#include "dialog7cb3f8slots.h"

#include <cstdint>

#include "../core/language.h"
#include "../core/vcall.h"
#include "../files/userslots.h"
#include "window.h"

namespace {

const Dialog7cb3f8SlotsEnv kGameEnv = {&UserSlots_Instance, &Records24_IfUsed, &Records24_Field14, &Language_WideText, &Window_SetShown};
const Dialog7cb3f8SlotsEnv *g_env = &kGameEnv;

u8 *dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cb3f8); }

const u32 kSlots[4] = {0x7a14, 0x7a19, 0x7a1a, 0x7a1b};

}  // namespace

void Dialog7cb3f8Slots_SetEnv(const Dialog7cb3f8SlotsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cb3f8_RefreshSlots() {
    if (!dialog()) return;
    for (u32 slot = 0; slot < 4; ++slot) {
        u8 *const row = Window_FindChild(Window_FindChild(dialog(), nullptr, kSlots[slot]), nullptr, 0x7a15);
        const u8 *const record = g_env->used(g_env->instance(), nullptr, slot);
        u32 last = 0;
        if (!record) {
            const u32 text = static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->wideText(0x1a5)));
            vcall<void>(row, 0x24, text);
            g_env->setShown(Window_FindChild(row, nullptr, 0x7a16), nullptr, 0);
        } else {
            vcall<void>(row, 0x24, *reinterpret_cast<const u32 *>(record + 8));
            g_env->setShown(Window_FindChild(row, nullptr, 0x7a16), nullptr, 1);
            last = g_env->field14(g_env->instance(), nullptr, slot);
        }
        g_env->setShown(Window_FindChild(Window_FindChild(dialog(), nullptr, kSlots[slot]), nullptr, 0x7a17), nullptr, last);
    }
}
}
