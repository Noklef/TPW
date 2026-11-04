// The list window's row found by its text: see window705950find.h.
#include "window705950find.h"

#include <cstdint>

namespace {
const Window705950FindEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(const void *, const void *)>((uintptr_t)0x0067c290)};
const Window705950FindEnv *g_env = &kGameEnv;
}  // namespace

void Window705950Find_SetEnv(const Window705950FindEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __fastcall Window705950_FindRow(u8 *self, void *, const void *text) {
    u8 *node = *reinterpret_cast<u8 **>(self + 0x144);
    s32 found = -1;
    if (!node || !text) return found;
    for (s32 row = 0; node && found == -1; ++row) {
        if (!g_env->compare(node, text)) found = row;
        node = *reinterpret_cast<u8 **>(node + 0x44);
    }
    return found;
}
}
