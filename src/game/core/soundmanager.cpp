#include "soundmanager.h"

#include <cstdint>

#include "gameheap.h"
#include "soundchannel.h"
#include "soundobject.h"
#include "soundworker.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const SoundManagerEnv kMakeEnv = {SoundWorkerHost_Ctor};
const SoundManagerEnv *g_makeEnv = &kMakeEnv;

}  // namespace

extern "C" {

void __fastcall SoundManager_Dtor(u8 *manager) {
    at<u32>(manager, 0) = (u32)(uintptr_t)SoundManager_vtable;
    u8 *const engine = at<u8 *>(manager, 4);
    if (!engine) return;
    vcall<void>(engine, 0, 1u);
    at<u8 *>(manager, 4) = nullptr;
}

void SoundManager_SetEnv(const SoundManagerEnv *env) { g_makeEnv = env ? env : &kMakeEnv; }

u32 *__cdecl SoundManager_Make(u32 *out, u32 a, u8 **manager) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x34));
    if (block) {
        g_makeEnv->engineCtor(block);
        at<u32>(block, 0) = (u32)(uintptr_t)Class70a2a0_vtable;
    }
    u32 status;
    SoundManager_Create(&status, manager, block, a);
    if ((s32)status >= 0) {
        *out = 0;
        return out;
    }
    vcall<u32 *>(*manager, 0, &a);
    *manager = nullptr;
    *out = 0xffffffffu;
    return out;
}

u32 *__stdcall SoundManager_Create(u32 *out, u8 **manager, u8 *engine, u32 a) {
    u8 *m = static_cast<u8 *>(GameHeap_Alloc(8));
    if (m) {
        at<u32>(m, 4) = 0;
        at<u32>(m, 0) = (u32)(uintptr_t)SoundManager_vtable;
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

u32 *__fastcall SoundManager_SetPlacement(u8 *manager, void *, u32 *out, const u32 *position, const u32 *direction, const u32 *value) {
    SoundObject_SetPlacement(at<u8 *>(manager, 4), nullptr, out, position, direction, value);
    return out;
}

u32 *__fastcall SoundManager_Apply(u8 *manager, void *, u32 *out, u8 *functor) {
    SoundChannel_Apply(at<u8 *>(manager, 4), nullptr, out, functor);
    return out;
}

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
