// The engine's error list: see engineerrors.h.
#include "engineerrors.h"

#include <cstdint>

#include "dxerrors.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const EngineErrorsEnv kGameEnv = {reinterpret_cast<void(__cdecl *)(const char *)>((uintptr_t)0x00564650)};
const EngineErrorsEnv *g_env = &kGameEnv;

}  // namespace

void EngineErrors_SetEnv(const EngineErrorsEnv *e) { g_env = e ? e : &kGameEnv; }
const EngineErrorsEnv *EngineErrors_GetEnv() { return g_env; }

extern "C" {

void __fastcall EngineErrors_Push(u8 *list, void *, u32 kind, u32 code) {
    s32 &count = at<s32>(list, 0x200);
    if (count >= 0x40) return;
    if ((*reinterpret_cast<const u32 *>((uintptr_t)0x008bd508) & 0x1000000) && kind == 3) {
        char text[100];
        const char *message = DxResult_Message(code);
        u32 i = 0;
        do text[i] = message[i];  // (sprintf "%s": no bound, sic; the messages are shorter)
        while (message[i++]);
        g_env->log(text);
    }
    at<u32>(list, count * 8) = kind;
    at<u32>(list, count * 8 + 4) = code;
    ++count;
}

}  // extern "C"
