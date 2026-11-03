#include "panel7c2534.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/guitext.h"
#include "../render/textparts.h"
#include "guirect.h"
#include "textpool.h"
#include "window.h"
#include "windowfactory.h"

namespace {

const Panel7c2534Env kGameEnv = {GuiText_Measure, Window_GetRect, Window_CreateChild, Window_SetHandler, TextPool_Dup, Window_SetDwordA0, GameHeap_Alloc, TextPart6ff530_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window_NotifyC00, Window_Notify40000};
const Panel7c2534Env *g_env = &kGameEnv;

s32 screenWord(u32 address) { return *reinterpret_cast<s16 *>((uintptr_t)address); }

}  // namespace

void Panel7c2534_SetEnv(const Panel7c2534Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __fastcall Panel7c2534_AddRow(u8 *self, void *, s32 top, const u16 *text, u32 id, u32 font, u32 red, u32 green, u32 blue) {
    s32 box[4] = {0, 0, 0, 0};
    const s32 measureRect[4] = {0, 0, 0x800, 0x600};
    g_env->measure(text, measureRect, (s8)font, 0, 0, 0, box);
    u8 rect[12];
    GuiRect_Ctor(rect);
    const s32 width = (s32)((u32)(box[2] - box[0] + 1) << 0xb) / screenWord(0x00faa5c4);
    const s32 height = (s32)((u32)((box[3] - box[1] + 5) * 3) << 9) / screenWord(0x00faa5c0);
    u8 *const parent = *reinterpret_cast<u8 **>(self + 0x10);
    g_env->getRect(parent, nullptr, rect, 1);
    const s32 from = *reinterpret_cast<s16 *>(rect + 4);
    const s32 to = *reinterpret_cast<s16 *>(rect + 8);
    const s32 left = ((to - from) >> 1) - (width >> 1) + from;
    u8 *const child = g_env->create(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 1, 1, id, (u32)left, (u32)top, (u32)(left + width), (u32)(top + height));
    if (child) {
        g_env->setHandler(child, nullptr, *reinterpret_cast<u32 *>(self));
        g_env->setDwordA0(child, nullptr, 1, (u32)(uintptr_t)g_env->dup(text));
        u8 *const memory = static_cast<u8 *>(g_env->alloc(0x1a4));
        g_env->setImpl(child, nullptr, memory ? g_env->textPart(memory, nullptr, 0) : nullptr);
        g_env->set4c(child, nullptr, g_env->font(font));
        g_env->notifyC00(child, nullptr, 1, 1);
        vcall<void>(child, 0x24, text);
        g_env->notify40000(child, nullptr, red, green, blue, 0xff);
        ++*reinterpret_cast<u32 *>(self + 4);
    }
    GuiRect_Dtor(rect);
    return top + height;
}
}
