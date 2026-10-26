#include "dialog7cb2b8.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/soundplay.h"
#include "../core/vcall.h"
#include "dialogs.h"
#include "guirect.h"
#include "window.h"
#include "windowtimers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *ptr(u32 value) { return reinterpret_cast<u8 *>((uintptr_t)value); }

// The tag shown when its inner right edge is past the limit's low word, then its children by Dialog7cb2b8_ShowRightOf (the loop the
// original has twice, after its own rectangle D).
void show_tag(u8 *tag, u32 limit) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    if (tag) {
        Window_GetRect(tag, nullptr, rect, 1);
        Window_SetShown(tag, nullptr, at<s16>(rect, 8) > (s16)limit ? 1 : 0);
    }
    for (u8 *child = ptr(Fn_65d547(tag, nullptr, 0)); child; child = ptr(Fn_65d56b(child))) Dialog7cb2b8_ShowRightOf(child, limit);
    GuiRect_Dtor(rect);
}

}  // namespace

extern "C" {

void __cdecl Dialog7cb2b8_ShowRightOf(u8 *window, u32 limit) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    if (window) {
        Window_GetRect(window, nullptr, rect, 1);
        Window_SetShown(window, nullptr, *reinterpret_cast<const s16 *>(rect + 8) > (s16)limit ? 1 : 0);
    }
    for (u8 *child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(window, nullptr, 0)); child;
         child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(child))) {
        Dialog7cb2b8_ShowRightOf(child, limit);
    }
    GuiRect_Dtor(rect);
}

s32 __cdecl Dialog7cb2b8_BoxHandler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0xa: {   // a tag to slide in (a the window)
        if (window[0x134]) {
            if (a) Window_SetDwordA0(window, nullptr, 2, a);
            break;
        }
        if (!(u16)Window_SetTimer(window, nullptr, 0x80080, 0x14)) break;
        u8 *const tag = ptr(a);
        u8 dialogRect[0xc], tagRect[0xc], rect[0xc];
        GuiRect_Ctor(dialogRect);
        GuiRect_Ctor(tagRect);
        GuiRect_Ctor(rect);
        u16 width = 0, height = 0;   // (the window and a argument slots: words written into them)
        at<u32>(window, 0x130) = 0xa;
        if ((u16)Window_AddChild(window, nullptr, tag, 1, 1)) {
            Window_SetDepth(tag, nullptr, (u16)(at<u16>(tag, 0xe0) - 3));
            Window_GetRect(global<u8 *>(0x007cb2b8), nullptr, dialogRect, 1);
            Window_GetRect(tag, nullptr, tagRect, 1);
            Window_GetRect(window, nullptr, rect, 1);
            GuiRect_GetSize(tagRect, nullptr, &width, &height);
            const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), bottom = at<s16>(rect, 0xa);
            const s32 h = (s16)height;
            const s32 x = (s16)(left - width - 0x32);
            const s32 y = (s16)(top + (bottom - top) / 2 - h / 2);
            Window_SetRectTo(tag, nullptr, (u32)x, (u32)y, (u32)(x + (s16)width), (u32)(y + h));
            Window_SetShown(tag, nullptr, 1);
            const u32 limit = at<u32>(rect, 4);   // (left, top: the left the limit's word)
            show_tag(tag, limit);
            const s32 span = (s16)(at<s16>(dialogRect, 8) - left + width + 0x64);
            Window_SetDwordA0(window, nullptr, 0, (u32)(s32)(s16)(span / at<s32>(window, 0x130)));
            Window_SetDwordA0(window, nullptr, 1, a);
            Window_StartTimer(window, nullptr, 0x80080);
            window[0x134] = 2;
            u8 *const close = Window_FindChild(window, nullptr, 0x24);
            if (close) vcall<void>(close, 0x14, 1u);
            Sound_PlayAt(0, global<u32>(0x00803a2c), 0x1c, 0, 0, 0);
            Window_SendMessage(tag, 0x1001d9, 0, 0);
        }
        GuiRect_Dtor(rect);
        GuiRect_Dtor(tagRect);
        GuiRect_Dtor(dialogRect);
        break;
    }
    case 0xb: {   // the tag slid out (a -1: then the parent posted 0xb)
        u8 *const tag = ptr(Window_DwordA0(window, nullptr, 1));
        if (!tag) break;
        if (!(u16)Window_SetTimer(window, nullptr, 0x80080, 0x14)) break;
        u8 rect[0xc], target[0xc];
        GuiRect_Ctor(rect);
        GuiRect_Ctor(target);
        Window_SendMessage(tag, 0x1001d8, 0, 0);
        Window_GetRect(global<u8 *>(0x007cb2d0), nullptr, target, 1);
        at<u32>(window, 0x130) = 0xa;
        Window_GetRect(window, nullptr, rect, 1);
        const s32 span = (s32)at<s16>(rect, 4) - (s32)at<s16>(target, 4);
        Window_SetDwordA0(window, nullptr, 0, (u32)(s32)(s16)(span / at<s32>(window, 0x130)));
        Window_StartTimer(window, nullptr, 0x80080);
        window[0x134] = a == 0xffffffff ? 4 : 3;
        u8 *const close = Window_FindChild(window, nullptr, 0x24);
        if (close) vcall<void>(close, 0x14, 0u);
        Sound_PlayAt(0, global<u32>(0x00803a2c), 0x1c, 0, 0, 0);
        GuiRect_Dtor(target);
        GuiRect_Dtor(rect);
        break;
    }
    case 0x10: {   // the slide's timer
        if (a != 0x80080) break;
        const u32 step = Window_DwordA0(window, nullptr, 0);
        u8 rect[0xc];
        GuiRect_Ctor(rect);
        const s32 count = at<s32>(window, 0x130);
        if (count == 0) {
            Window_StopTimer(window, nullptr, a);
            Window_GetRect14(window, nullptr, rect);
            u8 *const tag = ptr(Window_DwordA0(window, nullptr, 1));
            if (tag) {
                if (window[0x134] == 3) {
                    Window_SetShown(tag, nullptr, 0);
                    Window_Unlink(tag);
                    Window_SetDwordA0(window, nullptr, 1, 0);
                    Window_SetRect(window, nullptr, rect);
                    Window_SendMessage(tag, 0x1d, 0, 0);
                    SoundUi_Play(0xc2);
                } else if (window[0x134] == 4) {
                    Window_SetShown(tag, nullptr, 0);
                    Window_Unlink(tag);
                    Window_SetDwordA0(window, nullptr, 1, 0);
                    Window_SetRect(window, nullptr, rect);
                    Window_PostMessage(ptr(Fn_65d55a(window)), 0xb, 0, 0);
                } else {
                    Window_SendMessage(tag, 0x1d, 1, 0);
                }
            }
            window[0x134] = 0;
            const u32 next = Window_DwordA0(window, nullptr, 2);
            if (next) {
                Window_PostMessage(window, 0xa, next, 0);
                Window_SetDwordA0(window, nullptr, 2, 0);
            }
        } else {
            at<s32>(window, 0x130) = count - 1;
            Window_GetRect(window, nullptr, rect, 1);
            at<u16>(rect, 8) = (u16)(at<u16>(rect, 8) + (u16)step);   // (the right edge only)
            Window_SetRect(window, nullptr, rect);
        }
        GuiRect_Dtor(rect);
        break;
    }
    case 0x12: {   // the box moved across to a (the tag kept to its right edge)
        u8 *const tag = ptr(Window_DwordA0(window, nullptr, 1));
        if (!tag) break;
        u8 rect[0xc], parentRect[0xc];
        GuiRect_Ctor(rect);
        GuiRect_Ctor(parentRect);
        Window_GetRect20(window, nullptr, rect);
        u16 width = 0, height = 0;
        GuiRect_GetSize(rect, nullptr, &width, &height);
        // sic: the width word is written into a's stack slot and read back as a dword, a's upper half with it
        Window_Offset(tag, nullptr, a - ((a & 0xffff0000) | width), 0);
        Window_GetRect(tag, nullptr, rect, 1);
        Window_GetRect(ptr(Fn_65d55a(window)), nullptr, parentRect, 1);
        const u32 limit = at<u32>(parentRect, 8);   // (right, bottom: the right the limit's word)
        show_tag(tag, limit);
        GuiRect_Dtor(parentRect);
        GuiRect_Dtor(rect);
        break;
    }
    case 0x100:   // the close button 0x24
        if (a == 0x24) Dialog7cb2b8_ResetChild21(1);   // (inlined in the original)
        break;
    default:
        break;
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
}  // extern "C"
