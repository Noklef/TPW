// The chat line's handler: see window7cc8bcedit.h.
#include "window7cc8bcedit.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../core/wtraits.h"
#include "../online/chatcommand.h"
#include "../world/mousemode.h"
#include "cursor.h"
#include "panel7c2534menus.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Window7cc8bcEditEnv kGameEnv = {reinterpret_cast<s32(__fastcall *)(const u8 *, void *, u32, u32)>(&KeyMap_FindKey), &Window_KillFocus, &Window_SetFocus, &MouseModeHolder_Ctor, &MMOnline_Ctor, &MouseModeOnlineBase_Ctor,
                                      reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *)>(&MouseMode_Set), &Window_SetCursor, &Panel7c2534_Open, &Window_SendMessage, &OnlineChat_Instance,
                                      game<void(__fastcall *)(u8 *, void *, StdWString)>(0x004ea490), &KeyMaps_ResetAllAndSetFlag, &KeyMaps_EnableAllAndClearFlag, &Window_CallDefault};
const Window7cc8bcEditEnv *g_env = &kGameEnv;

u8 *&holder() { return global<u8 *>(0x007b05e8); }
u8 *inView() { return global<u8 *>(0x007cc8bc); }
u8 *inEntry() { return global<u8 *>(0x007cc8c0); }

// Slot 9 of a window or mode (its address in the table, by thiscall).
template <typename... A> u32 slot9(u8 *object, A... args) {
    return (*reinterpret_cast<u32(__fastcall **)(u8 *, void *, A...)>(*reinterpret_cast<uintptr_t *>(object) + 0x24))(object, nullptr, args...);
}

void makeHolder() {
    if (holder()) return;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(1));
    holder() = block ? g_env->holderCtor(block) : nullptr;
}

u32 mouseMode() {
    u8 *const mode = global<u8 *>(0x007b05d8);
    return mode ? slot9(mode) : 0;
}

void say(u8 *window) {
    const u16 *const chars = reinterpret_cast<const u16 *>((uintptr_t)0x007cc6a8);
    StdWString text;
    text.allocator = 0;   // (byte 0 the window's low byte, the rest unset: never read)
    text.ptr = nullptr;
    text.len = 0;
    text.res = 0;
    const u32 length = Crt().wcslen(chars);
    if (StdWString_Grow(&text, nullptr, length, 1)) {   // (0x4051d0, assign)
        WTraits_Copy(text.ptr, chars, length);
        ImeWString_SetLength(reinterpret_cast<u8 *>(&text), nullptr, length);
    }
    g_env->say(g_env->chat(), nullptr, text);
    slot9(window, static_cast<u32>(0x0078a374));
}

void toggleEntry() {
    const u8 entry = global<u8>(0x007cc8b4);
    g_env->send(inView(), 6, entry == 0, 0);
    g_env->send(inEntry(), 6, entry, 0);
    if (global<u8>(0x007cc8b4)) {
        u8 *const list = Window_FindChild(inView(), nullptr, 0xc3123);
        u8 *const column = Window_FindChild(list, nullptr, 0xc312b);
        g_env->setFocus(Window_FindChild(column, nullptr, 0xc312c), nullptr, 0);
        makeHolder();
        if (mouseMode() == 9) g_env->setCursor(inView(), nullptr, 0x13);
    } else {
        g_env->setFocus(inEntry(), nullptr, 0);
    }
}

void leaveLine(u8 *window) {
    g_env->killFocus(window, nullptr, 0);
    g_env->setFocus(inView(), nullptr, 0);
    makeHolder();
    if (mouseMode() != 9) {
        g_env->openPanel(0);
        return;
    }
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    u8 *const online = block ? g_env->onlineCtor(block) : nullptr;
    u8 *target = holder();
    if (!target) {
        target = static_cast<u8 *>(GameHeap_Alloc(1));
        if (target) {
            u8 *const baseBlock = static_cast<u8 *>(GameHeap_Alloc(8));
            g_env->setMode(target, nullptr, baseBlock ? g_env->baseCtor(baseBlock) : nullptr);
            global<s32>(0x007b05e0) = -1;
            global<s32>(0x007b05d4) = -1;
        }
        holder() = target;
    }
    g_env->setMode(target, nullptr, online);
    g_env->setCursor(inEntry(), nullptr, 0);
}

}  // namespace

void Window7cc8bcEdit_SetEnv(const Window7cc8bcEditEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Window7cc8bc_EditHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1f) {
        g_env->resetKeys();
    } else if (message == 0x20) {
        g_env->enableKeys();
    } else if (message == 0x1000b) {
        const u32 modifiers = (b >> 16) & 0xff;
        if (a == 9) {
            g_env->killFocus(window, nullptr, 0);
            g_env->setFocus(inView(), nullptr, 0);
            return 1;
        }
        if (a == 0xd) {
            if (!modifiers) {
                say(window);
                return 1;
            }
        } else if (a == 0x73) {
            if (!modifiers && !global<u8>(0x007cc6a0)) toggleEntry();
        } else if (!static_cast<u16>(g_env->findKey(global<const u8 *>(0x0078721c), nullptr, a, modifiers))) {
            leaveLine(window);
        }
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
