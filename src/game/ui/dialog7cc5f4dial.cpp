// The handler of the dial-up form: see dialog7cc5f4dial.h.
#include "dialog7cc5f4dial.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "dialogbase.h"
#include "focus.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc5f4DialEnv kGameEnv = {&KeyMaps_EnableAllAndClearFlag, &Focus_Restore, &Window_SendMessage, &Window_PostMessage, &Window_DwordA0, &Language_WideText,
                                      reinterpret_cast<u8 *(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020), &Window_SetHandler, &Window_SetFocus, &Dialog_BaseHandler};
const Dialog7cc5f4DialEnv *g_env = &kGameEnv;

u8 *front() { return at<u8 *>(global<u8 *>(0x00f82884), 4); }

const u16 *wide(u32 address) { return reinterpret_cast<const u16 *>((uintptr_t)address); }

void connect(u8 *window) {
    const u32 id = g_env->dwordA0(window, nullptr, 0);
    g_env->send(window, 4, 0, 0);
    u8 record[0x10];   // (+7 unset: the mask leaves it out)
    at<u32>(record, 0) = 0;
    record[4] = record[5] = record[6] = 0xff;
    at<u32>(record, 8) = 4;
    at<u32>(record, 0xc) = id;
    global<u32>(0x007cc50c) = id;
    global<s32>(0x007cc508) = -1;
    global<s32>(0x007cc504) = -1;
    u8 *const box = g_env->box(record, 2, 0x004b9f30, 0, 0, 0, 0, 0);
    global<u8 *>(0x007cc534) = box;
    g_env->setHandler(box, nullptr, 0x004b9f70);
    if (!global<u32(__fastcall *)(void *, void *, u32, u32, u32, u32, u32)>(0x006fd5b4)(reinterpret_cast<void *>((uintptr_t)0x007c24c0), nullptr, id, 0x004b9f60, 0x007cc590, 0x007cc548, id)) {
        u16 *const text = g_env->text(0x1d8);
        u8 *const shown = global<u8 *>(0x007cc534);
        (*reinterpret_cast<void(__fastcall **)(u8 *, void *, u16 *)>(at<uintptr_t>(shown, 0) + 0x24))(shown, nullptr, text);
    }
}

}  // namespace

void Dialog7cc5f4Dial_SetEnv(const Dialog7cc5f4DialEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7cc5f4_DialHandler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 0x14:
            g_env->enableKeys();
            g_env->restoreFocus();
            g_env->send(front(), 6, 1, 0);
            break;
        case 0x15:
            g_env->send(front(), 6, 0, 0);
            break;
        case 0x100:
            if (a == 0xfffffffe) {
                g_env->send(window, 4, 0, 0);
                if (void(__cdecl *const done)() = global<void(__cdecl *)()>(0x007cc52c)) done();
                return 0;
            }
            if (a == 0xffffffff && Crt().wcslen(wide(0x007cc590)) && Crt().wcslen(wide(0x007cc548))) {
                connect(window);
                return 0;
            }
            break;
        case 0x802:
            if (a == 0x49)
                g_env->setFocus(Window_FindChild(window, nullptr, 0x4b), nullptr, 0);
            else
                g_env->post(window, 0x100, 0xffffffff, 0);
            break;
        case 0x803:
            g_env->setFocus(Window_FindChild(window, nullptr, a == 0x49 ? 0x4b : 0x49), nullptr, 0);
            break;
        case 0x804:
            g_env->send(window, 4, 0, 0);
            return 0;
    }
    return g_env->base(window, message, a, b);
}
