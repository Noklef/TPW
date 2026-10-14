// A binary semaphore and its scoped lock: reimplementation of 0x6307c0-0x630850.
//
// Semaphore holds CreateSemaphoreA(0, 1, 1, 0). A SemaphoreLock copies the handle and is
// either taken (held) or not: Acquire waits forever or 5 ms, Release (also the destructor)
// gives it back once, unless `kept` is set.
#pragma once

#include "../../../re/types.h"

struct Semaphore {
    void *handle;
};
struct SemaphoreLock {
    void *handle;
    u32 held;
    u32 kept;
};

extern "C" {
// __thiscall. 0x6307c0 / 0x6307e0: create / close.
Semaphore *__fastcall Semaphore_Ctor(Semaphore *s);
s32 __fastcall Semaphore_Dtor(Semaphore *s);
// 0x6307f0 (semaphore, held): kept = 0.
SemaphoreLock *__fastcall SemaphoreLock_Ctor(SemaphoreLock *l, void *edx, const Semaphore *s, u32 held);
// 0x630810: Release.
void __fastcall SemaphoreLock_Dtor(SemaphoreLock *l);
// 0x630820 (forever): WaitForSingleObject(INFINITE or 5); held = it was signalled.
u32 __fastcall SemaphoreLock_Acquire(SemaphoreLock *l, void *edx, u32 forever);
// 0x630850: ReleaseSemaphore(1) when held and not kept.
void __fastcall SemaphoreLock_Release(SemaphoreLock *l);
}

// The import table slots the semaphores call through.
struct SemaphoreEnv {
    void *(__stdcall *const *create)(void *attributes, s32 initial, s32 maximum, const char *name);
    s32(__stdcall *const *close)(void *handle);
    u32(__stdcall *const *wait)(void *handle, u32 ms);
    s32(__stdcall *const *release)(void *handle, s32 count, s32 *previous);
};
// The calls go through env (nullptr: the game's).
void Semaphore_SetEnv(const SemaphoreEnv *env);
