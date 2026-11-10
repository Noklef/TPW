// The handler of the window [0x7cc654]: see dialog7cc654handler.h.
#include "dialog7cc654handler.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/dequeiter.h"
#include "../core/string.h"
#include "../files/userslots.h"
#include "../gfx/surface.h"
#include "../online/session.h"
#include "dialog7cc654.h"
#include "dialogbase.h"
#include "focus.h"
#include "window.h"
#include "window7059f0.h"
#include "window7059f0texts.h"
#include "window7cc8c0handler.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cc654HandlerEnv kGameEnv = {game<s32(__cdecl *)()>(0x004bc430), &Online_Restricted, &CommandQueue_Post, &Window_SendMessage, game<void(__cdecl *)(u32)>(0x004bc500), game<void(__fastcall *)(u8 *)>(0x005b6660),
                                         &Window7059f0_NthItemData, &Window7059f0_SetItemTexts, &Window7059f0_SetNthItemData, &Window7059f0_RefreshRows, reinterpret_cast<void(__fastcall *)(void *)>(&SurfaceInfo_Free), &Focus_Restore,
                                         &UserSlots_Instance, game<u8(__fastcall *)(u8 *)>(0x005cc5e0), game<u8(__fastcall *)(u8 *)>(0x005cc600), &String_AnsiToWide, &Dialog7cc654_FitText, game<void(__cdecl *)()>(0x004bc270), &Dialog_BaseHandler};
const Dialog7cc654HandlerEnv *g_env = &kGameEnv;

const u32 kMark1 = 0x1000000;
const u32 kMark2 = 0x2000000;

u16 *addresses() { return reinterpret_cast<u16 *>((uintptr_t)&gDialog7cc654_7cb4d0); }

void store(u8 *list, s32 row, u32 data, u32 refresh) {
    u32 texts[4];   // (the first two unset: the mask leaves them out)
    texts[2] = (data & kMark1) != 0;
    texts[3] = (data & kMark2) != 0;
    g_env->setTexts(list, nullptr, row, texts, 0xc, refresh, 0xffffffff);
    g_env->setData(list, nullptr, row, data);
}

void markAll(u32 clear, u32 set) {
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc654), nullptr, 0xee8);
    const s32 rows = at<s32>(list, 0x150);
    for (s32 row = 0; row < rows; ++row) {
        u32 data;
        g_env->nth(list, nullptr, row, &data);
        store(list, row, (data & ~clear) | set, 0);
    }
    g_env->refresh(list);
}

void flipMark(u32 b) {
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc654), nullptr, 0xee8);
    const s32 row = at<s32>(list, 0x14c);
    u32 data;
    g_env->nth(list, nullptr, row, &data);
    const u32 mark = b == 2 ? kMark1 : kMark2;
    if (data & mark) data &= ~mark;
    else data = (data | mark) & ~(mark ^ (kMark1 | kMark2));
    store(list, row, data, 1);
}

void showList(u8 *list, u32 b) {
    u32 data;
    g_env->nth(list, nullptr, static_cast<s32>(b), &data);
    u8 *const lists = g_env->source() + 0x64;
    global<u8 *>(0x007cc660) = lists + 0x10;
    g_env->first(lists);
    for (u32 count = data & ~(kMark1 | kMark2); count; --count) g_env->next(lists);
    u8 *node = *at<u8 **>(global<u8 *>(0x007cc660), 0x24);
    Crt().wcscpy(addresses(), reinterpret_cast<const u16 *>(gWindow7cc8c0handler_78a374));
    while (node != at<u8 *>(global<u8 *>(0x007cc660), 0x24)) {
        const char *const text = at<const char *>(node, 0xc);
        Crt().wcscat(addresses(), g_env->toWide(text ? text : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0), nullptr, 0));
        Crt().wcscat(addresses(), reinterpret_cast<const u16 *>(gDialog7cc654handler_7558f0));
        node = *reinterpret_cast<u8 **>(node);
    }
    g_env->fit();
    g_env->show();
}

// True when the window closes.
bool leave(u8 *window) {
    const s32 marked = g_env->marked();
    if (marked > 0 && !g_env->restricted(Online_Instance())) {
        g_env->post(0x4e, 0);
        return false;
    }
    g_env->send(window, 4, 0, 0);
    g_env->leave(marked != 0);
    g_env->closeSession(Online_Instance());
    return true;
}

}  // namespace

void Dialog7cc654Handler_SetEnv(const Dialog7cc654HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cc654_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u8 *>(0x007cc654) = nullptr;
        g_env->freeSurface(reinterpret_cast<void *>((uintptr_t)0x007cc628));
        g_env->send(at<u8 *>(global<u8 *>(0x00f82884), 4), 6, 1, 0);
        g_env->focus();
    } else if (message == 0x15) {
        g_env->send(at<u8 *>(global<u8 *>(0x00f82884), 4), 6, 0, 0);
    } else if (message == 0x100) {
        if (a == 0xffffffff) {
            if (leave(window)) return 0;
        } else if (a == 0xee5) {
            markAll(kMark2, kMark1);
        } else if (a == 0xee6) {
            markAll(kMark1, kMark2);
        } else if (a == 0xee7) {
            markAll(kMark1 | kMark2, 0);
        }
    } else if (message == 0x404) {
        if (static_cast<s32>(b) > 1) flipMark(b);
    } else if (message == 0x401) {
        u8 *const list = Window_FindChild(window, nullptr, a);
        if (list && b != 0xffffffff) showList(list, b);
    }
    return g_env->base(window, message, a, b);
}
}
