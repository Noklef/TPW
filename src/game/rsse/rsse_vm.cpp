#include "rsse.h"

extern "C" {

u32 __cdecl RSSE_FetchWord(RsseContext *ctx) {
    s32 pc = ctx->pc;
    if (pc >= 0 && pc < static_cast<s32>(ctx->code_len)) {
        ctx->pc = pc + 1;
        return ctx->code[pc];
    }
    // Original: Debug_Printf("RSSE: Tried to read outside script") (compiled out in retail).
    ctx->pc = RSSE_PC_DEAD;
    return 0;
}

s32 __cdecl RSSE_OperandValue(RsseContext *ctx, u32 word) {
    // The original tests the top *byte* (not nibble) and strips the tag with xor.
    // Variable indices are never range-checked.
    if ((word & 0xFF000000u) == RSSE_TAG_VAR)
        return ctx->vars[word ^ RSSE_TAG_VAR];
    return static_cast<s16>(word);
}

}  // extern "C"
