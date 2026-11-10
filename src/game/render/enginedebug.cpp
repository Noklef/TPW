// The engine's debug log: see enginedebug.h.
#include "enginedebug.h"

#include <cstdint>
#include "d3dsetup.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
const char *text_at(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

const EngineDebugEnv kGameEnv = {reinterpret_cast<void *(__cdecl *)(const char *, const char *)>((uintptr_t)0x0067b8a0),
                                 reinterpret_cast<s32(__cdecl *)(void *, const char *, ...)>((uintptr_t)0x0067dad0),
                                 reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)0x0067d070),
                                 reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)0x0067b620)};
const EngineDebugEnv *g_env = &kGameEnv;

}  // namespace

void EngineDebug_SetEnv(const EngineDebugEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __cdecl EngineDebug_LogFallback(const char *text) {
    void *file;
    if (global<u32>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x8) != 0) {
        file = g_env->fopen(text_at((u32)&gText_EngineDebug_LogFallback_766334), text_at(0x00766344));   // "fallback...", "a"
    } else {
        file = g_env->fopen(text_at((u32)&gText_EngineDebug_LogFallback_766334), text_at((u32)(uintptr_t)&gText_Enginedebug_766128));   // "w+b"
        global<u32>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x8)++;
    }
    if (file != nullptr) {
        g_env->fprintf(file, text_at((u32)&gText_Enginedebug_764ff0), text);
        g_env->fflush(file);
        g_env->fclose(file);
    }
}

void __cdecl EngineDebug_Log(const char *text) {
    void *file;
    if (global<u32>((u32)(uintptr_t)&gD3dsetup_879a20 + 0xc) != 0) {
        file = g_env->fopen(text_at(0x00766348), text_at(0x00766344));   // "enginedebug.txt", "a"
    } else {
        file = g_env->fopen(text_at(0x00766348), text_at((u32)(uintptr_t)&gText_Enginedebug_766128));   // "w+b"
        global<u32>((u32)(uintptr_t)&gD3dsetup_879a20 + 0xc)++;
    }
    if (file != nullptr) {
        g_env->fprintf(file, text_at((u32)&gText_Enginedebug_764ff0), text);   // "%s\n"
        g_env->fflush(file);
        g_env->fclose(file);
    }
}
}
