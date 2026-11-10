#include "panel7b9940.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vwrap.h"
#include "../render/renderer.h"
#include "../render/state.h"
#include "../world/mousemode.h"
#include "cursor.h"
#include "dialog7ca1bc.h"
#include "dialog7ca1c8.h"
#include "dialog7ca1e0.h"
#include "dialog7cb2b8build.h"
#include "dialogs.h"
#include "focus.h"
#include "guimanager.h"
#include "guirect.h"
#include "menuactions.h"
#include "msgtags.h"
#include "thingbrowser.h"
#include "window.h"
#include "window7cc8bc.h"
#include "windowfactory.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The offset k of the vbtable at self + table.
u32 vbase(u8 *self, u32 table, u32 k) { return *reinterpret_cast<const u32 *>(at<u8 *>(self, table) + k); }

u32 *const kSceneFlags = reinterpret_cast<u32 *>((uintptr_t)0x008bd508);
const u32 kOne = 0x3f800000;  // 1.0f

const Panel7b9940ExitEnv kExitGameEnv = {&Window7cc8bc_Close_4bf8e0};
const Panel7b9940ExitEnv *g_exit = &kExitGameEnv;

const Panel7b9940OpenEnv kOpenGameEnv = {&Gui_ResetQueues, &MsgTags_Create, &GameHeap_Alloc,&MouseModeOnlineBase_Ctor, &MouseMode_Set, &Renderer_Add, &Window_CreateChild, &Window_SetShown, &Window_SetHandler, &Window_GetRect, &Window_SetCursor, &WindowScript_Build,
                                         &Dialog7cb2b8_Build, &Dialog7ca1e0_Create, &ThingBrowser_Create, &Dialog7ca1bc_Create, &Dialog7ca098_Create, &Dialog7ca1c8_Create, &Focus_Restore, &GuiManager_CallSlot4, &GuiManager_CallSlot3, &Window7cc8bc_Create};
const Panel7b9940OpenEnv *g_open = &kOpenGameEnv;
}

void Panel7b9940_SetExitEnv(const Panel7b9940ExitEnv *env) { g_exit = env ? env : &kExitGameEnv; }
void Panel7b9940_SetOpenEnv(const Panel7b9940OpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }

extern "C" {

u8 *__fastcall Panel7b9940_Ctor(u8 *self, void *, u32 complete) {
    if (complete) {
        at<u32>(self, 0) = 0x6fdd00;
        at<u32>(self, 0xc) = 0x6fdcf8;
        at<u32>(self, 8) = 0x6fdce8;
        at<u32>(self, vbase(self, 0xc, 4) + 0xc) = 0x6fdcd8;
        const u32 d = vbase(self, 0xc, 4);
        at<u32>(self, d + 8) = d - 0x60;
        at<u32>(self, 0x5c) = 0;
    }
    at<u32>(self, vbase(self, 0, 4)) = 0x6fdcc8;
    const u32 d = vbase(self, 0, 4);
    at<u32>(self, d - 4) = d - 8;
    at<u32>(self, vbase(self, 0, 8) + 0x50) = 2;
    u8 *base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x2c) = 0;
    at<u32>(base, 0x30) = 0;
    at<u32>(base, 0x34) = kOne;
    base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    for (u32 off = 4; off <= 0x10; off += 4) at<u32>(base, off) = kOne;
    at<u32>(base, 0x54) = 0xffffffff;
    return self;
}

u32 __cdecl Panel7b9940_Detach() {
    GuiManager_CallSlot4();
    const u8 *const table = global<const u8 *>(0x007b9940);
    const u32 base = 0x007b9940 + *reinterpret_cast<const u32 *>(table + 4);
    return Renderer_RemoveFromSlot(gRenderer_87a6d0, nullptr, base, 0x20);
}

void __cdecl Panel7b9940_Open() {
    u8 *const self = reinterpret_cast<u8 *>((uintptr_t)0x007b9940);
    g_open->resetQueues();
    g_open->msgTags();
    if (!gPanel7b9940_7b05e8) {
        u8 *const mode = static_cast<u8 *>(g_open->alloc(1));
        if (mode) {
            u8 *const memory = static_cast<u8 *>(g_open->alloc(8));
            g_open->setMode(memory ? g_open->modeBase(memory) : nullptr);
            gPanel7b9940_7b05e8 = mode;
            global<u32>(0x007b05e0) = 0xffffffff;
            global<u32>(0x007b05d4) = 0xffffffff;
        } else {
            gPanel7b9940_7b05e8 = nullptr;
        }
    }
    if (u8 *const object = global<u8 *>(0x007b05d8)) (*reinterpret_cast<void(__fastcall *const *const *)(u8 *)>(object))[9](object);
    const u32 one = global<u32>(0x006ff148);
    u8 *base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x54) = 0xffffffff;
    for (u32 off = 4; off <= 0x10; off += 4) at<u32>(base, off) = one;
    base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x2c) = 0xbe99999a;
    at<u32>(base, 0x30) = 0xbf000000;
    at<u32>(base, 0x34) = 0x3f4ccccd;
    g_open->add(gRenderer_87a6d0, nullptr, self + vbase(self, 0, 4), 0x20);
    if (u8 *const window = g_open->create(global<u8 *>(0x00faa5d8), nullptr, 1, 0, 0, 0, 0, 0x7ff, 0x5ff)) {
        g_open->setShown(window, nullptr, 1);
        g_open->setHandler(window, nullptr, 0x004881a0);
        gPanel7b9940_7cb2ac = window;
        u8 rect[12];
        GuiRect_Ctor(rect);
        g_open->getRect(window, nullptr, rect, 1);
        const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6);
        const s32 halfWidth = (at<s16>(rect, 8) - left) / 2, halfHeight = (at<s16>(rect, 0xa) - top) / 2;
        const s32 scaleX = global<s16>(0x00faa5c4), scaleY = global<s16>(0x00faa5c0);
        global<float>(0x007a1c54) = (float)((halfWidth + left) * scaleX / 0x800);
        global<float>(0x007a1c58) = (float)((halfHeight + top) * scaleY / 0x600);
        global<float>(0x0074d104) = (float)(scaleX * halfWidth / 0x800);
        global<float>(0x0074d108) = (float)(scaleY * halfHeight / 0x600);
        GuiRect_Dtor(rect);
    }
    if (u8 *const window = g_open->create(global<u8 *>(0x00faa5d8), nullptr, 1, 0, 1, 0, 0, 0x7ff, 0x5ff)) {
        g_open->setHandler(window, nullptr, 0x00488a00);
        gPanel7b9940_7cb2b0 = window;
        g_open->setCursor(window, nullptr, 0x15);
        g_open->build(gPanel7b9940_7cb2b0, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x0074fa98), 0);
    }
    g_open->build7cb2b8();
    g_open->create7ca1e0();
    g_open->createBrowser();
    g_open->create7ca1bc();
    g_open->create7ca098();
    g_open->create7ca1c8();
    reinterpret_cast<u32 &>(gFocus_7c24c8) = 0;
    g_open->focus();
    if (global<u8>(0x0078d90e)) g_open->slot3();
    else g_open->slot4();
}

u32 __cdecl Panel7b9940_Exit() {
    g_exit->closeMenu();
    Window_SendMessage(gPanel7b9940_7cb2b0, 4, 0, 0);
    return Panel7b9940_Detach();
}

void __cdecl Panel7b9940_Close() {
    Windows7c26b0_Message5();
    const u8 *const table = global<const u8 *>(0x007b9940);
    const u32 base = 0x007b9940 + *reinterpret_cast<const u32 *>(table + 4);
    Renderer_RemoveFromSlot(gRenderer_87a6d0, nullptr, base, 0x20);
    GuiManager_CallSlot4();
    Global7ca1e0_Release();
    Globals7cc2fc_Release();
    Global7ca1bc_Release();
    Dialog7ca098_Message4IfUp();
    Global7ca1c8_Release();
    Window_SendMessage(gPanel7b9940_7cb2ac, 4, 0, 0);
    Window_SendMessage(gPanel7b9940_7cb2b0, 4, 0, 0);
    Table7c26b0_ReleaseIdle();
}

u32 __cdecl Panel7b9940_Show() {
    u8 *const self = reinterpret_cast<u8 *>((uintptr_t)0x007b9940);
    Gui_ResetQueues();
    const u32 one = global<u32>(0x006ff148);
    u8 *base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x54) = 0xffffffff;
    for (u32 off = 4; off <= 0x10; off += 4) at<u32>(base, off) = one;
    base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x2c) = 0xbe99999a;  // -0.3f
    at<u32>(base, 0x30) = 0xbf000000;  // -0.5f
    at<u32>(base, 0x34) = 0x3f4ccccd;  // 0.8f
    Renderer_Add(gRenderer_87a6d0, nullptr, self + vbase(self, 0, 4), 0x20);
    return global<u8>(0x0078d90e) ? GuiManager_CallSlot3() : GuiManager_CallSlot4();
}

u32 __cdecl Panel7b9940_ShowWindow() {
    u8 *const self = reinterpret_cast<u8 *>((uintptr_t)0x007b9940);
    g_open->resetQueues();
    const u32 one = global<u32>(0x006ff148);
    u8 *base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x54) = 0xffffffff;
    for (u32 off = 4; off <= 0x10; off += 4) at<u32>(base, off) = one;
    base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x2c) = 0xbe99999a;  // -0.3f
    at<u32>(base, 0x30) = 0xbf000000;  // -0.5f
    at<u32>(base, 0x34) = 0x3f4ccccd;  // 0.8f
    g_open->add(gRenderer_87a6d0, nullptr, self + vbase(self, 0, 4), 0x20);
    g_open->create7cc8bc();
    u8 *const made = g_open->create(global<u8 *>(0x00faa5d8), nullptr, 1, 0, 1, 0, 0, 0x7ff, 0x5ff);
    if (made) {
        g_open->setHandler(made, nullptr, 0x488a00);
        gPanel7b9940_7cb2b0 = made;
        g_open->setCursor(made, nullptr, 0x15);
        g_open->build(gPanel7b9940_7cb2b0, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x0074fa98), 0);
    }
    return global<u8>(0x0078d90e) ? g_open->slot3() : g_open->slot4();
}

void __cdecl Panel7b9940_SizeAtExit() {}
}
