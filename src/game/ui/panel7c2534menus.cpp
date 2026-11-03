#include "panel7c2534menus.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/options.h"
#include "../online/session.h"
#include "../world/object.h"
#include "dialogs.h"
#include "panel7c2534.h"

extern "C" const u16 *__cdecl Language_WideText(u32 id);

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Panel7c2534MenusEnv kGameEnv = {
    &Class492d80_Ctor,
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)0x00492e80),
    &Setting_ReadMode,
    reinterpret_cast<u32(__cdecl *)(u32)>((uintptr_t)0x0067a7d0),
    &Language_WideText,
    &Panel7c2534_AddRow,
    reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005b50b0),
    &Online_LoggedIn,
    &Obj_HasActivity,
};
const Panel7c2534MenusEnv *g_env = &kGameEnv;

const Panel7c2534OpenEnv kOpenEnv = {
    &Panel7c2534_BuildMain,
    &Panel7c2534_BuildOnline,
    &Dialog7cb2b8_ResetChild21,
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)0x004092a0),
    &Fn_662411,
    &WindowAt10_Show,
};
const Panel7c2534OpenEnv *g_open = &kOpenEnv;

void NewPanel(u32 handler) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    u8 *const panel = block ? g_env->ctor(block) : nullptr;
    global<u8 *>(0x007c2534) = panel;
    g_env->setHandlers(panel, nullptr, handler, 0x0048b2a0);
}

u8 *Online() {
    if (!global<u8 *>(0x00f7d88c)) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x404));
        global<u8 *>(0x00f7d88c) = block ? g_env->onlineCtor(block) : nullptr;
    }
    return global<u8 *>(0x00f7d88c);
}

s32 Row(s32 top, u32 text, u32 id, u32 colour) {
    const u16 *const wide = g_env->wideText(text);
    return g_env->addRow(global<u8 *>(0x007c2534), nullptr, top, wide, id, 0, colour, colour, colour);
}

}  // namespace

extern "C" {

void Panel7c2534Menus_SetEnv(const Panel7c2534MenusEnv *env) { g_env = env ? env : &kGameEnv; }

void Panel7c2534Open_SetEnv(const Panel7c2534OpenEnv *env) { g_open = env ? env : &kOpenEnv; }

void __cdecl Panel7c2534_BuildMain() {
    NewPanel(0x0048b6a0);
    if (!(global<u8>(0x00fb34c6) & 1)) {
        global<u8>(0x00fb34c6) |= 1;
        g_env->readMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_env->atExit(0x005d1750);
    }
    const u32 grey = 0x80;
    s32 top;
    if (global<u32>(0x00fb3b7c) == 1) {
        top = Row(0xa, 7, 0x10, grey);
        top = Row(top + 5, 0xc, 0x13, grey);
        Row(top + 5, 8, 0x14, grey);
        return;
    }
    if (at<u32>(global<u8 *>(0x007cf83c), 0x1da738) == 4) {
        top = Row(0xa, 3, 0x16, grey);
        top = Row(top + 5, 0xa, 0x17, grey);
        top = Row(top + 5, 7, 0x15, grey);
        top = Row(top + 5, 0xc, 0x18, grey);
        Row(top + 5, 8, 0x19, grey);
        return;
    }
    const s32 gap = g_env->loggedIn(Online()) ? 0 : 5;
    top = Row(gap * 2, 3, 1, grey);
    top = Row(gap + top, 4, 2, grey);
    top = Row(gap + top, 0xa, 3, grey);
    top = Row(gap + top, 5, 4, grey);
    if (g_env->loggedIn(Online())) top = Row(gap + top, 2, 7, grey);
    if (!g_env->hasActivity(Online())) top = Row(gap + top, 6, 5, grey);
    top = Row(gap + top, 7, 0, grey);
    top = Row(gap + top, 0xc, 6, grey);
    Row(gap + top, 8, 8, grey);
}

void __cdecl Panel7c2534_BuildOnline() {
    NewPanel(0x0048bd40);
    s32 top;
    if (at<u32>(Online(), 0x108)) {
        top = Row(5, 0xe0, 0xd, 0);
    } else {
        top = g_env->loggedIn(Online()) ? Row(5, 2, 0xa, 0) : Row(5, 1, 9, 0);
        if (!g_env->hasActivity(Online())) top = Row(top + 5, 6, 0xb, 0);
        u8 *const thing = at<u8 *>(global<u8 *>(0x00f85614), 4);
        if ((*reinterpret_cast<u32(__fastcall **)(u8 *)>(*reinterpret_cast<u32 *>(thing) + 0x24))(thing)) top = Row(top + 5, 0xd, 0xc, 0);
    }
    top = Row(top + 5, 7, 0xe, 0);
    Row(top + 5, 8, 0xf, 0);
}

void __cdecl Panel7c2534_Open(u32 flag) {
    if (global<u8 *>(0x007c2534)) return;
    if (flag) {
        g_open->buildOnline();
    } else {
        g_open->buildMain();
        g_open->resetChild(1);
        if (global<u32>(0x00786ba4) == 1) {
            g_open->leave(reinterpret_cast<u8 *>((uintptr_t)0x00786b68), nullptr, 0, 0);
            global<u32>(0x00786ba4) = 0;
            global<u8>(0x007c2518) = 0;
            g_open->refresh();
            g_open->show(global<u8 *>(0x007c2534), nullptr, flag);
            return;
        }
    }
    global<u8>(0x007c2518) = 1;
    g_open->show(global<u8 *>(0x007c2534), nullptr, flag);
}
}
