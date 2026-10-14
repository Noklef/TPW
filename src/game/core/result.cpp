// The Win32 error as a COM result: see result.h.
#include "result.h"

#include <cstdint>

namespace {

u32(__stdcall *const *const kGameGetLastError)() =
    reinterpret_cast<u32(__stdcall *const *)()>(static_cast<std::uintptr_t>(0x006fd138));
u32(__stdcall *const *g_getLastError)() = kGameGetLastError;

}  // namespace

void Result_SetEnv(u32(__stdcall *const *getLastError)()) {
    g_getLastError = getLastError ? getLastError : kGameGetLastError;
}

extern "C" s32 *__cdecl Result_FromLastError(s32 *out) {
    const u32 error = (*g_getLastError)();
    *out = error ? static_cast<s32>((error & 0xffff) | 0x80070000u) : 0;
    return out;
}
