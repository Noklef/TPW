// The handler of the dial-up progress box: see dialog7cc534handler.h.
#include "dialog7cc534handler.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/language.h"
#include "dialog7cc5f4.h"
#include "window.h"
#include "../online/postcardsend.h"
#include "dialog7cc530handler.h"
#include "dialog7cc5f4dial.h"
#include "dialog7cc530open.h"
#include "../core/imports.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cc534HandlerEnv kGameEnv = {&Language_WideText, &Window_PostMessage, &Dialog7cc5f4_Create, reinterpret_cast<s32(__cdecl *)(u8 *, u32, u32, u32)>((uintptr_t)0x0047eed0)};
const Dialog7cc534HandlerEnv *g_env = &kGameEnv;

u8 *box() { return gDialog7cc534handler_7cc534; }

// The box's slot 9 given the text.
void give(u32 text) { (*reinterpret_cast<void(__fastcall **)(u8 *, void *, u32)>(*reinterpret_cast<uintptr_t *>(box()) + 0x24))(box(), nullptr, text); }

void showText(u32 state, u32 index) {
    gDialog7cc534handler_7cc538 = state;
    if (box()) give(static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->text(index))));
}

void stage() {
    const s32 now = gDialog7cc5f4dial_7cc504;
    if (gDialog7cc534handler_7cc508 == now) return;
    gDialog7cc534handler_7cc508 = now;
    switch (now) {
        case -4:
            gDialog7cc534handler_7cc538 = 3;
            if (box()) give(0x0075852c);
            break;
        case -3: showText(1, 0x108); break;
        case -2: showText(1, 0x10a); break;
        case -1: showText(0, 0x109); break;
        case 1: showText(1, 0x103); break;
        case 2:
            gDialog7cc534handler_7cc538 = 2;
            gDialog7cc530open_7cc540 = 1;
            if (box()) g_env->post(box(), 4, 1, 0);
            break;
        case 3: showText(1, 0x104); break;
        case 4: showText(1, 0x105); break;
        case 5: showText(1, 0x106); break;
        case 6: showText(1, 0x107); break;
    }
}

}  // namespace

void Dialog7cc534Handler_SetEnv(const Dialog7cc534HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7cc534_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1e) {
        stage();
        return 0;
    }
    if (message == 0x14) {
        gDialog7cc534handler_7cc534 = nullptr;
    } else if (message == 4 && a == 1) {
        g_env->create(gDialog7cc530open_7cc52c);
        u8 properties[0x830];
        global<u32(__fastcall *)(void *, void *, u32, void *)>(IMPORT(Wearasr_Dialup_GetDialupProperties))(reinterpret_cast<void *>((uintptr_t)0x007c24c0), nullptr, gDialog7cc5f4dial_7cc50c, properties);
        Crt().sprintf(reinterpret_cast<char *>(gDialog7cc530handler_7cc4f0), gPostcardsend_74f8c4, properties + 0x212);
        gDialog7cc530handler_7cc53c = 1;
    }
    return g_env->base(window, message, a, b);
}
