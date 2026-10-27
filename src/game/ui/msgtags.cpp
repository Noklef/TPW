#include "msgtags.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/soundplay.h"
#include "../core/vcall.h"
#include "../files/file.h"
#include "../render/particles.h"
#include "../world/advisor.h"
#include "dialogs.h"
#include "guirect.h"
#include "window.h"
#include "windowreg.h"
#include "windowtimers.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }
}  // namespace

extern "C" {

u8 *__cdecl MsgTag_NewWindow(u32 id) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x138));
    u8 *const window = memory ? Window_Ctor(memory, nullptr, nullptr, 0, id, 0, 0, 0x46, 0x41) : nullptr;
    if (window) {
        Window_SetObject98(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName("msgtag")));
        Window_SetHandler(window, nullptr, 0x0048dae0);
        *reinterpret_cast<u32 *>(window + 0x90) = 0x1db;
    }
    return window;
}

void __cdecl MsgTags_Create() {
    for (u32 i = 0; i < 10; i++) {
        u8 *const record = reinterpret_cast<u8 *>((uintptr_t)(0x007c26b0 + 0x208 * i));
        *reinterpret_cast<u16 *>(record + 4) = 0;
        u8 *const window = MsgTag_NewWindow(0x384 + i);
        *reinterpret_cast<u8 **>(record) = window;
        if (window) Window_SetDwordA0(window, nullptr, 2, (u32)(uintptr_t)(record + 6));
    }
}

void __cdecl MsgTag_Open(u8 *tag) {
    if (global<u8 *>(0x007ca090) == tag) return;
    if (tag[0x134]) {
        const u32 kind = Window_DwordA0(tag, nullptr, 6);
        if (kind != 0x1b) {
            Window_StopTimer(tag, nullptr, 0x80080);
            Window_StopTimer(tag, nullptr, 0x80081);
            tag[0x134] = 0;
        }
        const s32 system = (s32)Window_DwordA0(tag, nullptr, 9);
        if (system > -1 && kind != 0x1b && Particle_IsAlive((u32)system)) ParticleSystem_End((u32)system);
    }
    static const u32 kFrom[5] = {2, 3, 5, 6, 8};
    for (u32 i = 0; i < 5; i++) {
        const u32 value = Window_DwordA0(tag, nullptr, kFrom[i]);
        Window_SetDwordA0(global<u8 *>(0x007ca098), nullptr, i + 1, value);
    }
    Dialog7cb2b8_Child21Post((u32)(uintptr_t)global<u8 *>(0x007ca098));
    global<u8 *>(0x007ca094) = tag;
    Advisor_PostId(0x11c);
}

u8 *__cdecl MsgTags_Add(const u8 *message) {
    if (!global<u32>(0x007cb2ac)) return nullptr;
    bool fresh = true;
    for (u8 *tag = global<u8 *>(0x007cb2c8); tag; tag = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(tag))) {
        if (at<u32>(message, 4) != Window_DwordA0(tag, nullptr, 1)) continue;
        if (at<u16>(message, 0x1c) != (u16)Window_DwordA0(tag, nullptr, 3)) continue;
        if (at<u32>(message, 0x20) != Window_DwordA0(tag, nullptr, 8)) continue;
        if (at<u32>(message, 0) == at<u32>(message, 4)) {
            u16 *const text = reinterpret_cast<u16 *>((uintptr_t)Window_DwordA0(tag, nullptr, 2));
            Crt().wcsncpy(text, at<const u16 *>(message, 8), 0xff);
            text[0xff] = 0;
            if (tag == global<u8 *>(0x007ca090)) vcall<void>(Window_FindChild(global<u8 *>(0x007ca098), nullptr, 0x10d32), 0x24, text);
            fresh = false;
        } else {
            Window_SendMessage(global<u8 *>(0x007cb2c4), 0x100002, (u32)(uintptr_t)tag, 1);
        }
    }
    if (!fresh || global<s32>(0x007ca09c) >= global<s32>(0x00750610) || !global<u8 *>(0x007cb2c4)) return nullptr;
    s16 width;
    Window_GetSize(global<u8 *>(0x007cb2c4), nullptr, &width, nullptr, 1);
    u8 *tag = nullptr;
    for (u8 *record = reinterpret_cast<u8 *>((uintptr_t)0x007c26b0); record < reinterpret_cast<u8 *>((uintptr_t)0x007c3b00); record += 0x208) {
        if (*reinterpret_cast<u16 *>(record + 4)) continue;
        *reinterpret_cast<u16 *>(record + 4) = 1;
        tag = *reinterpret_cast<u8 **>(record);
        Window_Notify1(tag, nullptr, (u32)(s32)at<s16>(message, 0x10));
        break;
    }
    if (!tag) return nullptr;
    if (!global<u8 *>(0x007cb2c8)) global<u8 *>(0x007cb2c8) = tag;
    u16 *const text = reinterpret_cast<u16 *>((uintptr_t)Window_DwordA0(tag, nullptr, 2));
    if (at<const u16 *>(message, 8) && Crt().wcslen(at<const u16 *>(message, 8)) && text) {
        Crt().wcsncpy(text, at<const u16 *>(message, 8), 0xff);
        text[0xff] = 0;
    }
    if (at<s32>(message, 0xc) > 0) {
        Window_SetTimer(tag, nullptr, 0x80080, at<u32>(message, 0xc));
        Window_StartTimer(tag, nullptr, 0x80080);
        tag[0x134] = 1;
        Window_SetTimer(tag, nullptr, 0x80081, at<u32>(message, 0xc) - 5000);
        Window_StartTimer(tag, nullptr, 0x80081);
    }
    Window_SetDwordA0(tag, nullptr, 1, at<u32>(message, 0));
    Window_SetDwordA0(tag, nullptr, 3, at<u16>(message, 0x1c));
    Window_SetDwordA0(tag, nullptr, 5, at<u32>(message, 0x14));
    Window_SetDwordA0(tag, nullptr, 6, at<u32>(message, 0x18));
    Window_SetDwordA0(tag, nullptr, 4, (u32)(s32)at<s16>(message, 0x10));
    Window_SetDwordA0(tag, nullptr, 8, at<u32>(message, 0x20));
    Window_SetDwordA0(tag, nullptr, 7, at<u32>(message, 0xc));
    Window_SetDwordA0(tag, nullptr, 9, 0xffffffff);
    Window_SetDwordA0(tag, nullptr, 10, 0);
    Window_PostMessage(global<u8 *>(0x007cb2c4), 0x100001, (u32)(uintptr_t)tag, 0);
    global<s32>(0x007ca09c)++;
    Sound_PlayAt(0, global<u32>(0x00803a2c), 0x1e, 0, 0, 0);
    return tag;
}

u32 __cdecl MsgTags_Post(const u8 *message) {
    if (message[0x24] != 1) return MsgTags_Add(message) ? 1 : 0;
    u8 *const tag = MsgTags_Add(message);
    if (!tag) return 0;
    MsgTag_Open(tag);
    return 1;
}

void __cdecl MsgTags_AddVersion() {
    u16 text[16];
    u8 message[0x28];
    Crt().swprintf(text, reinterpret_cast<const u16 *>((uintptr_t)0x00750694), 2, 0);
    *reinterpret_cast<u32 *>(message + 0) = 0x17e;
    *reinterpret_cast<u32 *>(message + 4) = 0x17e;
    *reinterpret_cast<u16 **>(message + 8) = text;
    *reinterpret_cast<u32 *>(message + 0xc) = 0;
    *reinterpret_cast<u16 *>(message + 0x10) = 0;
    *reinterpret_cast<u32 *>(message + 0x14) = 0;
    *reinterpret_cast<u32 *>(message + 0x18) = 0;
    *reinterpret_cast<u16 *>(message + 0x1c) = 0;
    *reinterpret_cast<u32 *>(message + 0x20) = 0;
    message[0x24] = 1;
    if (u8 *const tag = MsgTags_Add(message)) MsgTag_Open(tag);
}

u32 __cdecl MsgTags_Load(Stream *stream) {
    u8 message[0x28];
    *reinterpret_cast<u32 *>(message + 0) = 0x17f;
    *reinterpret_cast<u32 *>(message + 4) = 0x17f;
    *reinterpret_cast<u32 *>(message + 8) = 0;
    *reinterpret_cast<u32 *>(message + 0xc) = 0;
    *reinterpret_cast<u16 *>(message + 0x10) = 0;
    *reinterpret_cast<u32 *>(message + 0x14) = 0;
    *reinterpret_cast<u32 *>(message + 0x18) = 0;
    *reinterpret_cast<u16 *>(message + 0x1c) = 0;
    *reinterpret_cast<u32 *>(message + 0x20) = 0;
    message[0x24] = 0;
    s32 result, done, count;
    Stream_Read(&result, stream, &count, 4, &done);
    if (!count) return 0;
    u8 record[0x21c];
    do {
        Stream_Read(&result, stream, record, 0x21c, &done);
        *reinterpret_cast<u32 *>(message + 0) = at<u32>(record, 0);
        *reinterpret_cast<u16 *>(message + 0x10) = at<u16>(record, 0x208);
        *reinterpret_cast<u8 **>(message + 8) = record + 4;
        *reinterpret_cast<u16 *>(message + 0x1c) = at<u16>(record, 0x214);
        *reinterpret_cast<u32 *>(message + 0xc) = at<u32>(record, 0x204);
        *reinterpret_cast<u32 *>(message + 0x14) = at<u32>(record, 0x20c);
        *reinterpret_cast<u32 *>(message + 0x18) = at<u32>(record, 0x210);
        *reinterpret_cast<u32 *>(message + 0x20) = at<u32>(record, 0x218);
        if (message[0x24] == 1) {
            if (u8 *const tag = MsgTags_Add(message)) MsgTag_Open(tag);
        } else {
            MsgTags_Add(message);
        }
    } while (--count);
    return 0;
}

s32 *__cdecl MsgTags_Save(Stream *stream) {
    s32 count = 0;
    for (u32 entry = 0x007c26b4; entry < 0x007c3b04; entry += 0x208)
        if (*reinterpret_cast<u16 *>((uintptr_t)entry)) ++count;
    s32 result, done;
    s32 *answer = Stream_Write(&result, stream, &count, 4, &done);
    u8 record[0x21c];
    for (u8 *entry = reinterpret_cast<u8 *>((uintptr_t)0x007c26b0); entry < reinterpret_cast<u8 *>((uintptr_t)0x007c3b00); entry += 0x208) {
        if (!at<u16>(entry, 4)) continue;
        u8 *const window = at<u8 *>(entry, 0);
        *reinterpret_cast<u32 *>(record + 0) = Window_DwordA0(window, nullptr, 1);
        *reinterpret_cast<u32 *>(record + 0x204) = Window_DwordA0(window, nullptr, 7);
        *reinterpret_cast<u16 *>(record + 0x208) = (u16)Window_DwordA0(window, nullptr, 4);
        *reinterpret_cast<u32 *>(record + 0x20c) = Window_DwordA0(window, nullptr, 5);
        *reinterpret_cast<u32 *>(record + 0x210) = Window_DwordA0(window, nullptr, 6);
        *reinterpret_cast<u16 *>(record + 0x214) = (u16)Window_DwordA0(window, nullptr, 3);
        *reinterpret_cast<u32 *>(record + 0x218) = Window_DwordA0(window, nullptr, 8);
        Crt().wcscpy(reinterpret_cast<u16 *>(record + 4), reinterpret_cast<const u16 *>(entry + 6));   // (unbounded, sic)
        answer = Stream_Write(&result, stream, record, 0x21c, &done);
    }
    return answer;
}

u32 __stdcall UI_Load(Stream *stream) {
    Windows7c26b0_Message5();
    return MsgTags_Load(stream);
}

s32 __cdecl MsgTags_ButtonHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10006 && a == 1) Window_PostMessage(*reinterpret_cast<u8 **>((uintptr_t)0x007cb2c4), 0x100003, 0, 0);
    return Window_CallDefault(window, nullptr, message, a, b);
}

s32 __cdecl MsgTags_WindowHandler(u8 *window, u32 message, u8 *tag, u32 b) {
    if ((s32)message > 0x100001) {
        if (message == 0x100002) {
            if (tag && (Fn_65d57c(tag) & 1)) {
                Window_SetShown(tag, nullptr, 0);
                if (!b) {
                    Window_PostTimed(tag, 5, 1, 0, 500);
                } else {
                    Window_SendMessage(tag, 5, 1, 0);
                }
            }
        } else if (message == 0x100003) {
            u32 delay = 100;
            for (u8 *t = global<u8 *>(0x007cb2c8); t; t = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(t))) {
                Window_PostTimed(t, 5, 1, 0, delay);
                delay += 100;
            }
        }
    } else if (message == 0x100001) {
        const u32 first = Window_DwordA0(window, nullptr, 0);
        const u32 second = Window_DwordA0(window, nullptr, 1);
        if (tag) {
            u8 rect[0xc], tagRect[0xc];
            GuiRect_Ctor(rect);
            GuiRect_Ctor(tagRect);
            Window_AddChild(window, nullptr, tag, 1, 0);
            vcall<void>(tag, 0x14, 1u);
            Window_PostTimed(tag, 6, 1, 0, 500);
            Window_GetRect(tag, nullptr, tagRect, 1);
            s16 width, height;
            Window_GetSize(tag, nullptr, &width, &height, 1);
            const u32 drop = (((u32)(uintptr_t)window & 0xffff0000u) | (u16)height) + 10;   // (the window argument's slot, sic)
            Window_GetRect(window, nullptr, rect, 1);
            Window_AdjustRect(window, nullptr, 0, 0u - drop, 0, 0);
            Window_GetRect(window, nullptr, rect, 1);
            const u32 dy = at<u32>(rect, 6) - at<u32>(tagRect, 6) + 8;
            const u32 leftTop = at<u32>(rect, 4);
            const u32 left = (u32)(s32)(s16)leftTop;
            const s32 across = (s32)at<s16>(rect, 8) - (s32)left;
            const u32 halfWidth = (left & 0xffff0000u) | (u16)(s16)(width >> 1);   // (only CX shifted: the left's sign above it)
            const u32 dx = (u32)(across >> 1) - halfWidth + leftTop - at<u32>(tagRect, 4);
            Window_Offset(tag, nullptr, dx, dy);
            Window_SetDwordA0(window, nullptr, 0, first);
            Window_SetDwordA0(window, nullptr, 1, second);
            u8 *const child = Window_FindChild(window, nullptr, 0x2e);
            if (child && !child[0x135]) Window_RunScript(child, nullptr, reinterpret_cast<u8 *>(0x00752e48), 0x32);
            GuiRect_Dtor(tagRect);
            GuiRect_Dtor(rect);
        }
    } else if (message == 0x14) {
        global<u32>(0x007cb2c8) = 0;
    }
    return Window_CallDefault(window, nullptr, message, (u32)(uintptr_t)tag, b);
}
}  // extern "C"
