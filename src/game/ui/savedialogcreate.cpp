// The save and load dialogs' making: see savedialogcreate.h.
#include "savedialogcreate.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "dialogbase.h"
#include "focus.h"
#include "hoverlabel.h"
#include "savedialog.h"
#include "window.h"
#include "window7059f0.h"
#include "window705a20.h"
#include "windowreg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const SaveDialogCreateEnv kGameEnv = {game<void(__fastcall *)(u8 *, void *, u32, u32)>(0x004092a0), &Window7c24c8_Close, &Dialog_Create, &Window_SetFocus, &Language_WideText, &HoverLabel_Attach, &Window_SetShown, &Window7059f0_BuildRows, &Window7059f0_SetColumnBottom, &GuiWindows_FindByName,
                                      &Window7059f0_SetHighlight, &LoadDialog_FillList, &SaveDialog_FillList, &TextPart6ff4d0_Ctor, &Window_SetImpl, &Window_Set4c, &Language_Font, &Window705a20_SetBuffer, &Window705a20_SetCaret, &Fn_66781a, &KeyMaps_ResetAllAndSetFlag};
const SaveDialogCreateEnv *g_env = &kGameEnv;

const u32 kLoadDialog = reinterpret_cast<uintptr_t>(&gSavedialog_7cb244);
const u32 kSaveDialog = reinterpret_cast<uintptr_t>(&gSavedialog_7cb248);
const char *hilight() { return reinterpret_cast<const char *>((uintptr_t)0x0074fa84); }

// The controls off and any other window gone; the dialog made in its global, with its label.
u8 *Open(u32 dialog, u32 handler, u32 text) {
    g_env->disable(reinterpret_cast<u8 *>((uintptr_t)0x00786b68), nullptr, 0, 0);
    global<u32>(0x00786ba4) = 0;
    g_env->closeOther();
    u8 *const window = g_env->create(reinterpret_cast<u8 *>((uintptr_t)0x007523f0), handler, 1);
    global<u8 *>(dialog) = window;
    if (!window) return nullptr;
    g_env->setFocus(window, nullptr, 0);
    const u32 label = static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->wideText(text)));
    g_env->hover(global<u8 *>(dialog), 0x23bce1, label);
    return window;
}

void SetUpList(u8 *list) {
    g_env->buildRows(list, nullptr, game<u8 *(__cdecl *)()>(0x0049f0b0), game<void(__cdecl *)(u8 *)>(0x0049f170));   // (List7cb24c_NewRow, _InitRow: as the original pushes them)
    g_env->setColumn(list, nullptr, 0, 0);
    g_env->setColumn(list, nullptr, 1, 0);
    g_env->setHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(hilight())));
}

}  // namespace

void SaveDialogCreate_SetEnv(const SaveDialogCreateEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl LoadDialog_Create() {
    if (!Open(kLoadDialog, 0x0049e880, 0xca)) return;
    g_env->setShown(Window_FindChild(global<u8 *>(kLoadDialog), nullptr, 0xffffffff), nullptr, 0);
    g_env->setShown(Window_FindChild(global<u8 *>(kLoadDialog), nullptr, 0x23bce2), nullptr, 0);
    u8 *const list = Window_FindChild(global<u8 *>(kLoadDialog), nullptr, 0x23bce0);
    if (!list) return;
    SetUpList(list);
    g_env->fillLoad(list);
}

void __cdecl SaveDialog_Create() {
    if (!Open(kSaveDialog, 0x0049ea30, 0xc9)) return;
    u8 *const edit = Window_FindChild(global<u8 *>(kSaveDialog), nullptr, 0x23bce2);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(edit, nullptr, memory ? g_env->part(memory, nullptr, 0) : nullptr);
    g_env->set4c(edit, nullptr, g_env->font(7));
    g_env->setBuffer(edit, nullptr, 0x007ca720, 0xf);
    g_env->setCaret(edit, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(hilight())));
    const u32 text = static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->wideText(0xce)));
    vcall<void>(edit, 0x24, text);
    g_env->setRefused(edit, nullptr, 0x00752588);
    g_env->setFocus(edit, nullptr, 0);
    u8 *const list = Window_FindChild(global<u8 *>(kSaveDialog), nullptr, 0x23bce0);
    if (list) {
        SetUpList(list);
        g_env->fillSave(list);
    }
    g_env->keymaps();
}
}
