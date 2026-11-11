// The thing panels' go-to: see thinggoto.h.
#include "thinggoto.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../world/focuscell.h"
#include "../world/gamemode.h"
#include "../world/mapclick.h"
#include "panel7cc218.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ThingGotoEnv kGameEnv = {GameAddress<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020), &Language_WideText, &Global7c2654_Set, &FocusCell_Update, &GameMode_Set, &GameMode_MapClick};
const ThingGotoEnv *g_env = &kGameEnv;

void go(const u8 *thing) {
    const u32 cell = (static_cast<u32>(thing[7]) << 7) + thing[5];   // (the original adds 1 to store a word, then takes it off)
    g_env->set2654((u32)(uintptr_t)thing);
    g_env->focusCell(1);
    g_env->setMode(0x33, 1);
    g_env->mapClick(cell & 0x7f, cell >> 7, 0, 0);
}

}  // namespace

void ThingGoto_SetEnv(const ThingGotoEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall ThingGoto_Start(u8 *panel) {
    if (global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x37)) {
        struct Question {
            u16 *text;
            u8 colour[4];
            u32 kind;
            const u8 *thing;
        } question;
        question.text = g_env->wideText(0x18c);
        memset(question.colour, 0xff, 3);
        question.kind = 4;
        question.thing = gPanel7cc218_7c2658;
        g_env->messageBox(reinterpret_cast<const u8 *>(&question), 2, 0, 1, 0x0048cde0, 0, 0, 0);
    } else {
        go(gPanel7cc218_7c2658);
    }
    gPanel7cc218_7c2658 = nullptr;
    vcall<void>(panel, 0x2c);
}
