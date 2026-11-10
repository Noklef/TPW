// The dialogs that name a thing: see signnamedialog.h.
#include "signnamedialog.h"

#include <cstdint>

#include "../core/cheatmodule.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../files/file.h"
#include "../render/textparts.h"
#include "dialog7ca210.h"
#include "focus.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"
#include "windowscript.h"
#include "panel7cc218.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using NameFn = void(__fastcall *)(u8 *, void *, u16 *, u32);
using SetNameFn = void(__fastcall *)(u8 *, void *, const u16 *);

u16 *const kNameA = reinterpret_cast<u16 *>((uintptr_t)0x007c2668);
u16 *const kNameB = reinterpret_cast<u16 *>((uintptr_t)0x007c2688);
u16 *const kNameBuffer = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
const u32 kLabel = 0x3943e, kEditA = 0x3943c, kEditB = 0x3943d;

const SignNameDialogOpenEnv kOpenGameEnv = {WindowScript_Build, Window_SetDwordA0, GameHeap_Alloc, TextPart6ff530_Ctor, TextPart6ff4d0_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window_NotifyC00, Language_WideText, HoverLabel_SetYellow, Window_Notify40000,
                                            Window705a20_SetBuffer, GuiWindows_FindByName, Window705a20_SetCaret, reinterpret_cast<NameFn>((uintptr_t)0x004e2860), reinterpret_cast<NameFn>((uintptr_t)0x004e2920), Window705a20_SelectAll, KeyMaps_ResetAllAndSetFlag, Window_SetFocus};
const SignNameDialogOpenEnv *g_open = &kOpenGameEnv;

s32 __stdcall RemoveFont(const char *path) { return global<s32(__stdcall *)(const char *)>(0x006fd058)(path); }

const SignNameDialogHandlerEnv kHandlerGameEnv = {KeyMaps_EnableAllAndClearFlag, Focus_Restore, reinterpret_cast<SetNameFn>((uintptr_t)0x004e2950), reinterpret_cast<SetNameFn>((uintptr_t)0x004e2a50), reinterpret_cast<void(__cdecl *)(u8 *, const u16 *, const u16 *)>((uintptr_t)0x004682b0),
                                                  RemoveFont, File_Delete, GlobalSave_Instance, GlobalSave_SetSignNameA, GlobalSave_SetSignNameB, CheatSystem_Construct,reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0),
                                                  CheatSystem_SetOn, Thing_NameBuffer7ba058, Window_PostMessage, Window_SendMessage, Window_SetFocus};
const SignNameDialogHandlerEnv *g_handler = &kHandlerGameEnv;

u8 *tableThing() { return reinterpret_cast<u8 **>((uintptr_t)0x007cfb90)[at<u16>(global<u8 *>(0x007cf83c), 0x1da732) * 5]; }

void dressEdit(u8 *edit, u16 *buffer, u8 *thing, NameFn name) {
    u8 *const memory = static_cast<u8 *>(g_open->alloc(0x1a4));
    g_open->setImpl(edit, nullptr, memory ? g_open->editPart(memory, nullptr, 0) : nullptr);
    g_open->set4c(edit, nullptr, g_open->font(7));
    g_open->notify40000(edit, nullptr, 0xff, 0xff, 0xff, 0xff);
    g_open->setBuffer(edit, nullptr, (u32)(uintptr_t)buffer, 0xe);
    g_open->setCaret(edit, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_open->findByName(reinterpret_cast<const char *>((uintptr_t)0x0074fa84))));
    name(thing, nullptr, kNameBuffer, 0x3fff);
    vcall<void>(edit, 0x24, kNameBuffer);
}

void dressLabel(u8 *window) {
    u8 *const memory = static_cast<u8 *>(g_open->alloc(0x1a4));
    u8 *const part = memory ? g_open->labelPart(memory, nullptr, 0) : nullptr;
    g_open->setImpl(Window_FindChild(window, nullptr, kLabel), nullptr, part);
    u8 *const font = g_open->font(5);
    g_open->set4c(Window_FindChild(window, nullptr, kLabel), nullptr, font);
    g_open->notifyC00(Window_FindChild(window, nullptr, kLabel), nullptr, 1, 1);
    u8 *const label = Window_FindChild(window, nullptr, kLabel);
    vcall<void>(label, 0x24, g_open->text(0xc2));
    g_open->yellow(Window_FindChild(window, nullptr, kLabel));
}

void dressEdits(u8 *window, u8 *thing) {
    u8 *const first = Window_FindChild(window, nullptr, kEditA);
    dressEdit(first, kNameA, thing, g_open->nameA);
    g_open->selectAll(first);
    dressEdit(Window_FindChild(window, nullptr, kEditB), kNameB, thing, g_open->nameB);
    g_open->resetKeys();
    g_open->setFocus(Window_FindChild(window, nullptr, kEditA), nullptr, 0);
}

// The thing told the two names and the fonts gone (the vector's elements destroyed, its block kept).
void applyNames(u8 *thing) {
    g_handler->setNameA(thing, nullptr, kNameA);
    g_handler->setNameB(thing, nullptr, kNameB);
    g_handler->announce(thing, kNameA, kNameB);
    u8 *font = global<u8 *>(0x00789c1c);
    u8 *end = global<u8 *>(0x00789c20);
    if (font != end) {
        do {
            g_handler->removeFont(at<const char *>(font, 8));
            s32 result;
            g_handler->deleteFile(&result, at<const char *>(font, 8));
            end = global<u8 *>(0x00789c20);
            font += 0x14;
        } while (font != end);
        font = global<u8 *>(0x00789c1c);
    }
    for (u8 *name = font; name != end; name += 0x14) vcall<void>(name, 0xc, 0u);
    global<u8 *>(0x00789c20) = font;
}

// Each unit ^= its index; the length read again each time (a unit that turns 0 ends it, sic).
void scramble(u16 *name) {
    u32 i = 0;
    u32 length = Crt().wcslen(name);
    while (i < length) {
        name[i] ^= (u16)i;
        ++i;
        length = Crt().wcslen(name);
    }
}

// Everything the two handlers share but message 0x14 and the OK (0x100, -1).
s32 common(u8 *window, u32 message, u32 a, u32 b) {
    if ((message == 0x100 && a == 0xfffffffe) || message == 0x804) {
        g_handler->send(window, 4, 0, 0);
        return 0;
    }
    if (message == 0x802 && a != kEditA) {
        g_handler->post(window, 0x100, 0xffffffff, 0);
    } else if (message == 0x802 || message == 0x803) {
        g_handler->setFocus(Window_FindChild(window, nullptr, a == kEditA ? kEditB : kEditA), nullptr, 0);
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

}  // namespace

void SignNameDialog_SetOpenEnv(const SignNameDialogOpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }
void SignNameDialog_SetHandlerEnv(const SignNameDialogHandlerEnv *env) { g_handler = env ? env : &kHandlerGameEnv; }

extern "C" {

void __fastcall SignNameDialog_Open(u8 *self) {
    if (at<u8 *>(self, 0xc)) return;
    u8 *const thing = gPanel7cc218_7c2658;
    if (!thing || at<u8>(thing, 2) != 3) return;
    u8 *const parent = at<u8 *>(self, 0x10);
    const s32 depth = at<s16>(parent, 0xe0) + 0x11;
    u8 *const window = g_open->build(parent, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00750090), 0x0048d640);
    at<u8 *>(self, 0xc) = window;
    Window_SetDepth(window, nullptr, (u16)depth);
    g_open->setDword(window, nullptr, 0, (u32)(uintptr_t)thing);
    g_open->setDword(window, nullptr, 1, (u32)(uintptr_t)self);
    dressLabel(window);
    dressEdits(window, thing);
}

void __cdecl SignNameDialog7ca214_Open() {
    if (gSignNameDialog7ca214_7ca214) return;
    u8 *const parent = gDialog7ca210_7ca210;
    const s32 depth = at<s16>(parent, 0xe0) + 7;
    u8 *const window = g_open->build(parent, nullptr, gSignNameDialog7ca214_Open_751928, 0x004993e0);
    gSignNameDialog7ca214_7ca214 = window;
    Window_SetDepth(window, nullptr, (u16)depth);
    dressLabel(window);
    dressEdits(window, tableThing());
}

s32 __cdecl SignNameDialog_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        g_handler->enableKeys();
        at<u32>(reinterpret_cast<void *>((uintptr_t)Window_DwordA0(window, nullptr, 1)), 0xc) = 0;
        g_handler->restoreFocus();
    } else if (message == 0x100 && a == 0xffffffff) {
        applyNames(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 0)));
        vcall<void>(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 1)), 4);
        g_handler->send(window, 4, 0, 0);
        return 0;
    } else {
        return common(window, message, a, b);
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

s32 __cdecl SignNameDialog7ca214_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        g_handler->enableKeys();
        reinterpret_cast<u32 &>(gSignNameDialog7ca214_7ca214) = 0;
        g_handler->restoreFocus();
    } else if (message == 0x100 && a == 0xffffffff) {
        applyNames(tableThing());
        g_handler->saveNameA(g_handler->save(), nullptr, kNameA);
        g_handler->saveNameB(g_handler->save(), nullptr, kNameB);
        scramble(kNameA);
        scramble(kNameB);
        if (!Crt().wcscmp(gSignNameDialog7ca214_Handler_751a34, kNameA) && !Crt().wcscmp(gSignNameDialog7ca214_Handler_751a38, kNameB)) {
            u8 &guard = gSignNameDialog7ca214_Handler_fb34c4;
            u8 *const cheats = reinterpret_cast<u8 *>((uintptr_t)0x00fb3520);
            if (!(guard & 1)) {
                guard |= 1;
                g_handler->cheatsCtor(cheats);
                g_handler->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
            }
            g_handler->cheatsOn(cheats, nullptr, 1);
        }
        g_handler->send(window, 4, 0, 0);
        u8 *const thing = tableThing();
        u8 *const child = Window_FindChild(gDialog7ca210_7ca210, nullptr, 0x4f3af);
        vcall<void>(child, 0x24, g_handler->thingName(thing));
        return 0;
    } else {
        return common(window, message, a, b);
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
}
