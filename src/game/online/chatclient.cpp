// The chat client object's teardown, connection and entry to the world: see chatclient.h.
#include "chatclient.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/selectors.h"
#include "../core/stdwstring.h"
#include "../core/wtraits.h"
#include "chatlists.h"
#include "session.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

// The library's method through its import slot (thiscall).
template <typename... A> u32 session_call(u32 slot, u32 session, A... args) {
    return (*reinterpret_cast<u32(__fastcall *const *)(u32, u32, A...)>((uintptr_t)slot))(session, 0, args...);
}

const ChatClientEnv kGameEnv = {
    &ChatLists_SaveBuddies,
    &ChatLists_SaveIgnored,
    game<void(__fastcall *)(u8 *)>(0x005d29b0),
    game<void(__fastcall *)(u8 *)>(0x005d2a60),
    game<void(__fastcall *)(u8 *, void *, u32)>(0x004f7430),
    game<void(__fastcall *)(u8 *)>(0x006c2910),
    game<u32 *(__fastcall *)(u8 *, void *, u32 *, u32)>(0x005ce530),
    game<void(__fastcall *)(u8 *, void *, u32 *, u32)>(0x004f3400),
    game<void(__fastcall *)(u8 *)>(0x004e58e0),
    game<void(__fastcall *)(u8 *)>(0x004e5870),
    game<void(__fastcall *)(u8 *)>(0x004e5800),
    game<void(__fastcall *)(u8 *)>(0x005c8ba0),
    game<u32(__fastcall *)(u8 *)>(0x005d2a20),
    game<s32(__cdecl *)(u16 *, const u16 *, const char *)>(0x0067af00),
    game<void(__fastcall *)(u8 *, void *, u32 *, u32)>(0x004d8690),
    game<u8 *(__fastcall *)(u8 *, void *, u32, u32, const u8 *, u32)>(0x004f7b20),
    game<u16 *(__fastcall *)(const u8 *, void *, u16 *)>(0x0050b350),
    game<u8 *(__fastcall *)(u8 *, void *, u32)>(0x004cd300),
    game<void(__fastcall *)(u8 *, void *, u32, u32)>(0x004f7e20),
    game<u8 *(__fastcall *)(u8 *)>(0x0046ce80),
    game<u8 *(__fastcall *)(u8 *)>(0x0046c6a0),
    game<void(__fastcall *)(u8 *, void *, u8 *)>(0x0046c350),
};
const ChatClientEnv *g_env = &kGameEnv;

// A counted string's text pointer, length and capacity (+0, +4, +8) cleared; its count byte just before the text, its block `block_at` before the text.
void release_counted(u8 *text_field, u32 block_at) {
    u8 *const text = at<u8 *>(text_field, 0);
    if (text) {
        u8 &count = text[-1];
        if (count == 0 || count == 0xff) GameHeap_Free(text - block_at);
        else --count;
    }
    at<u32>(text_field, 0) = 0;
    at<u32>(text_field, 4) = 0;
    at<u32>(text_field, 8) = 0;
}

void assign(u8 *string, const u16 *text) {
    const u32 n = CRT_wcslen(text);
    if (StdWString_Grow(reinterpret_cast<StdWString *>(string), nullptr, n, 1)) {
        WTraits_Copy(at<u16 *>(string, 4), text, n);
        ImeWString_SetLength(string, nullptr, n);
    }
}

u8 *new_message(u8 *message) {
    g_env->messageCtor(message);
    at<u32>(message, 0) = 0x0070061c;
    return message;
}

}  // namespace

void ChatClient_SetEnv(const ChatClientEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall ChatClient_Dtor(u8 *self) {
    const char *const name = reinterpret_cast<const char *>((uintptr_t)0x00786de4);
    if (at<u32>(self, 0)) {
        session_call(0x006fd47c, at<u32>(self, 0));
        const u32 session = at<u32>(self, 0);
        if (session) {
            session_call(0x006fd4d8, session);
            GameHeap_Free(reinterpret_cast<void *>((uintptr_t)session));
        }
        at<u32>(self, 0) = 0;
        g_env->saveBuddies(self, nullptr, name);
        g_env->saveIgnored(self, nullptr, name);
    }
    if (at<u8>(self, 0x2b) == 1) {
        alignas(4) u8 message[8];
        g_env->messageSend(new_message(message));
    }
    release_counted(self + 0xf00, 2);
    u8 *const deque = self + 0xec8;
    while (at<u32>(deque, 0x2c)) {
        u8 *const element = at<u8 *>(deque, 0xc);
        at<u8 *>(deque, 0xc) = element + 0x10;
        g_env->elementDtor(element, nullptr, 0);
        if (--at<u32>(deque, 0x2c) == 0 || at<u32>(deque, 0xc) == at<u32>(deque, 8)) g_env->dequeNext(deque);
    }
    u8 *const block = self + 0xe60;
    u8 *const tree = block + 0x54;
    u32 first, erased;
    const u32 node = *g_env->treeFind(tree, nullptr, &first, at<u32>(block, 0x58));
    g_env->treeErase(tree, nullptr, &erased, node);
    GameHeap_Free(at<void *>(tree, 4));
    at<u32>(tree, 4) = 0;
    at<u32>(tree, 8) = 0;
    g_env->dtor40(block + 0x40);
    g_env->dtor30(block + 0x30);
    g_env->dtor20(block + 0x20);
    CountedBuf_Reset(block + 0x10, nullptr, 1);
    CountedBuf_Reset(block, nullptr, 1);
    g_env->listDtor(self + 0x48);
    g_env->listDtor(self + 0x30);
    release_counted(self + 0x1c, 1);
    release_counted(self + 0xc, 2);
}

void __fastcall ChatClient_Connect(u8 *self) {
    alignas(4) u8 message[8];
    if (!g_env->messageCheck(new_message(message))) at<u8>(self, 0x2b) = 1;
    u16 *const text = reinterpret_cast<u16 *>(self + 0x260);
    g_env->format(text, reinterpret_cast<const u16 *>((uintptr_t)0x0075c0cc), Online_String14(Online_Instance()));
    assign(self + 8, text);
    if (!at<u32>(self, 0)) {
        void *const block = GameHeap_Alloc(0xc);
        at<u32>(self, 0) = block ? session_call(0x006fd474, (u32)(uintptr_t)block) : 0;
    }
    const u32 host = at<u32>(self, 0x1c) ? at<u32>(self, 0x1c) : 0x006fe5b0u;
    const s32 port = global<s16>(0x007a1552);
    if (!session_call(0x006fd498, at<u32>(self, 0), host, port, 0x004e61f0u, (u32)(uintptr_t)Global7cc53c_Pick7cc4f0())) {
        (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)0x006fd1e0))(0x0075c0fc);
    } else {
        const u16 *const name = reinterpret_cast<const u16 *>(Online_String28(Online_Instance()));
        alignas(4) u8 strings[0x20];   // (two wide strings side by side, as the original's: the login's first argument hashes both)
        u8 *const first = strings, *const second = strings + 0x10;
        CountedBuf_Reset(second, nullptr, 0);
        assign(second, name);
        const u32 own = at<u32>(self, 0xc);
        CountedBuf_Reset(first, nullptr, 0);
        assign(first, reinterpret_cast<const u16 *>((uintptr_t)(own ? own : 0x006fd6c4u)));
        u8 *session = Online_Instance();
        if (!session) session = Online_Instance();
        const u32 word34 = at<u32>(session, 0x34);
        session_call(0x006fd494, at<u32>(self, 0), (u32)(uintptr_t)first, at<u32>(session, 0x38), word34, (u32)(uintptr_t)second, (u32)(uintptr_t)self);
        CountedBuf_Reset(first, nullptr, 1);
        release_counted(second + 4, 2);
    }
    at<u8>(self, 0xef8) = 0;
}

void __fastcall ChatClient_EnterWorld(u8 *self) {
    u32 cell = 0;
    g_env->entrance(global<u8 *>(0x007cf83c) + 0x2d8, nullptr, &cell, 0);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x3a8));
    u8 *player = nullptr;
    if (memory) {
        const u32 sprite = Online_User5c(Online_Instance());
        player = g_env->playerCtor(memory, nullptr, cell, sprite, self + 8, 1);
    }
    at<u8 *>(self, 4) = player;
    u16 handle;
    const u32 id = *g_env->handle(player, nullptr, &handle);
    const u32 word = cell & 0xffff;
    const u32 column = 0x80u | ((((u8)cell - 1) & 0x7f) << 8);
    const u32 row = 0x80u | (((word - 1) >> 7 & 0xff) << 8);
    u8 *const thing = g_env->byHandle(global<u8 *>(0x007cf83c), nullptr, id);
    if (at<u8>(thing, 2) == 0x12) {
        g_env->placeAt(thing, nullptr, column, row);
        session_call(0x006fd4cc, at<u32>(self, 0), (s32)(s16)column, (s32)(s16)row, 1u, (u32)(uintptr_t)self);
        at<u8>(self, 0xeb1) = 0;
        session_call(0x006fd4bc, at<u32>(self, 0), (u32)(uintptr_t)self);
    }
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    u8 *const mode = block ? g_env->modeCtor(block) : nullptr;
    if (!global<u8 *>(0x007b05e8)) {
        u8 *const holder = static_cast<u8 *>(GameHeap_Alloc(1));
        if (holder) {
            u8 *const base = static_cast<u8 *>(GameHeap_Alloc(8));
            g_env->setMode(holder, nullptr, base ? g_env->modeBase(base) : nullptr);
            global<s32>(0x007b05e0) = -1;
            global<s32>(0x007b05d4) = -1;
        }
        global<u8 *>(0x007b05e8) = holder;
    }
    g_env->setMode(global<u8 *>(0x007b05e8), nullptr, mode);
    session_call(0x006fd4d4, at<u32>(self, 0), 0xa00u, (u32)(uintptr_t)self);
    session_call(0x006fd4c0, at<u32>(self, 0), (u32)(uintptr_t)self);
}
}
