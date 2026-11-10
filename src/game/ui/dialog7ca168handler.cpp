// The staff list dialog's handler: see dialog7ca168handler.h.
#include "dialog7ca168handler.h"

#include <cstdint>

#include "../core/idmaps.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../world/camera.h"
#include "../world/staff.h"
#include "../world/things.h"
#include "dialog7ca098.h"
#include "dialog7ca13c.h"
#include "dialog7ca150.h"
#include "dialog7cb398.h"
#include "dialogbase.h"
#include "hoverlabel.h"
#include "panel7cc3b0.h"
#include "window.h"
#include "window7059f0.h"
#include "window7059f0texts.h"
#include "dialog7ca168.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7ca168HandlerEnv kGameEnv = {&Window_SendMessage, &Window7059f0_NthItemData, &Mode19c_ToIndex, &Staff_HasPatrolRegion, &Records_DataIfTerminated, &Language_Widen, &Staff_GetWage, &Staff_GradeLevel, &Window7059f0_SetItemTexts, &Dialog_KeyHandler, &Language_WideText, &HoverLabel_Attach,
                                         &Game_CancelAdvisor, &Dialog7cb398_Create, &Dialog7ca150_Open, &Dialog7ca13c_Open, &Thing_GetHandle, &Camera_CentreOnThing, &Panel7cc3b0_Show};
const Dialog7ca168HandlerEnv *g_env = &kGameEnv;

u8 *&dialog() { return gDialog7ca168_7ca168; }

void UpdateRows() {
    u8 *const list = Window_FindChild(dialog(), nullptr, 0x321);
    for (s32 row = *reinterpret_cast<s32 *>(list + 0x150) - 1; row >= 0; --row) {
        u32 data;
        g_env->nth(list, nullptr, row, &data);
        const u8 *const staff = reinterpret_cast<const u8 *>((uintptr_t)data);
        u32 mode = g_env->mode(staff);
        if (mode == 1 && g_env->hasRegion(staff) == 0) mode = 0;
        u32 texts[5];
        texts[0] = 0;
        const u8 *const record = g_env->record(gLanguage_78baa4, nullptr, mode);
        texts[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->widen(record)));
        texts[2] = static_cast<u32>(g_env->wage(staff));
        texts[3] = (g_env->grade(staff) << 10) / 100;
        const u32 happiness = static_cast<u32>(static_cast<__int64>(*reinterpret_cast<const float *>(staff + 0x1f8))) & 0xff;
        texts[4] = static_cast<s32>(happiness << 10) / 100;
        g_env->setTexts(list, nullptr, row, texts, 0x1a, row == 0 ? 1 : 0, 0xffffffff);
    }
}

}  // namespace

void Dialog7ca168Handler_SetEnv(const Dialog7ca168HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7ca168_WindowHandler(u8 *window, u32 message, u32 a, u32 b) {
    u32 data;
    switch (message) {
    case 5:
        g_env->send(dialog(), 4, 0, 0);
        return 1;
    case 0x10:
        if (a == 0x80083 && dialog()) UpdateRows();
        break;
    case 0x14:
        dialog() = nullptr;
        g_env->cancel(0x11d);
        break;
    case 0x15: {
        const u32 text = static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->wideText(0x64)));
        g_env->hover(window, 0x328, text);
        break;
    }
    case 0x100:
        if (a == 0xfffffffe) {
            g_env->send(dialog(), 4, 0, 0);
            return 0;
        }
        if (a == 0x32e) {
            g_env->hire();
            return 0;
        }
        if (a == 0x32f) {
            g_env->list(0);
            return 0;
        }
        if (a == 0x330) {
            g_env->jobs(-1, 0);
            return 0;
        }
        break;
    case 0x400: {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {
            g_env->nth(child, nullptr, static_cast<s32>(b), &data);
            g_env->show(reinterpret_cast<u8 *>((uintptr_t)0x007cc3b0), nullptr, reinterpret_cast<u8 *>((uintptr_t)data));
        }
        return 0;
    }
    case 0x402: {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {
            g_env->nth(child, nullptr, static_cast<s32>(b), &data);
            u16 handle;
            g_env->centre(*g_env->handle(reinterpret_cast<const u8 *>((uintptr_t)data), nullptr, &handle));
            g_env->send(dialog(), 4, 0, 0);
            return 0;
        }
        break;
    }
    case 0x406:
        reinterpret_cast<u32 &>(gDialog7ca168_75118c) = b;
        break;
    default:
        break;
    }
    return g_env->key(window, message, a, b);
}
}
