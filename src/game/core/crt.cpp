#include "crt.h"

namespace {

template <typename F> F game(u32 address) { return reinterpret_cast<F>(address); }

const CrtEnv kGameCrt = {
    game<s32(__cdecl *)(const char *, const char *)>(0x0067df40),
    game<char *(__cdecl *)(const char *, s32)>(0x0067b5f0),
    game<char *(__cdecl *)(const char *, s32)>(0x0067bfc0),
    game<s32(__cdecl *)(const char *, const char *, u32)>(0x0067c250),
    game<char *(__cdecl *)(const char *, const char *)>(0x0067c080),
    game<s32(__cdecl *)(s32)>(0x0067f1a0),
    game<s32(__cdecl *)(s32)>(0x0067fa60),
    game<s32(__cdecl *)(const char *)>(0x0067a520),
    game<s32(__cdecl *)(char *, const char *, ...)>(0x0067a650),
    game<u32(__cdecl *)(const u16 *)>(0x0067acd0),
};
const CrtEnv *g_crt = &kGameCrt;

}  // namespace

const CrtEnv &Crt() { return *g_crt; }

void Crt_SetEnv(const CrtEnv *env) { g_crt = env ? env : &kGameCrt; }
