#include "dialog7cc61c.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/guitext.h"
#include "../render/textparts.h"
#include "dialogbase.h"
#include "focus.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "radiogroup.h"
#include "window.h"
#include "window705b20.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *Owner() { return *reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4); }

void __cdecl Measure(const u16 *text, const s32 *rect, u32 font, u32 hAlign, u32 vAlign, u32 fit, s32 *box) {
    GuiText_Measure(text, rect, (s8)font, (s8)hAlign, (s8)vAlign, fit, box);
}

const Dialog7cc61cTextEnv kGameTextEnv = {&GameHeap_Alloc, reinterpret_cast<u8 *(__fastcall *)(u8 *, void *)>((uintptr_t)0x005b50b0), &Measure, &Language_Font, &Window_SetShown, &Window705b20_SetRange};
const Dialog7cc61cTextEnv *g_textEnv = &kGameTextEnv;

u8 *Holder() {
    u8 *&holder = global<u8 *>(0x00f7d88c);
    if (!holder) {
        u8 *const memory = static_cast<u8 *>(g_textEnv->alloc(0x404));
        holder = memory ? g_textEnv->holderCtor(memory, nullptr) : nullptr;
    }
    return holder;
}

}  // namespace

void Dialog7cc61c_SetTextEnv(const Dialog7cc61cTextEnv *env) { g_textEnv = env ? env : &kGameTextEnv; }

extern "C" void __cdecl Dialog7cc61c_FitText() {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007cc61c), nullptr, 0x5b7f);
    u8 *const bar = Window_FindChild(panel, nullptr, 0x5b80);
    u8 rect[12];
    GuiRect_Ctor(rect);
    const bool first = global<u8>(0x007cc620) != 0;
    u8 *const holder = Holder();
    const u16 *const text = at<const u16 *>(holder, first ? 0xa0 : 0xbc);
    Window_GetSavedRect(panel, nullptr, rect);
    const s32 scaleY = global<s16>(0x00faa5c0);
    const s32 bounds[4] = {0, 0, ((s32)at<s16>(rect, 8) - at<s16>(rect, 4)) * global<s16>(0x00faa5c4) / 0x800,
                           ((s32)at<s16>(rect, 0xa) - at<s16>(rect, 6)) * scaleY / 0x600};
    s32 box[4] = {0, 0, 0, 0};
    g_textEnv->measure(text, bounds, global<u8>(0x00758870), 0, 0, 1, box);
    const s32 room = (s32)at<s16>(rect, 0xa) - at<s16>(rect, 6);
    const s32 height = box[3] * 0x600 / global<s16>(0x00faa5c0);
    const s32 line = (s16)vcall<u32>(g_textEnv->font(global<u32>(0x00758870)), 4) * 0x600 / global<s16>(0x00faa5c0);
    if (height > room) {
        g_textEnv->setShown(bar, nullptr, 1);
        g_textEnv->setRange(bar, nullptr, (height - room) / line, 0);
    } else {
        g_textEnv->setShown(bar, nullptr, 0);
    }
    if (!(u16)vcall<u32>(bar, 0x1c, 0u))
        vcall<void>(panel, 0x24, text);
    GuiRect_Dtor(rect);
}

namespace {

const Dialog7cc61cOpenEnv kGameOpenEnv = {&Dialog_Create, &GameHeap_Alloc, &TextPart_Ctor, &Window_SetImpl, &Language_Font, &Window_Set4c, &Window_NotifyImpl100, &Window_Notify3000, &Window_NotifyC00, &Dialog7cc61c_FitText, &Window_SetHandler, &RadioGroup_Select, &Window_Notify40000, &Language_WideText, &HoverLabel_Attach, &Window_SetFocus};
const Dialog7cc61cOpenEnv *g_open = &kGameOpenEnv;

void setPart(u8 *window) {
    u8 *const memory = static_cast<u8 *>(g_open->alloc(0x1a4));
    g_open->setImpl(window, nullptr, memory ? g_open->part(memory, nullptr, 0) : nullptr);
}

void dressButton(u8 *group, u32 id, u32 text) {
    u8 *const button = Window_FindChild(group, nullptr, id);
    setPart(button);
    g_open->notifyC00(button, nullptr, 1, 1);
    g_open->set4c(button, nullptr, g_open->font(6));
    g_open->notify40000(button, nullptr, 0, 0, 0, 0xff);
    g_open->notify100(button, nullptr, 0);
    vcall<void>(button, 0x24, g_open->text(text));
}

}  // namespace

void Dialog7cc61c_SetOpenEnv(const Dialog7cc61cOpenEnv *env) { g_open = env ? env : &kGameOpenEnv; }

extern "C" void __cdecl Dialog7cc61c_Open() {
    u8 *&dialog = global<u8 *>(0x007cc61c);
    if (dialog || !global<u8 *>(0x00f82884)) return;
    dialog = g_open->create(reinterpret_cast<u8 *>((uintptr_t)0x007586f0), 0x004bb690, 0);
    Window_SetDepth(dialog, nullptr, (u16)(at<u16>(dialog, 0xe0) + 4));
    u8 *const panel = Window_FindChild(dialog, nullptr, 0x5b7f);
    setPart(panel);
    g_open->set4c(panel, nullptr, g_open->font(global<u32>(0x00758870)));
    g_open->notify100(panel, nullptr, 0);
    g_open->notify3000(panel, nullptr, 1, 0);
    g_open->notifyC00(panel, nullptr, 0, 0);
    global<u8>(0x007cc620) = 1;
    g_open->fitText();
    g_open->setHandler(panel, nullptr, 0x004bb720);
    u8 *const group = Window_FindChild(panel, nullptr, 0x5b81);
    g_open->select(group, nullptr, 0x5b82);
    dressButton(group, 0x5b82, 0x10c);
    dressButton(group, 0x5b83, 0x10d);
    g_open->attach(dialog, 0x5b84, (u32)(uintptr_t)g_open->text(0x10b));
    g_open->setFocus(dialog, nullptr, 0);
}

extern "C" s32 __cdecl Dialog7cc61c_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u8 *>(0x007cc61c) = nullptr;
        Window_SendMessage(Owner(), 6, 1, 0);
        Focus_Restore();
    } else if (message == 0x15) {
        Window_SendMessage(Owner(), 6, 0, 0);
    } else if (message == 0x100 && a == 0xffffffff) {
        Window_SendMessage(global<u8 *>(0x007cc61c), 4, 0, 0);
        return 0;
    }
    return Dialog_BaseHandler(window, message, a, b);
}
