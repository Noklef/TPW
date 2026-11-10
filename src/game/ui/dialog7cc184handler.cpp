#include "dialog7cc184handler.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../online/postcardsend.h"
#include "dialog7ca098.h"
#include "dialog7cc188.h"
#include "dialogbase.h"
#include "dialogs.h"
#include "focus.h"
#include "textpanel7cc18c.h"
#include "window.h"
#include "dialog74fb20.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cc184HandlerEnv kGameEnv = {&Window_SendMessage, &Blocks7_Free, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &Windows7cc8bc_Message6Pair, &Game_CancelAdvisor, &Focus_Restore, &KeyMaps_EnableAllAndClearFlag, &KeyMaps_ResetAllAndSetFlag, &Dialog7cc188_Open, &Window_FindChild,
                                         reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32)>(&Charset_FormatWide), &TextPanel7cc18c_Show, reinterpret_cast<u16 *(__cdecl *)(u16 *, const u16 *)>((uintptr_t)0x0067c430), &Language_WideText, reinterpret_cast<void(__cdecl *)(const void *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020),
                                         &Postcard_Send, &Dialog_KeyHandler, &Window_CallDefault, &Window_CaptureMouse, &Window_ReleaseMouse};
const Dialog7cc184HandlerEnv *g_env = &kGameEnv;

// wcschr (0x67c2f0), of a c not 0.
u16 *find(u16 *text, u16 c) {
    for (;; ++text) {
        if (*text == c) return text;
        if (!*text) return nullptr;
    }
}

bool modeOne() {
    u8 &guard = global<u8>(0x00fb34bf);
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    return gDialog74fb20_fb3b7c == 1;
}

// All addresses good: each with an '@' past its start and a '.' more than one place past that.
bool addressesGood(u16 *text) {
    if (!*text) return false;
    const u16 *const separators = reinterpret_cast<const u16 *>((uintptr_t)0x00754448);
    for (u16 *token = g_env->token(text, separators); token; token = g_env->token(nullptr, separators)) {
        u16 *const at = find(token, 0x40);
        if (at <= token) return false;
        u16 *const dot = find(at, 0x2e);
        if (dot <= at + 1) return false;
    }
    return true;
}

// The box's text record (+7 not set).
struct BoxText {
    const u16 *text;
    u8 colour[4];
    u32 four;
};

}  // namespace

void Dialog7cc184Handler_SetEnv(const Dialog7cc184HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cc184_PictureHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10005) {
        gDialog7cc184_PictureHandler_7cc1a4 = 1;
        reinterpret_cast<u32 &>(gDialog7cc184_PictureHandler_7cc1ac) = b & 0xffff;
        gDialog7cc184_PictureHandler_7cc1a8 = static_cast<s32>(b) >> 16;
        g_env->capture(window);
    } else if (message == 0x10004) {
        gDialog7cc184_PictureHandler_7cc1a4 = 0;
        g_env->release(window);
    } else if (message == 0x10003 && gDialog7cc184_PictureHandler_7cc1a4) {
        const s32 y = static_cast<s32>(a) >> 16;
        const s32 x = static_cast<s32>(a & 0xffff);
        const s32 dy = static_cast<s32>(static_cast<u32>(gDialog7cc184_PictureHandler_7cc1a8 - y) * static_cast<u32>(static_cast<s32>(global<s16>(0x00faa5c4))));
        gDialog7cc184_PictureHandler_7cc198 += dy / 2048;
        const s32 dx = static_cast<s32>(static_cast<u32>(gDialog7cc184_PictureHandler_7cc1ac - x) * static_cast<u32>(static_cast<s32>(global<s16>(0x00faa5c0))));
        gDialog7cc184_PictureHandler_7cc19c += dx / 1536;
        if (gDialog7cc184_PictureHandler_7cc198 < 0) gDialog7cc184_PictureHandler_7cc198 = 0;
        if (gDialog7cc184_PictureHandler_7cc19c < 0) gDialog7cc184_PictureHandler_7cc19c = 0;
        gDialog7cc184_PictureHandler_7cc1a8 = y;
        gDialog7cc184_PictureHandler_7cc1ac = x;
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}

s32 __cdecl Dialog7cc184_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const dialog = gDialog7cc184handler_7cc184;
    switch (message) {
    case 5:
        g_env->send(dialog, 4, 0, 0);
        return 1;
    case 0x14: {
        u8 *const panel = gDialog7cc184handler_7cc18c;
        reinterpret_cast<u32 &>(gDialog7cc184handler_7cc184) = 0;
        if (panel) {
            g_env->freeParts(panel);
            GameHeap_Free(panel);
        }
        reinterpret_cast<u32 &>(gDialog7cc184handler_7cc18c) = 0;
        if (modeOne())
            g_env->pair();
        else
            g_env->cancel(0x133);
        g_env->focus();
        g_env->enableKeys();
        break;
    }
    case 0x15:
        g_env->resetKeys();
        break;
    case 0x100:
        if (a == 0x5da0) {
            g_env->openAddress();
        } else if (a == 0x5da1) {
            for (u32 id = 0x5da3; id <= 0x5da5; ++id) vcall<void>(g_env->find(window, nullptr, id), 0x24, 0x0078a374);
        } else if (a == 0x5da2) {
            g_env->show(g_env->find(window, nullptr, 0x5d9f));
        } else if (a == 0xfffffffe) {
            g_env->send(dialog, 4, 0, 0);
            return 0;
        } else if (a == 0xffffffff) {
            if (!global<u8>(0x007cbf40)) g_env->format(global<u8 *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007cbf48), 0x100, 0xd9);
            if (!global<u8>(0x007cbf3c)) g_env->format(global<u8 *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007cb408), 0x60, 0xd8);
            g_env->show(g_env->find(window, nullptr, 0x5d9f));
            u16 text[0x200];
            Crt().wcscpy(text, reinterpret_cast<const u16 *>((uintptr_t)0x007cb4d0));
            if (addressesGood(text)) {
                g_env->sendCard();
                g_env->send(gDialog7cc184handler_7cc184, 4, 0, 0);
            } else {
                BoxText record;
                record.text = g_env->wideText(0xd6);
                record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
                record.four = 4;
                g_env->box(&record, 1, 0, 0, 0, 0, 0, 0);
            }
            gDialog7cc184handler_7cc1a0 = 1;
            return 0;
        }
        break;
    case 0x801:
        if (a == 0x5da3)
            global<u8>(0x007cbf40) = 1;
        else if (a == 0x5da4)
            global<u8>(0x007cbf3c) = 1;
        break;
    default:
        break;
    }
    if (modeOne()) return g_env->callDefault(window, nullptr, message, a, b);
    return g_env->keys(window, message, a, b);
}
}
