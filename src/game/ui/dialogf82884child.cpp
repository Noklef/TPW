// A child of the window at [0xf82884]+4: see dialogf82884child.h.
#include "dialogf82884child.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const DialogF82884ChildEnv kGameEnv = {game<u8 *(__cdecl *)()>(0x0048bcd0), game<u8 *(__fastcall *)(u8 *, void *, u32)>(0x0065d7d0), game<u32(__fastcall *)(u8 *, void *, u32)>(0x0065d9dd)};
const DialogF82884ChildEnv *g_env = &kGameEnv;

}  // namespace

void DialogF82884Child_SetEnv(const DialogF82884ChildEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" u32 __cdecl DialogF82884_ShowChild() {
    const u32 shown = at<u32>(g_env->slots(), 0xc4) ? 1 : 0;
    u8 *const child = g_env->findChild(at<u8 *>(*reinterpret_cast<u8 **>((uintptr_t)0x00f82884), 4), nullptr, 0xbf432);
    return g_env->setShown(child, nullptr, shown);
}
