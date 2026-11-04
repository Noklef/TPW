#include "guardheap.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename T> T &at(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

const u32 kHeadTag = 0x00774934, kTailTag = 0x0077492c, kFreedTag = 0x007749c0, kCount = 0x00f827f4;

const GuardHeapEnv kGameEnv = {&CRT_malloc, &CRT_free};
const GuardHeapEnv *g_env = &kGameEnv;

using Message = void(__cdecl *)(const char *, ...);
Message message() { return at<Message>(0x007748f4); }

// REPE CMPSB over four bytes: it stops at the first that differs.
bool same_tag(const u8 *p, u32 tag) {
    const u8 *const q = reinterpret_cast<const u8 *>((uintptr_t)tag);
    for (u32 i = 0; i < 4; ++i)
        if (q[i] != p[i]) return false;
    return true;
}

}  // namespace

void GuardHeap_SetEnv(const GuardHeapEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void *__stdcall GuardHeap_Alloc(s32 size) {
    if (size <= 0) {
        message()(text(0x0077493c), size);
        return nullptr;
    }
    u8 *const block = static_cast<u8 *>(g_env->malloc(static_cast<u32>(size) + 0xc));
    if (!block || block == reinterpret_cast<u8 *>((uintptr_t)0xffffffffu)) {
        message()(text(0x007748f8), size);
        return nullptr;
    }
    *reinterpret_cast<u32 *>(block + 4) = static_cast<u32>(size);
    *reinterpret_cast<u32 *>(block) = at<u32>(kHeadTag);
    *reinterpret_cast<u32 *>(block + size + 8) = at<u32>(kTailTag);
    ++at<s32>(kCount);
    return block + 8;
}

u32 __stdcall GuardHeap_Free(void *user) {
    const s32 address = static_cast<s32>(reinterpret_cast<uintptr_t>(user));
    if (address > -0x800 && address < 0x800) {
        message()(text(0x007749c8));
        return 0;
    }
    u8 *const base = static_cast<u8 *>(user) - 8;
    const s32 size = *reinterpret_cast<const s32 *>(base + 4);
    if (!(same_tag(base, kHeadTag) && size > 0 && size < 0x6400000 && same_tag(static_cast<const u8 *>(user) + size, kTailTag))) {
        message()(text(same_tag(base, kFreedTag) ? 0x00774998 : 0x00774958));
        return 0;
    }
    *reinterpret_cast<u32 *>(base) = at<u32>(kFreedTag);
    g_env->free(base);
    --at<s32>(kCount);
    return 1;
}
}
