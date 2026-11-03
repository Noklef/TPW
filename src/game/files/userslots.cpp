#include "userslots.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const UserSlotsEnv kGameEnv = {reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005c7420)};
const UserSlotsEnv *g_env = &kGameEnv;

}  // namespace

void UserSlots_SetEnv(const UserSlotsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__cdecl UserSlots_Instance() {
    u8 *&slots = *reinterpret_cast<u8 **>((uintptr_t)0x00f81ebc);
    if (!slots) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x110));
        slots = block ? g_env->ctor(block) : nullptr;
    }
    return slots;
}

u32 __fastcall Records24_CountUsed(const u8 *records) {
    u32 count = 0;
    for (u32 i = 0; i < 4; i++)
        count += at<u32>(records, 4 + 24 * i) != 0 ? 1 : 0;
    return count;
}

u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i) {
    u8 *record = records + 24 * i;
    return at<u32>(record, 4) != 0 ? record : nullptr;
}

u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i) { return at<u32>(records, 24 * i + 0x14); }

}  // extern "C"
