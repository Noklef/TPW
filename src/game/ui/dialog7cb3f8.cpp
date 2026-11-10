#include "dialog7cb3f8.h"

#include <cstdint>

#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/globals.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../files/userslots.h"
#include "../frontend/scenequeue.h"
#include "../frontend/sceneleave.h"
#include "../render/textparts.h"
#include "dialog7cb3f8slots.h"
#include "dialog7cb400.h"
#include "dialog7cb400close.h"
#include "guimanager.h"
#include "hoverlabel.h"
#include "window.h"
#include "windowscript.h"
#include "../online/chatlists.h"
#include "../core/textarg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cb3f8Env kGameEnv = {
    &KeyMaps_ResetAll,
    &Window_SetShown,
    &WindowScript_Build,
    &Window_SetFocus,
    &Window_FindChild,
    reinterpret_cast<void *(__cdecl *)(u32)>((uintptr_t)0x0067acc0),
    &TextPart6ff530_Ctor,
    &Window_SetImpl,
    &Language_Font,
    &Window_Set4c,
    &Window_NotifyC00,
    &HoverLabel_SetYellow,
    &Window_Notify40000,
    &Window_SetDwordA0,
    &Window_SetHandler,
    &Language_WideText,
    &Dialog7cb3f8_RefreshSlots,
    reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005c7420),
    &Records24_CountUsed,
    &Class702da0_Push,
    &Class702cb0_PlayFirst,
    &Window_CallDefault,
    &Window_DwordA0,
    &UserSlots_Instance,
    &Records24_IfUsed,
    reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, const u32 *)>(&Charset_FormatWide),
    reinterpret_cast<void(__cdecl *)(const void *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020),
    &Dialog7cb400_Open,
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x005c83b0),
    &GuiManager_CallSlot3,
    &GuiManager_CallSlot4,
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x005c7e90),
    &Global786b80_SetAndMode3,
    &Dialog7cb400_Close,
};
const Dialog7cb3f8Env *g_env = &kGameEnv;

u32 byteOf(u32 address) { return (u32)(s32)global<s8>(address); }

void notifyColour(u8 *window, u32 red, u32 green, u32 blue) { g_env->notify40000(window, nullptr, byteOf(red), byteOf(green), byteOf(blue), 0xff); }

// A question's text record (+0xf not set).
struct Question {
    const void *text;
    u8 colour[4];
    u32 four;
    u32 value;
};

void ask(const void *text, u32 value, u32 handler) {
    Question question;
    question.text = text;
    question.colour[0] = question.colour[1] = question.colour[2] = 0xff;
    question.four = 4;
    question.value = value;
    g_env->box(&question, 2, 0, 1, handler, 0, 0, 0);
}

// A text part as the window's impl, font 5, the bytes 1, 1 as 0xc00.
void label(u8 *window) {
    u8 *const memory = static_cast<u8 *>(g_env->alloc(0x1a4));
    g_env->setImpl(window, nullptr, memory ? g_env->textPart(memory, nullptr, 0) : nullptr);
    g_env->set4c(window, nullptr, g_env->font(5));
    g_env->notifyC00(window, nullptr, 1, 1);
}

void finish(u8 *window, u32 slot, u32 handler) {
    g_env->yellow(window);
    notifyColour(window, reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c0), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c4), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c8));
    g_env->setDwordA0(window, nullptr, 0, slot);
    g_env->setHandler(window, nullptr, handler);
}

void slotRow(u8 *dialog, u32 id, u32 slot) {
    u8 *const row = g_env->child(dialog, nullptr, id);
    u8 *const window = g_env->child(row, nullptr, 0x7a15);
    label(window);
    vcall<void>(window, 0x24, 0u);
    finish(window, slot, 0x004a6000);
}

}  // namespace

void Dialog7cb3f8_SetEnv(const Dialog7cb3f8Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cb3f8_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        g_env->close(0);
        return 0;
    }
    if (message == 0x14) global<u32>(0x007cb3f8) = 0;
    return g_env->callDefault(window, nullptr, message, a, b);
}

s32 __cdecl Dialog7cb3f8_SlotHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x100) {
        const u32 slot = g_env->dwordA0(window, nullptr, 0);
        const u32 name = *reinterpret_cast<const u32 *>(g_env->ifUsed(g_env->slots(), nullptr, slot) + 8);
        const u32 tag[4] = {(u32)(uintptr_t)TextArgString_vtable, 0x5641524d, 4, name};
        g_env->format(gLanguage_78ba94, reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, 0x18f, tag);
        ask(reinterpret_cast<const void *>((uintptr_t)0x007ba058), g_env->dwordA0(window, nullptr, 0), 0x004a61b0);
        return 0;
    }
    if (message == 0x10006) {
        const u32 slot = g_env->dwordA0(window, nullptr, 0);
        if (*reinterpret_cast<const u32 *>(g_env->slots() + slot * 24 + 4) == 0) {
            g_env->openSlot(slot);
            return 0;
        }
        g_env->useSlot(g_env->slots(), nullptr, slot);
        g_env->close(0);
        if (global<u8>(0x0078d90e))
            g_env->managerThree();
        else
            g_env->managerFour();
        return 0;
    }
    if (message == 0x10001) {
        notifyColour(window, reinterpret_cast<uintptr_t>(&gDialog7cb3f8_SlotHandler_7540cc), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_SlotHandler_7540d0), 0x007cb3f4);
    } else if (message == 0x10002) {
        notifyColour(window, reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c0), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c4), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c8));
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}

u32 __cdecl Dialog7cb3f8_ClearConfirmed(u32 slot) {
    g_env->clearSlot(g_env->slots(), nullptr, slot);
    g_env->refresh();
    return 1;
}

s32 __cdecl Dialog7cb3f8_DoneHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10006) {
        ask(g_env->text(9), 0, 0x004a6290);
        return 0;
    }
    if (message == 0x10001) {
        g_env->notify40000(window, nullptr, 0xff, 0, 0, 0xff);
    } else if (message == 0x10002) {
        notifyColour(window, reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c0), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c4), reinterpret_cast<uintptr_t>(&gDialog7cb3f8_7540c8));
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}

u32 __cdecl Dialog7cb3f8_DoneConfirmed() {
    g_env->setMode();
    g_env->close(1);
    return 1;
}

void __cdecl Dialog7cb3f8_Open() {
    g_env->resetKeys();
    g_env->setShown(*reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4), nullptr, 0);
    u8 *const dialog = g_env->build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00753c68), 0x004a5fc0);
    global<u8 *>(0x007cb3f8) = dialog;
    if (dialog) {
        g_env->setFocus(dialog, nullptr, 0);
        slotRow(global<u8 *>(0x007cb3f8), 0x7a14, 0);
        slotRow(global<u8 *>(0x007cb3f8), 0x7a19, 1);
        slotRow(global<u8 *>(0x007cb3f8), 0x7a1a, 2);
        slotRow(global<u8 *>(0x007cb3f8), 0x7a1b, 3);
        u8 *const done = g_env->child(global<u8 *>(0x007cb3f8), nullptr, 0x7a1c);
        label(done);
        vcall<void>(done, 0x24, reinterpret_cast<u32>(g_env->text(8)));
        finish(done, 3, 0x004a61d0);
        g_env->refresh();
    }
    u8 *records = gChatlists_f81ebc;
    if (!records) {
        u8 *const memory = static_cast<u8 *>(g_env->alloc(0x110));
        records = memory ? g_env->records(memory) : nullptr;
        gChatlists_f81ebc = records;
    }
    if (g_env->countUsed(records)) {
        g_env->push(*reinterpret_cast<u8 **>(global<u8 *>(0x00f85614) + 0x14), nullptr, 0x18e, 1);
        global<u32>(0x007cb3fc) = 0;
    } else {
        g_env->push(*reinterpret_cast<u8 **>(global<u8 *>(0x00f85614) + 0x14), nullptr, 0x186, 1);
        g_env->push(*reinterpret_cast<u8 **>(global<u8 *>(0x00f85614) + 0x14), nullptr, 0x187, 0);
        global<u32>(0x007cb3fc) = 1;
    }
    g_env->play(*reinterpret_cast<u8 **>(global<u8 *>(0x00f85614) + 4));
}
}
