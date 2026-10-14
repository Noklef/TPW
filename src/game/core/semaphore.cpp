// A binary semaphore and its scoped lock: see semaphore.h.
#include "semaphore.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const SemaphoreEnv kGameEnv = {
    at<void *(__stdcall *const *)(void *, s32, s32, const char *)>(0x006fd16c),
    at<s32(__stdcall *const *)(void *)>(0x006fd13c),
    at<u32(__stdcall *const *)(void *, u32)>(0x006fd128),
    at<s32(__stdcall *const *)(void *, s32, s32 *)>(0x006fd170),
};
const SemaphoreEnv *g_env = &kGameEnv;

}  // namespace

void Semaphore_SetEnv(const SemaphoreEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

Semaphore *__fastcall Semaphore_Ctor(Semaphore *s) {
    s->handle = (*g_env->create)(nullptr, 1, 1, nullptr);
    return s;
}

s32 __fastcall Semaphore_Dtor(Semaphore *s) { return (*g_env->close)(s->handle); }

SemaphoreLock *__fastcall SemaphoreLock_Ctor(SemaphoreLock *l, void *, const Semaphore *s, u32 held) {
    l->handle = s->handle;
    l->held = held;
    l->kept = 0;
    return l;
}

void __fastcall SemaphoreLock_Dtor(SemaphoreLock *l) { SemaphoreLock_Release(l); }

u32 __fastcall SemaphoreLock_Acquire(SemaphoreLock *l, void *, u32 forever) {
    l->held = (*g_env->wait)(l->handle, forever ? 0xffffffff : 5) == 0;
    return l->held;
}

void __fastcall SemaphoreLock_Release(SemaphoreLock *l) {
    if (l->held && !l->kept) {
        (*g_env->release)(l->handle, 1, nullptr);
        l->held = 0;
    }
}

}  // extern "C"
