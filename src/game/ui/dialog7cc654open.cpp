// The window [0x7cc654] opened: see dialog7cc654open.h.
#include "dialog7cc654open.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/string.h"
#include "../files/charsets.h"
#include "../files/userslots.h"
#include "../frontend/scenequeue.h"
#include "../gfx/surface.h"
#include "../render/renderer.h"
#include "../render/textparts.h"
#include "buttontext.h"
#include "dialogbase.h"
#include "hoverlabel.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "../core/textarg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cc654OpenEnv kGameEnv = {&Dialog_Create, &TextPart_Ctor, &RendererPart_Ctor, &Window_SetImpl, &Language_Font, &Window_Set4c, &Window_NotifyImpl100, &Window_Notify3000, &Window_NotifyC00, &Window_SetHandler, &SurfaceInfo_Alloc,
                                      &Window7059f0_BuildRows, &Window7059f0_SetColumnBottom, &GuiWindows_FindByName, &Window7059f0_SetHighlight, &Language_WideText, &ButtonText_Init, &HoverLabel_Attach,
                                      game<u8(__fastcall *)(u8 *)>(0x005cc5e0), game<u8(__fastcall *)(u8 *)>(0x005cc600), reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, const void *, const void *, const void *)>(&Charset_FormatWide),
                                      &Window7059f0_AddItem, &Window7059f0_RefreshRows, &Window_SetFocus, &Class702da0_Push};
const Dialog7cc654OpenEnv *g_env = &kGameEnv;

struct NumberArg {
    u32 vtable;
    u32 tag;
    u32 number;
    u32 value;
    u32 zero;
};

void headText(u8 *list, u32 id, u32 text, u32 value) {
    u8 *const child = Window_FindChild(list, nullptr, id);
    g_env->buttonText(child, (u32)(uintptr_t)g_env->wideText(text), 8, 0xff, 0xef, 0, value);
}

void addRows(u8 *list, u8 *lists) {
    g_env->first(lists);
    u32 index = 0;
    do {
        const char *const name = at<const char *>(lists, 0x24);
        u32 texts[4];
        texts[0] = (u32)(uintptr_t)String_AnsiToWide(name ? name : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0), nullptr, 0);
        NumberArg args[3];
        for (u32 i = 0; i < 3; ++i) args[i].vtable = (u32)(uintptr_t)TextArgInt_vtable, args[i].tag = 0x5641524d, args[i].zero = 0;
        args[0].number = 0x12;
        args[0].value = at<u32>(lists, 0x54);
        args[1].number = 0x10;
        args[1].value = at<u32>(lists, 0x4c);
        args[2].number = 0x11;
        args[2].value = at<u32>(lists, 0x50);
        u16 text[0x20];
        g_env->format(global<u8 *>(0x0078ba94), text, 0x1f, 0x1c0, &args[1], &args[2], &args[0]);
        texts[1] = (u32)(uintptr_t)text;
        texts[2] = 1;
        texts[3] = 0;
        g_env->add(list, nullptr, texts, index | 0x1000000, -1, 0);
        ++index;
    } while (g_env->next(lists));
}

}  // namespace

void Dialog7cc654Open_SetEnv(const Dialog7cc654OpenEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cc654_Open() {
    if (!at<u32>(UserSlots_Instance(), 0xc4)) return 0;
    u8 *&window = global<u8 *>(0x007cc654);
    window = g_env->create(gDialog7cc654_Open_758878, 0x004bbc20, 1);

    u8 *child = Window_FindChild(window, nullptr, 0xee2);
    u8 *memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(child, nullptr, memory ? g_env->textPart(memory, nullptr, 0) : nullptr);
    g_env->set4c(child, nullptr, g_env->font(6));
    g_env->notifyImpl100(child, nullptr, 0);
    g_env->notify3000(child, nullptr, 1, 0);
    g_env->notifyC00(child, nullptr, 0, 0);
    g_env->setHandler(child, nullptr, 0x004bcb70);

    child = Window_FindChild(window, nullptr, 0xee4);
    memory = static_cast<u8 *>(GameHeap_Alloc(0xd8));
    g_env->setImpl(child, nullptr, memory ? g_env->rendererPart(memory, nullptr, 0) : nullptr);
    s32 size;
    g_env->surface(reinterpret_cast<SurfaceInfo *>((uintptr_t)0x007cc628), nullptr, &size);

    u8 *const list = Window_FindChild(window, nullptr, 0xee8);
    g_env->buildRows(list, nullptr, game<u8 *(__cdecl *)()>(0x004bcbc0), game<void(__cdecl *)(u8 *)>(0x004bcca0));
    g_env->columnBottom(list, nullptr, 0, 0);
    g_env->columnBottom(list, nullptr, 1, 0);
    g_env->columnBottom(list, nullptr, 2, 1);
    g_env->columnBottom(list, nullptr, 3, 1);
    g_env->setHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(reinterpret_cast<const char *>((uintptr_t)0x0074fa84))));
    headText(list, 0x10, 0x136, 0x187);
    headText(list, 0x11, 0x137, 0x188);
    headText(list, 0x12, 0x138, 0x189);
    headText(list, 0x13, 0x139, 0x18a);
    const u32 hoverText = (u32)(uintptr_t)g_env->wideText(0x135);
    g_env->hover(window, 0xee9, hoverText);

    u8 *const lists = UserSlots_Instance() + 0x64;
    u8 *const rows = Window_FindChild(window, nullptr, 0xee8);
    addRows(rows, lists);
    g_env->refresh(rows);
    g_env->focus(window, nullptr, 0);
    g_env->push(at<u8 *>(global<u8 *>(0x00f85614), 0x14), nullptr, 0x255, 1);
    return 1;
}
}
