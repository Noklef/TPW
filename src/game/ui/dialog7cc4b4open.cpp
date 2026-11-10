// The dialog [0x7cc4b4] built: see dialog7cc4b4open.h.
#include "dialog7cc4b4open.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../render/textparts.h"
#include "dialog7cc4b4update.h"
#include "window.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc4b4OpenEnv kGameEnv = {&WindowScript_Build, &TextPart_Ctor, &TextPart6ff530_Ctor, &Window_SetImpl, &Language_Font, &Window_Set4c, &Window_NotifyImpl100, &Window_NotifyC00, &Window_Notify1, &Window_SetShown, &Dialog7cc4b4_Update, &Window_Notify40000,
                                      reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x004bbbd0)};
const Dialog7cc4b4OpenEnv *g_env = &kGameEnv;

u8 *dialog() { return gDialog7cc4b4open_7cc4b4; }

u8 *child(u8 *window, u32 id) { return Window_FindChild(window, nullptr, id); }

}  // namespace

void Dialog7cc4b4Open_SetEnv(const Dialog7cc4b4OpenEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __cdecl Dialog7cc4b4_Open() {
    if (dialog()) return;
    u8 *const parent = at<u8 *>(global<u8 *>(0x00f82884), 4);
    if (!parent) return;
    gDialog7cc4b4open_7cc4b4 = g_env->build(parent, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00757f60), 0x004b8b70);

    u8 *const panel = child(dialog(), 0x1e0ec);
    u8 *const text = child(panel, 0x1e0ee);
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(text, nullptr, block ? g_env->textPart(block, nullptr, 0) : nullptr);
    g_env->set4c(text, nullptr, g_env->font(2));
    g_env->notify100(text, nullptr, 0);
    g_env->notifyC00(text, nullptr, 2, 1);
    g_env->notify1(child(panel, 0x1e0ed), nullptr, 3);
    g_env->setShown(panel, nullptr, 0);
    g_env->notify1(child(dialog(), 0x1e0ea), nullptr, 1);
    g_env->notify1(child(child(dialog(), 0x1e0ea), 0x1e0eb), nullptr, 5);
    reinterpret_cast<u32 &>(gDialog7cc4b4_7cc4b8) = 0;
    gDialog7cc4b4update_7cc4bc = 0;
    g_env->update();

    u8 *const note = child(dialog(), 0x1e0ef);
    block = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(note, nullptr, block ? g_env->textPart530(block, nullptr, 0) : nullptr);
    g_env->set4c(note, nullptr, g_env->font(0));
    g_env->notifyC00(note, nullptr, 1, 1);
    (*reinterpret_cast<void(__fastcall **)(u8 *, void *, u32)>(at<uintptr_t>(note, 0) + 0x24))(note, nullptr, 0x0075817c);
    g_env->notify40000(note, nullptr, 0xfc, 0xe8, 7, 0xff);
    g_env->finish();
}
