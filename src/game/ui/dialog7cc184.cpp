// The dialog [0x7cc184]: see dialog7cc184.h.
#include "dialog7cc184.h"

#include <cstdint>

#include "../core/dequeiter.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../gfx/imageops.h"
#include "../online/session.h"
#include "../render/renderer.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "focus.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"
#include "windowscript.h"
#include "../online/chatcommand.h"
#include "dialog7cc184handler.h"
#include "dialog74fb20.h"
#include "gamepause.h"
#include "window7cc8bc.h"
#include "panel7b9940.h"
#include "dialog7cc188.h"
#include "dialog7cc654.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cc184OpenEnv kGameEnv = {Setting_ReadMode, GameAddress<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0), GameHeap_Alloc, GameAddress<u8 *(__fastcall *)(u8 *, void *)>(0x005b50b0), Online_Restricted,
                                      CommandQueue_Post, Window7c24c8_Close, Window7c24c8_Set, WindowScript_Build, Language_WideText, HoverLabel_Attach, TextPart6ff4d0_Ctor, Window705a20_SetBuffer, Window_SetImpl, Language_Font, Window_Set4c,
                                      Window_Notify40000, Window_Notify3000, Window_NotifyC00, GuiWindows_FindByName, Window705a20_SetCaret, Window_SetHandler, RendererPart_Ctor, Window_ImplSlot7, Panel_Reset, Panel_ShrinkInto, Window_ImplSlot6, Advisor_PostId};
const Dialog7cc184OpenEnv *g_open = &kGameEnv;

bool settingIsOne() {
    u8 &guard = gGamepause_fb34bf;
    if (!(guard & 1)) {
        guard |= 1;
        g_open->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_open->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    return gDialog74fb20_fb3b7c == 1;
}

u8 *newEditPart() {
    u8 *const memory = static_cast<u8 *>(g_open->alloc(0x1a4));
    return memory ? g_open->editPart(memory, nullptr, 0) : nullptr;
}

void dressEdit(u8 *edit, u32 buffer, u32 dwords, u32 capacity, u32 a, u32 text, bool partFirst) {
    u8 *part = partFirst ? newEditPart() : nullptr;
    for (u32 i = 0; i < dwords; ++i) reinterpret_cast<u32 *>((uintptr_t)buffer)[i] = 0;
    g_open->setBuffer(edit, nullptr, buffer, capacity);
    if (!partFirst) part = newEditPart();
    g_open->setImpl(edit, nullptr, part);
    g_open->set4c(edit, nullptr, g_open->font(0xb));
    g_open->notify40000(edit, nullptr, 0xff, 0xff, 0xff, 0xff);
    g_open->notify3000(edit, nullptr, a, 0);
    g_open->notifyC00(edit, nullptr, 0, 0);
    g_open->setCaret(edit, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_open->findByName(reinterpret_cast<const char *>((uintptr_t)&gText_Dialog7cc188_74fa84))));
    vcall<void>(edit, 0x24, g_open->text(text));
}

void showPicture(u8 *picture) {
    g_open->setHandler(picture, nullptr, 0x004a7c50);
    u8 *const memory = static_cast<u8 *>(g_open->alloc(0xd8));
    g_open->setImpl(picture, nullptr, memory ? g_open->picturePart(memory, nullptr, 0) : nullptr);
    u8 rect[12];
    GuiRect_Ctor(rect);
    s32 width, height;
    u8 *const surface = reinterpret_cast<u8 *>((uintptr_t)g_open->slot7(picture, nullptr, (u32)(uintptr_t)&width, (u32)(uintptr_t)&height));
    if (surface && gDialog7cc184handler_7cc18c) {
        g_open->reset(gDialog7cc184handler_7cc18c);
        const u32 mode = global<u32>((u32)(uintptr_t)&gTexSettings_8bd550 + 0x4);
        if (mode == 2 || mode == 3) g_open->shrink(gDialog7cc184handler_7cc18c, nullptr, surface, width, height, mode == 2 ? 2 : 1);
        g_open->slot6(picture);
    }
    GuiRect_Dtor(rect);
}

}  // namespace

void Dialog7cc184_SetOpenEnv(const Dialog7cc184OpenEnv *env) { g_open = env ? env : &kGameEnv; }

extern "C" void __cdecl Dialog7cc184_Open() {
    u8 *&dialog = gDialog7cc184handler_7cc184;
    if (dialog) return;
    u8 *&session = gChatcommand_f7d88c;
    if (!session) {
        u8 *const memory = static_cast<u8 *>(g_open->alloc(0x404));
        session = memory ? g_open->holderCtor(memory, nullptr) : nullptr;
    }
    if (!g_open->restricted(session)) {
        g_open->post(0x4e, 0);
        return;
    }
    u8 *parent;
    if (settingIsOne()) {
        parent = gWindow7cc8bc_7cc8c0;
    } else {
        g_open->closeModal();
        parent = gPanel7b9940_7cb2ac;
    }
    if (u8 *const window = g_open->build(parent, nullptr, gDialog7cc184_Open_7540d8, 0x004a7d70)) {
        dialog = window;
        if (!settingIsOne()) g_open->setModal(dialog);
        g_open->attach(dialog, 0x5d9e, (u32)(uintptr_t)g_open->text(0xd0));
        if (u8 *const edit = Window_FindChild(dialog, nullptr, 0x5da5)) dressEdit(edit, (u32)(uintptr_t)&gDialog7cc654_7cb4d0, 0x100, 0x1ff, 1, 0xd3, true);
        if (u8 *const edit = Window_FindChild(dialog, nullptr, 0x5da4)) dressEdit(edit, 0x007cb408, 0x30, 0x5f, 0, 0xd1, false);
        if (u8 *const edit = Window_FindChild(dialog, nullptr, 0x5da3)) dressEdit(edit, (u32)(uintptr_t)&gDialog7cc184_7cbf48, 0x80, 0xff, 1, 0xd2, false);
        if (u8 *const picture = Window_FindChild(dialog, nullptr, 0x5d9f)) showPicture(picture);
    }
    gDialog7cc184_7cbf40 = 0;
    gDialog7cc184_7cbf3c = 0;
    if (!settingIsOne()) g_open->advisor(0x133);
}
