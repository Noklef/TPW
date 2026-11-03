#include "dialog7cc2ec.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "focus.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *dialog() { return global<u8 *>(0x007cc2ec); }

const Dialog7cc2ecEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &HoverLabel_Attach, &Things_AverageOfKind};
const Dialog7cc2ecEnv *g_env = &kGameEnv;

void bar(u32 id, u32 index, u32 kind) {
    u8 *const window = Window_FindChild(dialog(), nullptr, id);
    panelparts::rangedNumber(window, 25);
    const u8 *const table = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da718_519450(global<u8 *>(0x007cf83c)));
    vcall<void>(window, 0x1c, *reinterpret_cast<const u32 *>(table + 0xc + index * 4));
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
    u8 *const mesh = memory ? MeshOverlay_Construct(memory, nullptr, reinterpret_cast<const char *>(0x00750e00)) : nullptr;
    Window_SetImpl(Window_FindChild(window, nullptr, 0x2e8), nullptr, mesh);
    u8 *const child = Window_FindChild(window, nullptr, 0x2e8);
    vcall<void>(child, 0x1c, static_cast<s32>(static_cast<u32>(g_env->average(kind)) << 10) / 100);
}

}  // namespace

void Dialog7cc2ec_SetEnv(const Dialog7cc2ecEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc2ec_Open() {
    if (dialog()) return;
    Window7c24c8_Close();
    Global7cb28c_Select(3, 9);
    u8 *const built = WindowScript_Build(global<u8 *>(0x007cb2ac), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00756400), 0x004b24b0);
    if (!built) return;
    global<u8 *>(0x007cc2ec) = built;
    Window7c24c8_Set(built);
    GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c));
    static const struct {
        u32 id, kind;
    } kBars[5] = {{0x2e7, 5}, {0x2ec, 4}, {0x2ea, 6}, {0x2eb, 7}, {0x2ed, 8}};
    for (u32 i = 0; i < 5; ++i) bar(kBars[i].id, i, kBars[i].kind);
    if (u8 *const figures = Window_FindChild(dialog(), nullptr, 0x2de)) {
        const u8 *const rings = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
        panelparts::text(figures, 0x2e3, 0x165);
        panelparts::redText(figures, 0x2e6, 0x166);
        panelparts::redText(figures, 0x2e1, 0x167);
        panelparts::text(figures, 0x2e5, 0x169);
        const u32 first = panelparts::newestOfRing(rings + 0x1fc94);
        global<u32>(0x007cc2d4) = first;
        vcall<void>(panelparts::numberPart(figures, 0x2e2), 0x1c, first);
        const u32 second = panelparts::newestOfRing(rings + 0x1f7f4);
        global<u32>(0x007cc2e0) = second;
        u8 *child = panelparts::numberPart(figures, 0x2df);
        vcall<void>(child, 0x1c, second);
        panelparts::red(child);
        const u32 third = panelparts::newestOfRing(rings + 0x1f5a4);
        global<u32>(0x007cc2dc) = third;
        global<u32>(0x007cc2e4) = third - second;
        child = panelparts::numberPart(figures, 0x2e0);
        vcall<void>(child, 0x1c, third - second);
        panelparts::red(child);
        vcall<void>(panelparts::numberPart(figures, 0x2e4), 0x1c, first - third);
    }
    g_env->attach(dialog(), 0x2da, (u32)(uintptr_t)Language_WideText(0x93));
    u8 &guard = global<u8>(0x00fb34c9);
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (global<u32>(0x00fb3b7c) == 2) vcall<void>(Window_FindChild(dialog(), nullptr, 0x2dd), 0x14, 0u);
}
