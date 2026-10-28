#include "endthing.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "objectdefs.h"
#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const EndThingEnv kGameEnv = {
    reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>((uintptr_t)0x004db090),
    reinterpret_cast<void(__fastcall *)(void *, void *, u8 *)>((uintptr_t)0x00481b60),
};
const EndThingEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void EndThing_SetEnv(const EndThingEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl EndThing_Create() {
    u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
    u8 *def = ObjectDefs_First(defs);
    while (def && Crt().wcsicmp(reinterpret_cast<const u16 *>(def + 0x390), global<const u16 *>(0x007cf9c8)) != 0) def = ObjectDefs_Next(defs);
    u8 *object = nullptr;
    if (def) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x450));
        if (block) object = g_env->construct(block, nullptr, 1, at<u16>(def, 0), 0, 0);
        u16 handle;
        global<u16>(0x007cf4f4) = *Thing_GetHandle(object, nullptr, &handle);
    } else {
        global<u16>(0x007cf4f4) = 0;
    }
    g_env->pass(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, object);
}
}
