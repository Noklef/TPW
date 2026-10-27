#include "soundmanager.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall SoundManager_Dtor(u8 *manager) {
    at<u32>(manager, 0) = 0x0070a288;
    u8 *const engine = at<u8 *>(manager, 4);
    if (!engine) return;
    vcall<void>(engine, 0, 1u);
    at<u8 *>(manager, 4) = nullptr;
}

u32 *__stdcall SoundManager_Create(u32 *out, u8 **manager, u8 *engine, u32 a) {
    u8 *m = static_cast<u8 *>(GameHeap_Alloc(8));
    if (m) {
        at<u32>(m, 4) = 0;
        at<u32>(m, 0) = 0x0070a288;
    }
    *manager = m;
    if (!m) {
        *out = 0xffffffff;
        return out;
    }
    at<u8 *>(m, 4) = engine;
    SoundManager_Call1(*manager, nullptr, out, a);
    return out;
}

u32 *__fastcall SoundManager_Release(u8 *manager, void *, u32 *out) {
    if (manager) vcall<void>(manager, 0xc, 1u);
    *out = 0;
    return out;
}

u32 *__fastcall SoundManager_Query(u8 *manager, void *, u32 *out, u32 a, u32 b, u32 c) {
    vcall<void>(at<u8 *>(manager, 4), 8, out, a, b, c);
    return out;
}

u32 __fastcall SoundManager_Post(u8 *manager, void *, u32 owner, u32 command) { return vcall<u32>(at<u8 *>(manager, 4), 0x10, owner, command); }

u32 __fastcall SoundManager_Call3(u8 *manager, void *, u32 a, u32 b, u32 c) { return vcall<u32>(at<u8 *>(manager, 4), 0xc, a, b, c); }

u32 __fastcall SoundManager_Call5(u8 *manager, void *, u32 a, u32 b) { return vcall<u32>(at<u8 *>(manager, 4), 0x14, a, b); }

u32 *__fastcall SoundManager_Call1(u8 *manager, void *, u32 *out, u32 a) {
    vcall<void>(at<u8 *>(manager, 4), 4, out, a);
    return out;
}

u8 *__fastcall SoundManager_DeletingDtor(u8 *manager, void *, u32 flags) {
    SoundManager_Dtor(manager);
    if (flags & 1) GameHeap_Free(manager);
    return manager;
}

}  // extern "C"
