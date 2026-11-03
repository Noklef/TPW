// The game's menu's handler: see panel7c2534handler.h.
#include "panel7c2534handler.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../online/session.h"
#include "../world/object.h"
#include "dialog7cb2fcopen.h"
#include "dialogs.h"
#include "savedialogcreate.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Panel7c2534HandlerEnv kGameEnv = {game<void(__fastcall *)(u8 *)>(0x00409300), game<u32(__cdecl *)()>(0x00662420), WindowAt10_Hide, WindowAt10_Message4IfAny, WindowAt10_ScalarDtor, reinterpret_cast<void(__cdecl *)(u32)>(&LoadDialog_Create),
                                        reinterpret_cast<void(__cdecl *)(u32)>(&SaveDialog_Create), Dialog7cb2fc_Open, Language_WideText, game<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020), Online_Instance, Obj_HasActivity,
                                        game<void(__fastcall *)(u8 *)>(0x005b5e40), Global879088_Set_5508b0, reinterpret_cast<s32(__fastcall *)(const u8 *, void *, u32, u32)>(&KeyMap_FindKey), WindowAt10_Handler};
const Panel7c2534HandlerEnv *g_env = &kGameEnv;

u8 *&menu() { return global<u8 *>(0x007c2534); }

void hide() {
    if (!global<u32>(0x00786ba4) && !global<u8>(0x007c2518)) {
        global<u32>(0x00786ba4) = 1;
        g_env->controls(&global<u8>(0x00786b68));
        g_env->f662420();
    }
    g_env->hide(menu());
}

void close() {
    hide();
    u8 *const object = menu();
    if (object) {
        g_env->tell(object);
        GameHeap_Free(object);
    }
    menu() = nullptr;
}

void ask(u32 text, u32 callback) {
    struct {
        u16 *text;
        u8 colour[4];
        u32 kind, data;
    } record;
    record.text = g_env->text(text);
    record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
    record.kind = 4;
    record.data = 0;
    g_env->box(reinterpret_cast<const u8 *>(&record), 2, 0, 1, callback, 0, 0, 0);
}

}  // namespace

void Panel7c2534Handler_SetEnv(const Panel7c2534HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Panel7c2534_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1000b) {
        if (a == 0x1b || !static_cast<u16>(g_env->findKey(global<const u8 *>(0x0078721c), nullptr, a, (b >> 16) & 0xff))) {
            hide();
            if (menu()) g_env->destroy(menu(), nullptr, 1);
            menu() = nullptr;
            return 0;
        }
    } else if (message == 0x100001) {
        switch (a) {
            case 0:
            case 0x10:
            case 0x15: close(); return 0;
            case 1: close(); g_env->load(0); return 0;
            case 0x16: close(); g_env->load(1); return 0;
            case 2: close(); g_env->save(0); return 0;
            case 3: ask(0xb, 0x0048b5b0); break;
            case 0x17: ask(0xb, 0x0048b620); break;
            case 4: ask(0xdd, 0x0048b400); break;
            case 8:
            case 0x14:
            case 0x19: ask(9, 0x0048b310); break;
            case 5:
                if (!g_env->hasActivity(g_env->online())) {
                    close();
                    g_env->options();
                    return 0;
                }
                break;
            case 0x13: global<u32>(0x00f8288c) = 1;  // fall through
            case 6:
            case 0x18:
                g_env->setGlobal(2);
                close();
                return 1;
            case 7:
                g_env->leave(g_env->online());
                close();
                return 1;
            default: break;
        }
    }
    return g_env->handler(window, message, a, b);
}
}
