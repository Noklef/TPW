#include "dialog7cc61c.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/guitext.h"
#include "dialogbase.h"
#include "focus.h"
#include "guirect.h"
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
