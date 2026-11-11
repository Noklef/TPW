// The menu's online rows and the answers of its question boxes: see panel7c2534online.h.
#include "panel7c2534online.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/string.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "../core/vobj2.h"
#include "../files/userslots.h"
#include "../frontend/scenequeue.h"
#include "../online/session.h"
#include "../world/object.h"
#include "dialog7cb2fcopen.h"
#include "dialog7cb3f8.h"
#include "dialogs.h"
#include "panel7c2534menus.h"
#include "../world/world.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Panel7c2534OnlineEnv kGameEnv = {game<void(__fastcall *)(u8 *)>(0x00409300), game<u32(__cdecl *)()>(0x00662420), WindowAt10_Hide, WindowAt10_Message4IfAny, Global879088_Set_5508b0, game<WString *(__fastcall *)(u8 *, void *, WString *)>(0x005b8160), String_AssignFromWide,
                                       game<void(__fastcall *)(u8 *, void *, u32, void *, u32)>(0x00414920), game<void(__fastcall *)(u8 *, void *, u16 *)>(0x0051ac20), Obj_SetNameAndWords, game<void(__fastcall *)(u8 *)>(0x005c8650), Dialog7cb3f8_Open, Global786b80_SetAndMode3,
                                       game<void(__fastcall *)(u8 *, void *, u32, u32)>(0x005b5cc0), game<void(__fastcall *)(u8 *)>(0x005b5e40), Obj_HasActivity, Dialog7cb2fc_Open, Language_WideText, game<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020),
                                       game<void(__fastcall *)(u8 *)>(0x005b6070), reinterpret_cast<s32(__fastcall *)(const u8 *, void *, u32, u32)>(&KeyMap_FindKey), Class702da0_Push, WindowAt10_Handler};
const Panel7c2534OnlineEnv *g_env = &kGameEnv;

u8 *&menu() { return gPanel7c2534menus_7c2534; }
u8 *place(u32 offset) { return at<u8 *>(global<u8 *>(0x00f85614), offset); }

void close() {
    if (!global<u32>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x3c) && !gPanel7c2534menus_7c2518) {
        global<u32>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x3c) = 1;
        g_env->controls(&global<u8>((u32)(uintptr_t)&gNamedObjects_786b68));
        g_env->f662420();
    }
    g_env->hide(menu());
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

void playOn() {
    close();
    g_env->leave(Online_Instance());
    u8 *chosen = place(0xc);
    if (!vcall<u32>(chosen, 0x24)) {
        chosen = place(8);
        if (!vcall<u32>(chosen, 0x24)) return;
    }
    vcall<void>(chosen, 0x20);
    vcall<void>(place(4), 0x1c);
    g_env->push(place(0x14), nullptr, 0x258, 1);
}

}  // namespace

void Panel7c2534Online_SetEnv(const Panel7c2534OnlineEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Panel7c2534_OnlineHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1000b) {
        if (a == 0x1b || !static_cast<u16>(g_env->findKey(gKeymap_78721c, nullptr, a, (b >> 16) & 0xff))) {
            close();
            return 0;
        }
    } else if (message == 0x100001) {
        switch (a) {
            case 9:
                close();
                g_env->start(Online_Instance(), nullptr, 0x005da3c0, 0);
                return 0;
            case 10:
                playOn();
                return 0;
            case 11:
                if (g_env->hasActivity(Online_Instance())) break;
                close();
                g_env->options();
                return 0;
            case 12:
                ask(0xe, 0x0048bc50);
                break;
            case 13:
                close();
                g_env->logOff(Online_Instance());
                vcall<void>(place(0x10), 0x2c);
                return 0;
            case 14:
                close();
                return 0;
            case 15:
                ask(9, 0x0048bc30);
                break;
            default:
                break;
        }
    }
    return g_env->handler(window, message, a, b);
}

u32 __cdecl Panel7c2534_AnswerSave() {
    WString name;
    g_env->assign(reinterpret_cast<String *>((uintptr_t)&gString_f82838), g_env->sessionName(Online_Instance(), nullptr, &name));
    WString_Dtor(&name);
    u8 buffer[0x3d4];
    g_env->save(reinterpret_cast<u8 *>((uintptr_t)0x0078a450), nullptr, global<u32>((u32)(uintptr_t)&gString_f82838 + 0x8), buffer, 1);
    g_env->setGlobal(2);
    u16 table[0x100];
    g_env->words(gWorldPtr, nullptr, table);
    u8 *const text = at<u8 *>(Obj_Self(reinterpret_cast<u8 *>((uintptr_t)&gNamedObjects_786b68)), 8);
    g_env->setNameWords(Online_Instance(), nullptr, 1, text, table);
    close();
    return 1;
}

u32 __cdecl Panel7c2534_AnswerQuit() {
    g_env->setGlobal(1);
    close();
    return 1;
}

u32 __cdecl Panel7c2534_AnswerQuit2() {
    g_env->setGlobal(1);
    close();
    return 1;
}

u32 __cdecl Panel7c2534_AnswerMode3() {
    g_env->setMode3();
    return 1;
}

u32 __cdecl Panel7c2534_AnswerSaves() {
    g_env->forgetSlots(UserSlots_Instance());
    g_env->openSaves();
    close();
    return 1;
}
}
