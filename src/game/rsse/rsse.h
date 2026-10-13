// RSSE script VM: reimplemented functions. See .docs/03-rsse-scripting.md.
#pragma once

#include "../../../re/types.h"

// Operand tags (top nibble of a code word).
enum : u32 {
    RSSE_TAG_MASK  = 0xF0000000u,
    RSSE_TAG_OP    = 0x80000000u,
    RSSE_TAG_VAR   = 0x40000000u,
    RSSE_TAG_LABEL = 0x20000000u,
    RSSE_TAG_STR   = 0x10000000u,
};

// pc value that terminates a script (END, or any runtime error).
constexpr s32 RSSE_PC_DEAD = -10000;

extern "C" {
// 0x557a70: next code word, advancing pc; out of range kills the script and returns 0.
u32 __cdecl RSSE_FetchWord(RsseContext *ctx);
// 0x5573a0: value of a source operand: a variable, or a 16-bit sign-extended immediate.
s32 __cdecl RSSE_OperandValue(RsseContext *ctx, u32 word);
}
