#include "rsse.h"
#include <cstring>

#include "../core/gameclock.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

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

void __cdecl RSSE_Init() {
    if (global<u32>(0x008791a0))
        return;   // Original: Debug_Printf("RSSE: Tried to Initialise twice") (compiled out in retail).
    global<u32>(0x008791a0) = 1;
    global<u32>(0x008791a8) = 1;
    global<u32>(0x008791b0) = 0;
    global<u32>(0x008791ac) = 0;
    global<u32>(0x008791a4) = 0;
    global<u32>(0x00879190) = 0;
    global<u32>(0x0087919c) = 0;
    // The checksum runs over the names in the opcode table one pointer at a time (the table holds a mnemonic and an
    // operand string per opcode: every pointer is read, sic) and stops at the first pointer to an empty string.
    const u32 *entry = reinterpret_cast<const u32 *>(0x00765280);
    if (!*reinterpret_cast<const char *>(*entry))
        return;
    do {
        const char *const name = reinterpret_cast<const char *>(*entry);
        for (u32 i = 0; i < strlen(name); i++)
            global<u32>(0x00879190) += static_cast<s8>(name[i]);
        entry++;
    } while (*reinterpret_cast<const char *>(*entry));
}

const char *__cdecl RsseScript_EventName(const u8 *script, u32 index, u32 *group_out, u32 *entry_out, u32 *count_out) {
    static const u32 names[12] = {0x74d19c, 0x74d194, 0x74d18c, 0x74d184, 0x74d17c, 0x74d174, 0x74d170, 0x74d168, 0x74d160, 0x74d158, 0x74d150, 0x74d148};
    for (u32 group = 0; group < 12; group++) {
        const u8 *const record = script + 0x1c + group * 12;
        for (u32 entry = 0; entry < at<u32>(record, 0); entry++) {
            if (at<u32>(at<const u8 *>(script, 0xac), index * 4) != at<u32>(at<const u8 *>(record, 4), entry * 8 + 4))
                continue;
            if (group_out)
                *group_out = group;
            if (entry_out)
                *entry_out = entry;
            if (count_out)
                *count_out = at<u32>(record, 0);
            return reinterpret_cast<const char *>(names[group]);
        }
    }
    return reinterpret_cast<const char *>(0x74d6d4);
}

u32 __cdecl RSSE_StartTimer(RsseContext *ctx, u32 ms, u32 a, u32 b) {
    at<u32>(ctx, 0x80) = Clocks_Now(reinterpret_cast<u8 *>(0x00785970));
    at<u32>(ctx, 0x84) = ms;
    at<u16>(ctx, 0x88) = (u16)a;
    at<u16>(ctx, 0x8a) = (u16)b;
    return (ms & 0xffff0000u) | (u16)b;  // sic: EAX as the original leaves it (the opcode handler returns it)
}

}  // extern "C"
