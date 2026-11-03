// The chat's name menus: see chatnamemenu.h.
#include "chatnamemenu.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/wtraits.h"
#include "../online/chat.h"
#include "../online/chatcommand.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ChatNameMenuEnv kGameEnv = {Window_DwordA0, OnlineChat_Ctor, ChatAvatar_Blackmark, ChatAvatar_Page, ChatAvatar_Locate, ChatAvatar_Goto, ChatAvatar_AddBuddy, ChatAvatar_RemoveBuddy, ChatAvatar_AddIgnore, ChatAvatar_RemoveIgnore,
                                  Window_SetShown, Window_Unlink, Window_SetFocus, Window_CallDefault};
const ChatNameMenuEnv *g_env = &kGameEnv;

typedef void(__fastcall *NameCall)(u8 *avatar, void *, const StdWString *name);

NameCall command(u32 row, u32 b) {
    switch (row) {
        case 0x3004:
        case 0x178a: return g_env->blackmark;
        case 0x3005:
        case 0x178b: return g_env->gotoName;
        case 0x3006:
        case 0x178c: return b == 1 ? g_env->addIgnore : g_env->removeIgnore;
        case 0x3007: return g_env->addBuddy;
        case 0x178d: return g_env->removeBuddy;
        case 0x178e: return g_env->locate;
        case 0x178f: return g_env->page;
        default: return nullptr;
    }
}

u8 *chat() {
    u8 *&made = global<u8 *>(0x007cddec);
    if (!made) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xf0c));
        made = block ? g_env->chatCtor(block) : nullptr;
    }
    return made;
}

}  // namespace

void ChatNameMenu_SetEnv(const ChatNameMenuEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl ChatNameMenu_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x100) {
        const u16 *const name = reinterpret_cast<const u16 *>((uintptr_t)g_env->dwordA0(window, nullptr, 0));
        const NameCall call = command(a, b);
        if (call) {
            StdWString text;
            CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
            const u32 length = Crt().wcslen(name);
            if (StdWString_Grow(&text, nullptr, length, 1)) {   // (0x4051d0, assign)
                WTraits_Copy(text.ptr, name, length);
                ImeWString_SetLength(reinterpret_cast<u8 *>(&text), nullptr, length);
            }
            call(chat(), nullptr, &text);
            CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 1);
        }
        g_env->setShown(global<u8 *>(0x007cc8cc), nullptr, 0);
        g_env->unlink(global<u8 *>(0x007cc8cc));
        g_env->setFocus(global<u8 *>(0x007cc8bc), nullptr, 0);
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
}
