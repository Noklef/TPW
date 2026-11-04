// The text button object's window made: see textbuttonwindow.h.
#include "textbuttonwindow.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "window.h"
#include "windowreg.h"

namespace {

const TextButtonWindowEnv kGameEnv = {&Window_Ctor, &Window_SetHandler, &GuiWindows_FindByName, &Window_SetObject98, &Window_SetDwordA0, &Window_SetShown};
const TextButtonWindowEnv *g_env = &kGameEnv;

}  // namespace

void TextButtonWindow_SetEnv(const TextButtonWindowEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall TextButton_CreateWindow(u8 *object, void *, u32 handler, u32 second) {
    u8 *&window = *reinterpret_cast<u8 **>(object + 0x10);
    if (window) return;
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x138));
    if (memory) {
        const u32 high = (u32)Crt().rand();
        const u32 low = (u32)Crt().rand();
        const u32 id = high * low + (u32)Crt().rand();
        window = g_env->ctor(memory, nullptr, nullptr, 1, id, 0, 0, 0x7ff, 0x5ff);
    } else {
        window = nullptr;
    }
    if (!window) return;
    g_env->setHandler(window, nullptr, handler);
    g_env->setObject98(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->find(reinterpret_cast<const char *>((uintptr_t)0x0074f9bc))));
    g_env->setDword(window, nullptr, 0, (u32)(uintptr_t)object);
    if (second) *reinterpret_cast<u32 *>(object) = second;
    g_env->setShown(window, nullptr, 0);
}
}
