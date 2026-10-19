#include "dialogs.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/particles.h"
#include "guirect.h"
#include "window.h"
#include "window705a50.h"

namespace {

u8 *window(u32 address) { return *reinterpret_cast<u8 **>((uintptr_t)address); }
}

extern "C" {

s32 __cdecl Dialog7cc47c_Message4() { return Window_SendMessage(window(0x007cc47c), 4, 0, 0); }

s32 __cdecl Dialog7cc47c_Message6On() { return Window_SendMessage(window(0x007cc47c), 6, 1, 0); }

s32 __cdecl Dialog7cc47c_Message6Off() { return Window_SendMessage(window(0x007cc47c), 6, 0, 0); }

s32 __cdecl Dialog7cc4b4_Message4() { return Window_SendMessage(window(0x007cc4b4), 4, 0, 0); }

s32 __cdecl Dialog7cc4b4_Message6Off() { return Window_SendMessage(window(0x007cc4b4), 6, 0, 0); }

void __cdecl Dialog7cc4b4_SetEffect(u8 *window, u32 on) {
    u32 &effect = *reinterpret_cast<u32 *>(0x007cc4bc);
    if ((u8)on == 0) {
        if (effect != 0) {
            ParticleSystem_End(effect);
            effect = 0;
        }
        return;
    }
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect(window, nullptr, rect, 1);
    const s32 top = *reinterpret_cast<s16 *>(rect + 6), bottom = *reinterpret_cast<s16 *>(rect + 0xa);
    const s32 centre = top + ((bottom - top) >> 1);
    const s32 z = (s32)((u32)centre * 75000u) / 1536;
    if (effect == 0) effect = (u32)Particle_Spawn(0x61, 50000, 0, z);
    GuiRect_Dtor(rect);
}

s32 __cdecl Dialog7cc4c4_Message4() { return Window_SendMessage(window(0x007cc4c4), 4, 0, 0); }

s32 __cdecl Dialog7cc4c4_Message6On() { return Window_SendMessage(window(0x007cc4c4), 6, 1, 0); }

s32 __cdecl Dialog7cdb0c_Message4() { return Window_SendMessage(window(0x007cdb0c), 4, 0, 0); }

s32 __cdecl Dialog7cdb0c_Message6On() { return Window_SendMessage(window(0x007cdb0c), 6, 1, 0); }

s32 __cdecl Dialog7cdb0c_Message6Off() { return Window_SendMessage(window(0x007cdb0c), 6, 0, 0); }

s32 __cdecl Dialog7cdb0c_Child16007Message6On() {
    return Window_SendMessage(Window_FindChild(window(0x007cdb0c), nullptr, 0x16007), 6, 1, 0);
}

s32 __cdecl Dialog7cdb0c_Child16007Message6Off() {
    return Window_SendMessage(Window_FindChild(window(0x007cdb0c), nullptr, 0x16007), 6, 0, 0);
}

s32 __cdecl Dialog7ca098_Message4IfUp() {
    s32 result = 0;
    if (u8 *const w = window(0x007ca098)) result = Window_SendMessage(w, 4, 0, 0);
    *reinterpret_cast<u32 *>((uintptr_t)0x007ca090) = 0;
    return result;
}

s32 __cdecl Dialog7cca94_Message4IfUp() {
    u8 *const w = window(0x007cca94);
    return w ? Window_SendMessage(w, 4, 0, 0) : 0;
}

s32 __fastcall WindowAt10_Message4(u8 *object) { return Window_SendMessage(*reinterpret_cast<u8 **>(object + 0x10), 4, 0, 0); }

s32 __fastcall WindowAt10_Message4IfAny(u8 *object) {
    u8 *const w = *reinterpret_cast<u8 **>(object + 0x10);
    return w ? Window_SendMessage(w, 4, 0, 0) : 0;
}

u32 __cdecl Dialog7ca168_ShowText(u32 which) {
    static const u32 kTexts[5] = {0x6b, 0x6c, 0x6e, 0x6d, 0x6f};
    u8 *const label = Window_FindChild(Window_FindChild(window(0x007ca168), nullptr, 0x329), nullptr, 0x32a);
    if (which > 4) return which;
    return vcall<u32>(label, 0x24, Language_WideText(kTexts[which]));
}

s32 __cdecl Dialog7cb2b8_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) *reinterpret_cast<u32 *>(0x007cb2b8) = 0;
    return Window_CallDefault(window, nullptr, message, a, b);
}

s32 __cdecl Window_DefaultHandler(u8 *window, u32 message, u32 a, u32 b) { return Window_CallDefault(window, nullptr, message, a, b); }

s32 __cdecl WindowAt10_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) *reinterpret_cast<u32 *>(Window_DwordA0(window, nullptr, 0) + 0x10) = 0;
    return Window_CallDefault(window, nullptr, message, a, b);
}

u8 *__fastcall WindowAt10_ScalarDtor(u8 *object, void *, u32 flags) {
    WindowAt10_Message4IfAny(object);
    if (flags & 1) GameHeap_Free(object);
    return object;
}

s32 __cdecl Windows7cc8bc_Message6Pair() {
    u8 *const first = window(0x007cc8bc);
    u32 &chosen = *reinterpret_cast<u32 *>((uintptr_t)0x007cc698);
    const bool same = chosen == (u32)(uintptr_t)first;
    Window_SendMessage(first, 6, same, 0);
    const s32 result = Window_SendMessage(window(0x007cc8c0), 6, !same, 0);
    chosen = 0;
    return result;
}

u32 __cdecl Windows7cc8bc_SelectSecond() {
    Window_SendMessage(window(0x007cc8bc), 6, 0, 0);
    Window_SendMessage(window(0x007cc8c0), 6, 1, 0);
    *reinterpret_cast<u8 *>((uintptr_t)0x007cc6a0) = 1;
    return Window_SetFocus(window(0x007cc8c0), nullptr, 0);
}

void __cdecl Windows7c26b0_Message5() {
    for (u32 entry = 0x007c26b0; entry < 0x007c3b00; entry += 0x208)
        if (*reinterpret_cast<u16 *>((uintptr_t)entry + 4)) Window_SendMessage(*reinterpret_cast<u8 **>((uintptr_t)entry), 5, 0, 0);
}

u32 __cdecl Dialog7cb2b8_Child21Post(u32 value) {
    u8 *const dialog = window(0x007cb2b8);
    if (!dialog || !value) return value;
    u8 *const child = Window_FindChild(dialog, nullptr, 0x21);
    if (!child) return value;
    if (Window_DwordA0(child, nullptr, 2)) return 0;
    Window_SendMessage(child, 0xb, 0, 0);
    Window_PostMessage(child, 0xa, value, 0);
    return value;
}

u32 __cdecl Global7cc698_Choose() {
    u32 &chosen = *reinterpret_cast<u32 *>((uintptr_t)0x007cc698);
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x007cc8b8)) {
        chosen = *reinterpret_cast<const u32 *>((uintptr_t)0x007cc8b8);
        return chosen;
    }
    if (*reinterpret_cast<const u8 *>((uintptr_t)0x007cc8b4)) {
        chosen = *reinterpret_cast<const u32 *>((uintptr_t)0x007cc8bc);
        return chosen;
    }
    chosen = *reinterpret_cast<const u32 *>((uintptr_t)0x007cc8c0);
    return 0;
}

void __cdecl Window7cc8bc_SetC3128(u8 value) {
    if (u8 *const w = window(0x007cc8bc)) {
        u8 *const child = Window_FindChild(Window_FindChild(Window_FindChild(w, nullptr, 0xc3123), nullptr, 0xc3124), nullptr, 0xc3128);
        Window705a50_SetState(child, nullptr, value == 1 ? 1 : 0, 1);
    }
}

void __cdecl Window7cc8bc_SetC3129(u8 value) {
    if (u8 *const w = window(0x007cc8bc)) {
        u8 *const child = Window_FindChild(Window_FindChild(Window_FindChild(w, nullptr, 0xc3123), nullptr, 0xc3124), nullptr, 0xc3129);
        vcall<void>(child, 0x14, (u32)(value == 0 ? 1 : 0));
    }
}

void __cdecl Window7ca2e8_ClearButtons(u32 mode) {
    u8 *const panel = Window_FindChild(window(0x007ca2e8), nullptr, 0x12272);
    static const u32 kThree[] = {0x12277, 0x1227b};
    static const u32 kOther[] = {0x12279, 0x12278, 0x1227c, 0x1227d, 0x1227a, 0x1227e};
    const u32 *const ids = mode == 3 ? kThree : kOther;
    const u32 n = mode == 3 ? 2 : 6;
    for (u32 i = 0; i < n; i++)
        Window705a50_SetState(Window_FindChild(panel, nullptr, ids[i]), nullptr, 0, 1);
}
}
