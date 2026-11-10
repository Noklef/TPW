// The window [0x7cc654] left: see dialog7cc654lists.h.
#include "dialog7cc654lists.h"

#include <cstdint>

#include "../core/dequeiter.h"
#include "../core/language.h"
#include "../core/selectors.h"
#include "../core/stdlist.h"
#include "../files/userslots.h"
#include "window.h"
#include "window7059f0.h"
#include "../online/postcardsend.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cc654ListsEnv kGameEnv = {&Window7059f0_NthItemData, game<u8(__fastcall *)(u8 *)>(0x005cc5e0), game<u8(__fastcall *)(u8 *)>(0x005cc600), &List68_EraseMatching, &List74_EraseMatching,
                                       game<void(__fastcall *)(u8 *)>(0x005cc640), game<void(__fastcall *)(u8 *)>(0x005cc770), &UserSlots_Instance, &Language_WideText, game<void(__fastcall *)(u8 *)>(0x005cd0a0),
                                       game<u8(__fastcall *)(u8 *, void *, const char *, s32, u8 *)>(0x005cc8a0), &Global7cc53c_Pick7cc4f0,
                                       game<u8 *(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020), &Window_SetHandler, &Window_SetShown, &CommandQueue_Post, &Window_PostMessage,
                                       game<s32(__cdecl *)(u8 *, u32, u32, u32)>(0x0047eed0)};
const Dialog7cc654ListsEnv *g_env = &kGameEnv;

const u32 kMark1 = 0x1000000;
const u32 kMark2 = 0x2000000;

void hideLink() {
    g_env->setShown(Window_FindChild(at<u8 *>(global<u8 *>(0x00f82884), 4), nullptr, 0xbf432), nullptr, 0);
}

}  // namespace

void Dialog7cc654Lists_SetEnv(const Dialog7cc654ListsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7cc654_ApplyMarks() {
    s32 count = 0;
    g_env->source();
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc654), nullptr, 0xee8);
    const s32 rows = at<s32>(list, 0x150);
    for (s32 row = 0; row < rows; ++row) {
        u32 data;
        g_env->nth(list, nullptr, row, &data);
        u8 *const lists = g_env->source() + 0x64;
        g_env->first(lists);
        for (u32 left = data & ~(kMark1 | kMark2); left; --left) g_env->next(lists);
        g_env->erase68(lists);
        g_env->erase74(lists);
        if (data & kMark1) {
            g_env->mark1(lists);
            ++count;
        } else if (data & kMark2) {
            g_env->mark2(lists);
        }
    }
    return count;
}

extern "C" void __cdecl Dialog7cc654_SendChanges(u32 changed) {
    u8 *const lists = g_env->source() + 0x64;
    u8 record[0x10];   // (+7 unset: the mask leaves it out)
    at<u16 *>(record, 0) = g_env->text(0x13a);
    record[4] = record[5] = record[6] = 0xff;
    at<u32>(record, 8) = 4;
    at<u32>(record, 0xc) = 0;
    at<u32>(lists, 0x94) = 0x004bc640;
    g_env->prepare(lists);
    if (!static_cast<u8>(changed)) return;

    const char *const host = global<const char *>(0x007a14f4);
    const char *const name = host ? host : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
    u8 sent;
    if (*name == 0) {
        sent = g_env->connect(lists, nullptr, gPostcardsend_754454, 0x19, g_env->pick());
    } else {
        const s32 port = global<s16>(0x007a1550);
        sent = g_env->connect(lists, nullptr, name, port, g_env->pick());
    }
    if (sent) {
        global<s32>(0x007cc658) = -1;
        global<s32>(0x007cc684) = -1;
        global<s32>(0x007cc664) = -1;
        global<s32>(0x007cc65c) = -1;
        u8 *const box = g_env->box(record, 0, 0, 0, 0, 0, 0, 0);
        global<u8 *>(0x007cc688) = box;
        g_env->setHandler(box, nullptr, 0x004bc660);
    } else if (!at<u32>(g_env->source(), 0xc4)) {
        hideLink();
    } else {
        g_env->post(0x39, 0);
    }
}

extern "C" s32 __cdecl Dialog7cc654_WaitHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1e) {
        u8 *const box = global<u8 *>(0x007cc688);
        if (box && (global<u32>(0x007cc658) != global<u32>(0x007cc684) || global<u32>(0x007cc664) != global<u32>(0x007cc65c))) {
            global<u32>(0x007cc658) = global<u32>(0x007cc684);
            global<u32>(0x007cc664) = global<u32>(0x007cc65c);
            g_env->postMessage(box, 4, 0, 0);
        }
        return 0;
    }
    if (message == 0x14) {
        global<u8 *>(0x007cc688) = nullptr;
        if (!at<u32>(g_env->source(), 0xc4)) hideLink();
    }
    return g_env->base(window, message, a, b);
}
