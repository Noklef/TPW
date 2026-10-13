// Function patching: overwrite an original function's entry with `jmp replacement`.
#pragma once

#include <cstdint>

namespace tpw::hook {

// One replaceable original function.
struct Hook {
    const char   *name;          // matches re/symbols.csv; also the tpw.ini key
    std::uint32_t address;       // VA in tp.exe (image base 0x400000, no relocations)
    void         *replacement;   // our reimplementation, same calling convention
    std::uint8_t  expect[5];     // original first 5 bytes, checked before patching
};

enum class PatchResult { Ok, BytesMismatch, ProtectFailed };

// Writes a 5-byte `E9 rel32` jump at `target` to `replacement`, but only if the
// current bytes equal `expect`. `saved` (optional) receives the original bytes.
PatchResult patch_jump(void *target, const void *replacement, const std::uint8_t expect[5],
                       std::uint8_t saved[5] = nullptr);

// Restores bytes previously saved by patch_jump.
bool unpatch(void *target, const std::uint8_t saved[5]);

}  // namespace tpw::hook
