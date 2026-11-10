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
#include "dialog7ca098.h"
#include "dialogs.h"
#include "guirect.h"
#include "window.h"
#include "windowreg.h"
#include "windowtimers.h"
#include "dialog7cb2b8build.h"
#include "panel7b9940.h"
#include "../core/soundset.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

const MsgTagHandlerEnv kGameHandlerEnv = {Dialog7ca098_RunAction, Window_SetShown, Particle_IsAlive, ParticleSystem_End, Window_StopTimer, Window_GetSize, Window_AdjustRect, Window_Offset, Window_RunScript, Window_Unlink, Dialog7cb2b8_ResetChild21, Window_PostMessage, Particle_Spawn, Window_SetDwordA0, Window_IsShown,
                                          ParticleSystem_Set2a, ParticleSystem_SetVectorA, Particle_SpawnAtWindow, SoundUi_Play, MsgTag_Open, Window_CallDefault};
const MsgTagHandlerEnv *g_handler = &kGameHandlerEnv;

// The first of the ten records that holds the window, or -1.
s32 recordOf(const u8 *window) {
    for (s32 i = 0; i < 10; ++i)
        if (global<const u8 *>((u32)(uintptr_t)&gMsgtags_7c26b0 + (u32)i * 0x208) == window) return i;
    return -1;
}

u32 particleOf(u8 *window) { return Window_DwordA0(window, nullptr, 9); }
bool particleLive(u32 handle) { return (s32)handle > -1 && g_handler->isAlive(handle); }

// The inner rectangle's centre in the particles' units.
void centre(u8 *window, u8 *rect, s32 &x, s32 &z) {
    Window_GetRect(window, nullptr, rect, 1);
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const s32 cy = ((bottom - top) >> 1) + top, cx = ((right - left) >> 1) + left;
    x = (s32)((u32)cx * 100000u) / 2048;
    z = (s32)((u32)cy * 75000u) / 1536;
}

void resetWhenShown(u8 *window) {
    u8 *const open = gDialog7ca098_7ca090;
    if (open == window && open) g_handler->resetChild(0);
}

s32 closed(u8 *window, u32 message, u32 a) {
    if (Window_DwordA0(window, nullptr, 6) == 0x1b) g_handler->runAction(0x1b, 0, 0);
    const s32 index = recordOf(window);
    g_handler->setShown(window, nullptr, 0);
    const u32 particle = particleOf(window);
    if (particleLive(particle)) g_handler->end(particle);
    g_handler->stopTimer(window, nullptr, 0x80080);
    g_handler->stopTimer(window, nullptr, 0x80081);
    u16 &used = global<u16>(((u32)(uintptr_t)&gMsgtags_7c26b0 + 0x4) + (u32)index * 0x208);
    if (!used) return 0;
    used = 0;
    --gMsgtags_7ca09c;
    u32 height = message;
    g_handler->getSize(window, nullptr, nullptr, reinterpret_cast<s16 *>(&height), 1);
    height += 10;
    g_handler->adjustRect(gDialog7cb2b8build_7cb2c4, nullptr, 0, height, 0, 0);
    if (window == gMsgtags_7cb2c8) reinterpret_cast<u32 &>(gMsgtags_7cb2c8) = Fn_65d56b(window);
    if (a) {
        for (u8 *tag = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(window)); tag; tag = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(tag))) {
            g_handler->offset(tag, nullptr, 0, height);
            if (!tag[0x135]) g_handler->runScript(tag, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007502f0), 0x14);
        }
        if (u8 *const child = Window_FindChild(gDialog7cb2b8build_7cb2c4, nullptr, 0x2e)) g_handler->runScript(child, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007502f0), 0x32);
    }
    g_handler->unlink(window);
    resetWhenShown(window);
    return 1;
}

void timer(u8 *window, u32 a) {
    if (a == 0x80080 && window[0x134]) {
        vcall<void>(window, 0x14, 0u);
        window[0x134] = 0;
        g_handler->post(gDialog7cb2b8build_7cb2c4, 0x100002, (u32)(uintptr_t)window, 0);
    } else if (a == 0x80081) {
        u8 rect[12];
        GuiRect_Ctor(rect);
        if (particleOf(window) == 0xffffffff) {
            s32 x, z;
            centre(window, rect, x, z);
            const u32 handle = (u32)g_handler->spawn(0x5b, x, 0, z);
            g_handler->setDword(window, nullptr, 9, handle);
            g_handler->stopTimer(window, nullptr, 0x80081);
            if (!g_handler->isShown(window)) g_handler->set2a(handle, 1);
        }
        GuiRect_Dtor(rect);
    }
}

void clicked(u8 *window, u32 a) {
    if (a == 0) {
        g_handler->sound(0xc0);
        if (gDialog7ca098_7ca090 == window) resetWhenShown(window);
        else g_handler->open(window);
    } else if (a == 1 && window != gDialog7ca098_7ca094) {
        vcall<void>(window, 0x14, 0u);
        window[0x134] = 0;
        resetWhenShown(window);
        g_handler->post(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(window)), 0x100002, (u32)(uintptr_t)window, 0);
    }
}
}  // namespace

void MsgTag_SetHandlerEnv(const MsgTagHandlerEnv *env) { g_handler = env ? env : &kGameHandlerEnv; }

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
        u8 *const record = reinterpret_cast<u8 *>((uintptr_t)((u32)(uintptr_t)&gMsgtags_7c26b0 + 0x208 * i));
        *reinterpret_cast<u16 *>(record + 4) = 0;
        u8 *const window = MsgTag_NewWindow(0x384 + i);
        *reinterpret_cast<u8 **>(record) = window;
        if (window) Window_SetDwordA0(window, nullptr, 2, (u32)(uintptr_t)(record + 6));
    }
}

void __cdecl MsgTag_Open(u8 *tag) {
    if (gDialog7ca098_7ca090 == tag) return;
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
        Window_SetDwordA0(gDialogs_7ca098, nullptr, i + 1, value);
    }
    Dialog7cb2b8_Child21Post((u32)(uintptr_t)gDialogs_7ca098);
    gDialog7ca098_7ca094 = tag;
    Advisor_PostId(0x11c);
}

u8 *__cdecl MsgTags_Add(const u8 *message) {
    if (!reinterpret_cast<u32 &>(gPanel7b9940_7cb2ac)) return nullptr;
    bool fresh = true;
    for (u8 *tag = gMsgtags_7cb2c8; tag; tag = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(tag))) {
        if (at<u32>(message, 4) != Window_DwordA0(tag, nullptr, 1)) continue;
        if (at<u16>(message, 0x1c) != (u16)Window_DwordA0(tag, nullptr, 3)) continue;
        if (at<u32>(message, 0x20) != Window_DwordA0(tag, nullptr, 8)) continue;
        if (at<u32>(message, 0) == at<u32>(message, 4)) {
            u16 *const text = reinterpret_cast<u16 *>((uintptr_t)Window_DwordA0(tag, nullptr, 2));
            Crt().wcsncpy(text, at<const u16 *>(message, 8), 0xff);
            text[0xff] = 0;
            if (tag == gDialog7ca098_7ca090) vcall<void>(Window_FindChild(gDialogs_7ca098, nullptr, 0x10d32), 0x24, text);
            fresh = false;
        } else {
            Window_SendMessage(gDialog7cb2b8build_7cb2c4, 0x100002, (u32)(uintptr_t)tag, 1);
        }
    }
    if (!fresh || gMsgtags_7ca09c >= gMsgTags_Add_750610 || !gDialog7cb2b8build_7cb2c4) return nullptr;
    s16 width;
    Window_GetSize(gDialog7cb2b8build_7cb2c4, nullptr, &width, nullptr, 1);
    u8 *tag = nullptr;
    for (u8 *record = reinterpret_cast<u8 *>((uintptr_t)&gMsgtags_7c26b0); record < reinterpret_cast<u8 *>(&gMsgtags_7c3b00); record += 0x208) {
        if (*reinterpret_cast<u16 *>(record + 4)) continue;
        *reinterpret_cast<u16 *>(record + 4) = 1;
        tag = *reinterpret_cast<u8 **>(record);
        Window_Notify1(tag, nullptr, (u32)(s32)at<s16>(message, 0x10));
        break;
    }
    if (!tag) return nullptr;
    if (!gMsgtags_7cb2c8) gMsgtags_7cb2c8 = tag;
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
    Window_PostMessage(gDialog7cb2b8build_7cb2c4, 0x100001, (u32)(uintptr_t)tag, 0);
    gMsgtags_7ca09c++;
    Sound_PlayAt(0, gSoundCategories.ui, 0x1e, 0, 0, 0);
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
    Crt().swprintf(text, reinterpret_cast<const u16 *>((uintptr_t)&gMsgTags_AddVersion_750694), 2, 0);
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
    for (u32 entry = ((u32)(uintptr_t)&gMsgtags_7c26b0 + 0x4); entry < 0x007c3b04; entry += 0x208)
        if (*reinterpret_cast<u16 *>((uintptr_t)entry)) ++count;
    s32 result, done;
    s32 *answer = Stream_Write(&result, stream, &count, 4, &done);
    u8 record[0x21c];
    for (u8 *entry = reinterpret_cast<u8 *>((uintptr_t)&gMsgtags_7c26b0); entry < reinterpret_cast<u8 *>(&gMsgtags_7c3b00); entry += 0x208) {
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
    if (message == 0x10006 && a == 1) Window_PostMessage(gDialog7cb2b8build_7cb2c4, 0x100003, 0, 0);
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
            for (u8 *t = gMsgtags_7cb2c8; t; t = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(t))) {
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
        reinterpret_cast<u32 &>(gMsgtags_7cb2c8) = 0;
    }
    return Window_CallDefault(window, nullptr, message, (u32)(uintptr_t)tag, b);
}

s32 __cdecl MsgTag_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5: return closed(window, message, a);
    case 6: {
        const s32 index = recordOf(window);
        if (index < 0 || !global<u16>(((u32)(uintptr_t)&gMsgtags_7c26b0 + 0x4) + (u32)index * 0x208)) return 0;
        break;
    }
    case 0x10: timer(window, a); break;
    case 0x11: {
        if (a && !Window_DwordA0(window, nullptr, 10)) {
            g_handler->spawnAtWindow(0x5a, window);
            g_handler->setDword(window, nullptr, 10, 1);
        }
        const u32 particle = particleOf(window);
        if (particleLive(particle)) g_handler->set2a(particle, a ? 0 : 1);
        break;
    }
    case 0x13: {
        const u32 particle = particleOf(window);
        u8 rect[12];
        GuiRect_Ctor(rect);
        if (particleLive(particle)) {
            s32 x, z;
            centre(window, rect, x, z);
            g_handler->setVector(particle, x, 0, z);
        }
        GuiRect_Dtor(rect);
        break;
    }
    case 0x14: {
        const s32 index = recordOf(window);
        if (index != -1) {
            global<u16>(((u32)(uintptr_t)&gMsgtags_7c26b0 + 0x4) + (u32)index * 0x208) = 0;
            global<u32>((u32)(uintptr_t)&gMsgtags_7c26b0 + (u32)index * 0x208) = 0;
        }
        break;
    }
    case 0x10006: clicked(window, a); break;
    default: break;
    }
    return g_handler->callDefault(window, nullptr, message, a, b);
}
}  // extern "C"
