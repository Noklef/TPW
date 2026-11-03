#include "textpanel7cc18c.h"

#include <cstdint>

#include "../gfx/guitext.h"
#include "../gfx/imageops.h"
#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TextPanel7cc18cEnv kGameEnv = {Window_ImplSlot7, Panel_Reset, GuiText_DrawRgb, Panel_ShrinkInto, Window_ImplSlot6};
const TextPanel7cc18cEnv *g_env = &kGameEnv;

}  // namespace

void TextPanel7cc18c_SetEnv(const TextPanel7cc18cEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl TextPanel7cc18c_Show(u8 *window) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    s32 width, height;
    u8 *const image = reinterpret_cast<u8 *>((uintptr_t)g_env->open(window, nullptr, (u32)(uintptr_t)&width, (u32)(uintptr_t)&height));
    if (image && global<u8 *>(0x007cc18c)) {
        g_env->reset(global<u8 *>(0x007cc18c));
        u8 *const panel = global<u8 *>(0x007cc18c);
        g_env->draw(reinterpret_cast<const u16 *>((uintptr_t)0x007cbf48), *reinterpret_cast<u32 *>(panel + 0x5c), *reinterpret_cast<u32 *>(panel + 0x50), *reinterpret_cast<u32 *>(panel + 0x54), 0x142, 0x5e, 0x27c, 0xd7, 0xff0000cc);
        const u32 mode = global<u32>(0x008bd554);
        if (mode == 2) g_env->shrink(global<u8 *>(0x007cc18c), nullptr, image, width, height, 2);
        else if (mode == 3) g_env->shrink(global<u8 *>(0x007cc18c), nullptr, image, width, height, 1);
        g_env->close(window);
    }
    GuiRect_Dtor(rect);
}
}
