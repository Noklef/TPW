#include "guistartup.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/app.h"
#include "../core/language.h"
#include "../core/trivial.h"
#include "../core/vwrap.h"
#include "../gfx/screen.h"
#include "../gfx/surface.h"
#include "../render/textparts.h"
#include "cursor.h"
#include "guifont.h"
#include "guisystem.h"
#include "textpool.h"
#include "../core/cmdline.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *object(u32 address) { return reinterpret_cast<u8 *>((uintptr_t)address); }

template <typename F> F byAddress(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const GuiStartupEnv kStartupEnv = {
    &Screen_Open,
    &Surface_SetDirectDraw,
    &App_SetWindow,
    byAddress<void(__cdecl *)(void *)>(0x005f55c0),
    byAddress<void(__cdecl *)(void *)>(0x005f5600),
    byAddress<void(__cdecl *)()>(0x0047e7d0),
    &GuiFont_Init,
    &Language_IsHebrew,
    reinterpret_cast<u32(__cdecl *)(u32, u32, u32)>(&Gui_Init),
    byAddress<void(__cdecl *)(u32)>(0x00659674),
    &Fn_6590ea,
    &TextPool_Init,
    &OutOne_44e600,
    &CursorTable_Init,
    &GameCursors_LoadAni,
    &GameCursors_LoadTga,
    &Gui_SetCursorTable,
    byAddress<void(__cdecl *)(u32)>(0x006591d4),
    &Gui_SetCursor,
    byAddress<void(__cdecl *)()>(0x006591e3),
    byAddress<void(__cdecl *)()>(0x004891d0),
    byAddress<void(__cdecl *)()>(0x004893c0),
    &TextPart_SetLayout,
    byAddress<void(__cdecl *)()>(0x00492470),
};
const GuiTeardownEnv kTeardownEnv = {
    &CursorTable_Clear,
    byAddress<void(__cdecl *)()>(0x005f5620),
    byAddress<void(__cdecl *)()>(0x005f55e0),
    &Screen_Close,
    &Global7b4b84_Delete,
    &Gui_Shutdown,
    &TextPool_Free,
    byAddress<void(__fastcall *)(u8 *)>(0x00657dff),
    reinterpret_cast<u32(__cdecl *)(u32)>(&Const1_424ec0),
};
const GuiStartupEnv *g_startup = &kStartupEnv;
const GuiTeardownEnv *g_teardown = &kTeardownEnv;

bool WindowMode() { return (global<u32>(0x007a1a8c) & 0x800) && !gCmdline_788270; }

void Delete(u32 holder) {
    u8 *const thing = global<u8 *>(holder);
    if (thing) (*reinterpret_cast<void(__fastcall **)(u8 *, void *, u32)>(*reinterpret_cast<u32 *>(thing)))(thing, nullptr, 1);
}

}  // namespace

extern "C" {

void GuiStartup_SetEnv(const GuiStartupEnv *startup, const GuiTeardownEnv *teardown) {
    g_startup = startup ? startup : &kStartupEnv;
    g_teardown = teardown ? teardown : &kTeardownEnv;
}

void __cdecl Gui_Startup() {
    const GuiStartupEnv *const E = g_startup;
    u8 local[0x30];
    E->screenOpen(reinterpret_cast<s32 *>(local), nullptr, 0, nullptr, nullptr);
    E->setDirectDraw(*global<void **>(0x00879a24));
    if (WindowMode()) {
        E->setWindow(global<void *>(0x007a130c));
        E->fn5f55c0(local);
        E->fn5f5600(local);
        global<u32>(0x007b9fe0) = 0;
        global<u32>(0x007b9fe4) = 0;
        global<u32>(0x007b9fe8) = 0;
        global<u32>(0x007b9fec) = 0;
        global<u32>(0x007b9ff0) = 0x00486c70;
        E->fn47e7d0();
    }
    const s32 screenY = global<s16>(0x00faa5c0);
    const s32 b = 0x3000 / screenY;
    const s32 height = 0x4800 / screenY;
    const s32 width = 0x4000 / global<s16>(0x00faa5c4);
    E->fontInit(object(0x00786de8), nullptr, 0xff, width, 0, height, b);
    E->guiInit(0x80, 0x800, E->isHebrew(object(0x0078b530)));
    E->fn659674(0x007b9864);
    E->fn6590ea(0x00485780);
    E->poolInit(0x40, 0x80, 0x10, 0x80);
    E->outOne(0x100000, 3, 0xd, &gGuistartup_7c249c);
    global<u32(__stdcall *)(u32)>(0x006fd30c)(0);
    E->tableInit(object(0x007b9e88), nullptr, 0x16);
    E->tableInit(object(0x007b9ab0), nullptr, 0x16);
    E->loadAni();
    E->loadTga();
    E->setTable(WindowMode() ? object(0x007b9ab0) : object(0x007b9e88));
    E->fn6591d4(0);
    E->setCursor(1);
    E->fn6591e3();
    E->fn4891d0();
    E->fn4893c0();
    global<u16>(0x007c2128) = 1;
    u8 *const inner = local + 4;
    inner[4] = 1;
    inner[5] = 1;
    inner[9] = 0;
    inner[0x10] = 0xff;
    inner[0x11] = 0xff;
    inner[0x12] = 0xff;
    u8 *const info = local + 0x18;
    *reinterpret_cast<u8 **>(info + 0x10) = inner;
    E->setLayout(object(0x007c2118), nullptr, 0xc00, info);
    E->fn492470();
    gGuistartup_7c24d4 = global<u32(__fastcall *)(u8 *)>(0x006fd5ac)(object(0x007c24c0));
}

u32 __cdecl Gui_Teardown() {
    const GuiTeardownEnv *const E = g_teardown;
    E->tableClear(object(0x007b9ab0));
    E->tableClear(object(0x007b9e88));
    E->fn5f5620();
    E->fn5f55e0();
    E->screenClose();
    E->listenEnd();
    E->guiShutdown();
    Delete(0x007b9a04);
    Delete(reinterpret_cast<uintptr_t>(&gGui_Teardown_7c231c));
    Delete(0x007b9804);
    E->poolFree();
    E->fn657dff(object(0x00786de8));
    E->fn424ec0(gGuistartup_7c249c);
    return global<u32(__fastcall *)(u8 *)>(0x006fd5b0)(object(0x007c24c0));
}
}
