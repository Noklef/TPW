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
    game<char *(__cdecl *)(char *, const char *, u32)>(0x0067ae00),
    game<u32(__cdecl *)(u32)>(0x0067a730),
    game<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067bf10),
    game<s32(__cdecl *)(s32)>(0x0067a530),
    game<s32(__cdecl *)(s32)>(0x0067a560),
    game<s32(__cdecl *)(s32)>(0x0067a590),
    game<s32(__cdecl *)(s32)>(0x0067a5c0),
    game<double(__cdecl *)(const char *)>(0x0067a6c0),
    game<s32(__cdecl *)(u16 *, const u16 *, ...)>(0x0067af00),
    game<u16 *(__cdecl *)(u16 *, const u16 *, u32)>(0x0067b0d0),
    game<s32(__cdecl *)(const u16 *, const u16 *)>(0x0067b140),
    game<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067bed0),
    game<s32(__cdecl *)(const u16 *, const u16 *)>(0x0067c290),
};
const CrtEnv *g_crt = &kGameCrt;

}  // namespace

const CrtEnv &Crt() { return *g_crt; }

void Crt_SetEnv(const CrtEnv *env) { g_crt = env ? env : &kGameCrt; }

extern "C" {

void __stdcall CRT_VectorCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *), void *) {
    u8 *element = array;
    for (s32 i = 0; i < count; ++i) {
        ctor(element);
        element += size;
    }
}

void __stdcall CRT_VectorCtorIteratorNoEh(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *)) {
    if ((s32)((u32)count - 1) < 0) return;
    u32 left = (u32)count;  // (0x80000000 too, sic)
    do {
        ctor(array);
        array += size;
    } while (--left);
}

void __stdcall CRT_VectorVbaseCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *, void *, u32), void *) {
    u8 *element = array;
    for (s32 i = 0; i < count; ++i) {
        ctor(element, nullptr, 1);
        element += size;
    }
}

void __stdcall CRT_VectorDtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *dtor)(void *)) {
    u8 *element = array + count * size;
    for (s32 remaining = count - 1; remaining >= 0; --remaining) {
        element -= size;
        dtor(element);
    }
}

}  // extern "C"
