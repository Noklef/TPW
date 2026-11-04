#include "userslots.h"

#include <cstdint>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../core/stdlist.h"
#include "../core/stdstring.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const UserSlotsEnv kGameEnv = {reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005c7420)};
const UserSlotsEnv *g_env = &kGameEnv;

}  // namespace

void UserSlots_SetEnv(const UserSlotsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall UserSlotsPart_Ctor(u8 *self) {
    for (const u32 offset : {0u, 0x10u, 0x20u})
        StdString_Tidy(reinterpret_cast<StdString *>(self + offset), nullptr, 0);
    ListHead_CtorFrom(self + 0x30, nullptr, self + 0x30);
    StdString_Tidy(reinterpret_cast<StdString *>(self + 0x3c), nullptr, 0);
    for (const u32 offset : {0x5cu, 0x68u, 0x74u}) {
        u8 *const sentinel = static_cast<u8 *>(GameHeap_Alloc(0xc));
        at<u8 *>(sentinel, 0) = sentinel;
        at<u8 *>(sentinel, 4) = sentinel;
        at<u8 *>(self, offset) = sentinel;
        at<u32>(self, offset + 4) = 0;
    }
    at<u32>(self, 0x84) = 0;
    at<u32>(self, 0x88) = 0;
    at<u32>(self, 0x8c) = 0;
    at<u16>(self, 0x90) = 0x19;
    at<u32>(self, 0x94) = 0;
    at<u32>(self, 0x98) = 0;
    return self;
}

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
