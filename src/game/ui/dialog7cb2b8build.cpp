// The game's main bar's windows: see dialog7cb2b8build.h.
#include "dialog7cb2b8build.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../gfx/guitext.h"
#include "../render/textparts.h"
#include "guirect.h"
#include "window.h"
#include "windowscript.h"
#include "dialog74fb20.h"
#include "panel7b9940.h"
#include "window7cb2a4.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void __cdecl Measure(const u16 *text, const s32 *rect, u32 font, u32 hAlign, u32 vAlign, u32 fit, s32 *box) {
    GuiText_Measure(text, rect, (s8)font, (s8)hAlign, (s8)vAlign, fit, box);
}

const Dialog7cb2b8BuildEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &Measure, &BarMesh_Construct, &Window_SetObject98, &Window_SetHandler};
const Dialog7cb2b8BuildEnv *g_env = &kGameEnv;

using PartCtor = u8 *(__fastcall *)(u8 *, void *, u32);

u8 *newPart(PartCtor ctor) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    return memory ? ctor(memory, nullptr, 0) : nullptr;
}

void counter(u8 *child) {
    Window_SetDepth(child, nullptr, 0);
    Window_SetImpl(child, nullptr, newPart(TextPart_Ctor));
    Window_Notify40000(child, nullptr, 0xff, 0xff, 0xff, 0xff);
    Window_Set4c(child, nullptr, Language_Font(2));
    Window_NotifyImpl100(child, nullptr, 0);
    Window_NotifyC00(child, nullptr, 2, 1);
    vcall<void>(child, 0x24, (u32)(uintptr_t)&gText_Dialog7cb2b8build_752f24);
}

void buildRepair(u8 *bar) {   // (the child 0x2d and its 0x2e)
    u8 *const panel = gDialog7cb2b8build_7cb2c4 = bar;
    if (!panel) return;
    u8 *const inner = Window_FindChild(panel, nullptr, 0x2e);
    if (!inner) return;
    g_env->setHandler(panel, nullptr, 0x004a11e0);
    u8 outer[12], own[12];
    GuiRect_Ctor(outer);
    GuiRect_Ctor(own);
    Window_GetRect(panel, nullptr, outer, 1);
    Window_GetRect(inner, nullptr, own, 1);
    Window_Offset(inner, nullptr, 0, (u32)(s32)(s16)(at<u16>(outer, 6) - at<u16>(own, 0xa)));
    g_env->setHandler(inner, nullptr, 0x004a1190);
    at<u16>(outer, 6) = (u16)(at<u16>(outer, 0xa) - 8);
    Window_SetRect(panel, nullptr, outer);
    GuiRect_Dtor(own);
    GuiRect_Dtor(outer);
}

void buildMoney(u8 *window) {   // (the child 0x2f)
    Window_SetDepth(window, nullptr, 0);
    g_env->setHandler(window, nullptr, 0x004a0db0);
    s32 box[4] = {0, 0, 0, 0};
    u8 rect[12], slot[12];
    GuiRect_Ctor(rect);
    GuiRect_Ctor(slot);
    u32 align = 0;
    Language_MirrorAlign(&align);
    const s32 bounds[4] = {0, 0, 0xfde8, 0xfde8};
    g_env->measure(reinterpret_cast<const u16 *>((uintptr_t)&gText_Dialog7cb2b8build_752f10), bounds, 1, align, 0, 0, box);
    const s32 height = box[3] * 0x600 / global<s16>(0x00faa5c0) + 0x10;
    Window_GetRect(window, nullptr, rect, 1);
    Window_GetRect(Window_FindChild(window, nullptr, 0x32), nullptr, slot, 1);
    at<u16>(rect, 8) = (u16)((box[2] << 0xb) / global<s16>(0x00faa5c4) + at<u16>(rect, 4));
    const s16 top = at<s16>(slot, 6);
    at<u16>(rect, 6) = (u16)((at<s16>(slot, 0xa) - top) / 2 - height / 2 + top);
    at<u16>(rect, 0xa) = (u16)(height + at<u16>(rect, 6));
    Window_SetRect(window, nullptr, rect);
    Window_SetImpl(window, nullptr, newPart(TextPart6ff350_Ctor));
    Window_Set4c(window, nullptr, Language_Font(1));
    Window_NotifyC00(window, nullptr, align, 1);
    Window_Notify40000(window, nullptr, 0xff, 0xff, 0xff, 0xff);
    align = 2;
    Language_MirrorAlign(&align);
    if (u8 *const label = Window_FindChild(window, nullptr, 0x30)) {
        Window_SetDepth(label, nullptr, 0);
        Window_SetImpl(label, nullptr, newPart(TextPart6ff3e0_Ctor));
        Window_Set4c(label, nullptr, Language_Font(2));
        Window_NotifyC00(label, nullptr, align, 1);
        Window_Notify40000(label, nullptr, 0xff, 0xff, 0, 0xff);
        Window_SetShown(label, nullptr, 0);
        vcall<void>(label, 0x14, 0u);
    }
    if (u8 *const other = Window_FindChild(window, nullptr, 0x31)) vcall<void>(other, 0x14, 0u);
    vcall<void>(window, 0x14, 0u);
    GuiRect_Dtor(slot);
    GuiRect_Dtor(rect);
}

void buildScroller(u8 *strip) {   // (the child 0x21, its 0x22 and that one's 0x23; `strip` not null)
    u8 *const middle = Window_FindChild(strip, nullptr, 0x22);
    if (!middle) return;
    u8 *const end = Window_FindChild(middle, nullptr, 0x23);
    if (!end) return;
    gDialog7cb2b8build_7cb2d0 = end;
    g_env->setHandler(strip, nullptr, 0x004a14f0);
    u8 rect[12], own[12];
    GuiRect_Ctor(rect);
    GuiRect_Ctor(own);
    Window_GetRect(strip, nullptr, rect, 1);
    Window_GetRect(end, nullptr, own, 1);
    at<u16>(rect, 8) = (u16)(at<u16>(rect, 8) + at<u16>(rect, 4) - at<u16>(own, 4));
    Window_SetRect(strip, nullptr, rect);
    Window_SetRect14(strip, nullptr, rect);
    Window_SetDepth(strip, nullptr, 4);
    Window_SetDepth(middle, nullptr, 4);
    Window_SetDepth(gDialog7cb2b8build_7cb2d0, nullptr, 9);
    if (u8 *const top = Window_FindChild(strip, nullptr, 0x24)) {
        Window_SetDepth(top, nullptr, 7);
        vcall<void>(top, 0x14, 0u);
    }
    GuiRect_Dtor(own);
    GuiRect_Dtor(rect);
}

}  // namespace

void Dialog7cb2b8_SetBuildEnv(const Dialog7cb2b8BuildEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cb2b8_Build() {
    if (reinterpret_cast<u32 &>(gDialog7cb2b8build_7cb2b8)) return;
    u8 frame[12];
    GuiRect_Ctor(frame);
    gDialog7cb2b8build_7cb28c = 1;
    gDialog7cb2b8build_7cb2a0 = 3;
    gDialog7cb2b8build_7cb290 = 0xa;
    if (WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, gDialog7cb2b8_Build_752940, 0)) {
        u8 *const top = gDialog7cb2b8_Build_7cb2a4 = Window_FindChild(gPanel7b9940_7cb2ac, nullptr, 0x33);
        Window_SetDepth(top, nullptr, 0);
        vcall<void>(gDialog7cb2b8_Build_7cb2a4, 0x14, 0u);
        g_env->setHandler(gDialog7cb2b8_Build_7cb2a4, nullptr, 0x004a0fb0);
        counter(Window_FindChild(gDialog7cb2b8_Build_7cb2a4, nullptr, 0x37));
        u8 &guard = gDialog7cb2b8build_fb34c1;
        if (!(guard & 1)) {
            guard |= 1;
            Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
            g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
        }
        if (gDialog74fb20_fb3b7c != 2) {
            counter(Window_FindChild(gDialog7cb2b8_Build_7cb2a4, nullptr, 0x36));
            Window_Notify1(Window_FindChild(gDialog7cb2b8_Build_7cb2a4, nullptr, 0x34), nullptr, 3);
        } else {
            Window_SetShown(Window_FindChild(gDialog7cb2b8_Build_7cb2a4, nullptr, 0x34), nullptr, 0);
        }
        buildRepair(Window_FindChild(gPanel7b9940_7cb2ac, nullptr, 0x2d));
        if (u8 *const money = Window_FindChild(gPanel7b9940_7cb2ac, nullptr, 0x2f)) buildMoney(money);
    }
    if (u8 *const dialog = Window_FindChild(gPanel7b9940_7cb2ac, nullptr, 0x1d)) {
        g_env->setHandler(dialog, nullptr, 0x004a0a20);
        u8 rect[12];
        GuiRect_Ctor(rect);
        Window_SetDepth(dialog, nullptr, 8);
        Window_GetRect(dialog, nullptr, rect, 1);
        if (u8 *const strip = Window_FindChild(dialog, nullptr, 0x21)) buildScroller(strip);
        u8 *const graph = Window_FindChild(dialog, nullptr, 0x1e);
        g_env->setHandler(graph, nullptr, 0x004a0a90);
        if (u8 *const bars = Window_FindChild(graph, nullptr, 0x1f)) {
            u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
            g_env->setObject(bars, nullptr, memory ? g_env->barMesh(memory, nullptr, reinterpret_cast<const char *>((uintptr_t)&gWindow7cb2a4_752ef8 + 0xc)) : nullptr);
            g_env->setHandler(bars, nullptr, 0x004a1cd0);
            vcall<void>(bars, 0x14, 0u);
        }
        u8 *const text = Window_FindChild(dialog, nullptr, 0x20);
        u8 *const part = newPart(TextPart_Ctor);
        g_env->setHandler(text, nullptr, 0x004a0a90);
        Window_SetImpl(text, nullptr, part);
        gDialog7cb2b8_Build_7cb2d4 = 3;
        Window_Set4c(text, nullptr, Language_Font(3));
        Window_Notify40000(text, nullptr, 0, 0, 0, 0xff);
        Window_NotifyC00(text, nullptr, 1, 1);
        g_env->setHandler(Window_FindChild(dialog, nullptr, 0x25), nullptr, 0x004a0840);
        gDialog7cb2b8build_7cb2b8 = dialog;
        GuiRect_Dtor(rect);
    }
    GuiRect_Dtor(frame);
}
