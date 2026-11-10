// The dialog [0x7cc944]: see dialog7cc944.h.
#include "dialog7cc944.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../frontend/scenequeue.h"
#include "../online/session.h"
#include "../render/textparts.h"
#include "buttontext.h"
#include "hoverlabel.h"
#include "listrows.h"
#include "window.h"
#include "window7059f0.h"
#include "window705a20.h"
#include "windowreg.h"
#include "windowscript.h"
#include "window7cc8c0handler.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl FormatWide(u8 *charset, u16 *dest, u32 size, u32 entry) { return Charset_FormatWide(charset, dest, size, entry); }

const Dialog7cc944Env kGameEnv = {&Window705a20_SetBuffer, &Window705a20_SetCaret, &Window_SetFocus, &Window7059f0_BuildRows, &Window7059f0_SetColumnBottom, &Window7059f0_SetHighlight, &ButtonText_Init, &Window7059f0_AddItem, &FormatWide, &HoverLabel_Attach, &Class702da0_Push};
const Dialog7cc944Env *g_env = &kGameEnv;

u8 *hilight() { return reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))); }

}  // namespace

void Dialog7cc944_SetEnv(const Dialog7cc944Env *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc944_Open(u32 value) {
    if (global<u8 *>(0x007cc944)) return;
    u8 *const built = WindowScript_Build(*reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00758f40), 0x004bf9f0);
    global<u32>(0x007cc948) = value;
    global<u8 *>(0x007cc944) = built;
    u8 *const edit = Window_FindChild(built, nullptr, 0xe05a);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(edit, nullptr, memory ? TextPart6ff4d0_Ctor(memory, nullptr, 0) : nullptr);
    Window_Set4c(edit, nullptr, Language_Font(6));
    g_env->setBuffer(edit, nullptr, reinterpret_cast<uintptr_t>(&gDialog7cc944_7cc908), 0xf);
    g_env->setCaret(edit, nullptr, hilight());
    vcall<void>(edit, 0x24, (u32)(uintptr_t)Online_String14(Online_Instance()));
    g_env->setFocus(edit, nullptr, 0);
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc944), nullptr, 0xe05b);
    g_env->buildRows(list, nullptr, reinterpret_cast<u8 *(__cdecl *)()>((uintptr_t)0x004c02d0), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004c03e0));
    for (u32 column = 0; column < 6; ++column) g_env->setColumn(list, nullptr, column, column >= 3);
    g_env->setHighlight(list, nullptr, hilight());
    for (u32 i = 0; i < 6; ++i) {
        const u16 *const text = Language_WideText(0x12c + i);
        g_env->buttonText(Window_FindChild(list, nullptr, 0x10 + i), (u32)(uintptr_t)text, 8, 0xff, 0xef, 0, 0x17d + i);
    }
    const u32 texts[6] = {(u32)(uintptr_t)Language_WideText(0x133), reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374), reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374), 0, 0, 0};
    g_env->add(list, nullptr, texts, 0, -1, 1);
    u16 *const buffer = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
    g_env->format(global<u8 *>(0x0078ba94), buffer, 0x3fff, 0x12b);
    g_env->attach(global<u8 *>(0x007cc944), 0xe05c, 0x007ba058);
    g_env->push(*reinterpret_cast<u8 **>(global<u8 *>(0x00f85614) + 0x14), nullptr, 0x254, 1);
}
