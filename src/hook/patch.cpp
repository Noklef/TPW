#include "patch.h"

#include <cstring>
#include <windows.h>

namespace tpw::hook {

static bool write_code(void *target, const void *bytes, size_t n) {
    DWORD old;
    if (!VirtualProtect(target, n, PAGE_EXECUTE_READWRITE, &old))
        return false;
    std::memcpy(target, bytes, n);
    VirtualProtect(target, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, n);
    return true;
}

PatchResult patch_jump(void *target, const void *replacement, const std::uint8_t expect[5],
                       std::uint8_t saved[5]) {
    if (std::memcmp(target, expect, 5) != 0)
        return PatchResult::BytesMismatch;
    if (saved)
        std::memcpy(saved, target, 5);

    std::uint8_t jmp[5] = {0xE9};
    std::int32_t rel = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(replacement) -
                                                 (reinterpret_cast<std::uintptr_t>(target) + 5));
    std::memcpy(jmp + 1, &rel, 4);
    return write_code(target, jmp, 5) ? PatchResult::Ok : PatchResult::ProtectFailed;
}

bool unpatch(void *target, const std::uint8_t saved[5]) {
    return write_code(target, saved, 5);
}

}  // namespace tpw::hook
