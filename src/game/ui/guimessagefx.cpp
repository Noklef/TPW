#include "guimessagefx.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/initializers.h"
#include "../core/soundplay.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../render/particles.h"
#include "guirect.h"
#include "imehostattach.h"
#include "window.h"
#include "window705a50handler.h"
#include "../core/ime.h"
#include "../core/soundset.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const GuiMessageFxEnv kGameEnv = {&Sound_PlayAt, &ImeHost_Attach, &ImeHost_Detach, &ImeHost_OnWindow, &Window_FindChild, &Window_GetRect, &ParticlePool2_Finish, &ParticlePool2_Empty, &Fn_5eda40, &ParticlePool2_Setup};
const GuiMessageFxEnv *g_env = &kGameEnv;

u8 *Host() { return reinterpret_cast<u8 *>((uintptr_t)0x007b4e38); }
s32 Kind(u8 *window) { return vcall<s32>(window, 0xc); }

u32 Bits(float f) {
    u32 bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}
// fild n; fmul a float constant; fstp a float.
float Scaled(s32 n, u32 constant) { return (float)X87_MulIntDouble(n, global<float>(constant)); }

// The inner rectangle's words: left, top, right, bottom.
struct Box {
    s32 left, top, right, bottom;
    explicit Box(u8 *rect) {
        const s16 *words = reinterpret_cast<const s16 *>(rect + 4);
        left = words[0];
        top = words[1];
        right = words[2];
        bottom = words[3];
    }
    u32 CentreX() const { return Bits(Scaled((right - left) / 2 + left - 0x400, reinterpret_cast<uintptr_t>(&gGuimessagefx_6ff154))); }
    u32 CentreY() const { return Bits(Scaled((bottom - top) / 2 + top - 0x300, reinterpret_cast<uintptr_t>(&gGuimessagefx_6ff150))); }
};

}  // namespace

void GuiMessageFx_SetEnv(const GuiMessageFxEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __cdecl GuiMessageFx_Handler(u8 *window, u32 message, u32 a, u32) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    if (message == 0x100) g_env->playAt(0, gSoundCategories.ui, (Fn_65d57c(window) & 0x10) ? 0xbd : 0x1f, 0, 0, 0);
    if (message == 0x1f) {
        if (Kind(window) == 5) g_env->attach(Host(), nullptr, window);
    } else if (message == 0x14 || message == 0x20) {
        if (Kind(window) == 5) g_env->detach(Host());
    } else if (message == 0x801) {
        g_env->onWindow(Host(), nullptr, g_env->findChild(window, nullptr, a));
    }
    if (global<u8>(0x0078d90e)) {
        switch (message) {
            case 0x11:
                if (Kind(window) == 2 && window[0x139] == 1) g_env->finish();
                break;
            case 0x13:
                if (Kind(window) == 2 && Window705a50_StateWord(window) == 1) {
                    g_env->getRect(window, nullptr, rect, 1);
                    const Box box(rect);
                    const u32 y = box.CentreY();
                    g_env->position(box.CentreX(), y);
                }
                break;
            case 0x14:
                if (Kind(window) == 2 && (window[0x139] == 1 || Window705a50_StateWord(window) == 1)) g_env->finish();
                break;
            case 0x10001:
                if (Kind(window) == 2) {
                    g_env->getRect(window, nullptr, rect, 1);
                    const Box box(rect);
                    const float height = Scaled(box.bottom - box.top, reinterpret_cast<uintptr_t>(&gGuiMessageFx_Handler_6ff158));
                    const float width = Scaled(box.right - box.left, reinterpret_cast<uintptr_t>(&gGuiMessageFx_Handler_6ff15c));
                    const u32 y = box.CentreY();
                    g_env->setup(box.CentreX(), y, width, height);
                }
                break;
            case 0x10002:
                if (Kind(window) == 2) g_env->empty();
                break;
            default:
                break;
        }
    }
    GuiRect_Dtor(rect);
    return 0;
}
}
