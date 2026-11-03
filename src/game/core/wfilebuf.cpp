#include "wfilebuf.h"

#include <cstdint>

#include "vcall.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const WFileBufEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)()>((uintptr_t)0x00699da0)};
const WFileBufEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void WFileBuf_SetEnv(const WFileBufEnv *env) { g_env = env ? env : &kGameEnv; }

u32 *__fastcall LocaleRef_Ctor(u32 *ref) {
    *ref = g_env->localeInit();
    u32 &uses = at<u32>(global<u8 *>(0x00fb1dd0), 4);
    if (uses < 0xffffffffu) ++uses;
    return ref;
}

void __fastcall WFileBuf_Init(u8 *buf, void *, u32 file, s32 which) {
    buf[0x48] = which == 1;
    if (which == 0) {
        u8 *const old = at<u8 *>(buf, 0x4c);
        if (old) {
            u8 *const dead = Counted_Release(old);
            if (dead) vcall<void>(dead, 0, (u32)1);
        }
        LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
        at<u32>(buf, 0x44) = 0;
    }
    static const u8 kSlots[6][2] = {{0xc, 4}, {0x1c, 0x14}, {0x20, 0x18}, {0x10, 8}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots) {
        at<u8 *>(buf, slot[0]) = buf + slot[1];
        at<u32>(buf, slot[1]) = 0;
    }
    at<u32>(buf, 0x50) = file;
    at<u32>(buf, 0x40) = global<u32>(0x00fb34b4);
    at<u32>(buf, 0x3c) = global<u32>(0x00fb34b4);
    at<u32>(buf, 0x38) = 0;
}
}
