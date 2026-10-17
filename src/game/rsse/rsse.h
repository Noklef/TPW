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

// 0x551600 (cdecl; RSSE_Init in the symbols): once only (a second call does nothing): the initialised flag (0x8791a0) and 0x8791a8 are set to 1, the script list head (0x8791b0), the count
// (0x8791ac), the tick counter (0x8791a4), the checksum (0x879190) and the lock (0x87919c) are cleared; then the checksum becomes the sum of the signed characters of the names in the opcode
// table at 0x765280, read one pointer after another until a pointer to an empty string.
void __cdecl RSSE_Init();

// 0x468600 (cdecl; the script header, an event key's index, three optional outputs; name unconfirmed): the header holds twelve event tables from +0x1c ({count, entries}, 12 bytes each, 8-byte
// entries with a key at +4) and the pointer at +0xac to the keys. The first table (and entry in it) whose key equals keys[index] is stored through the outputs (the table number, the entry, the
// count), and the name of that table's event ("Create", "Destroy", ... "Other", "Other" are strings at 0x74d148-0x74d19c; the thirteenth, "None", is never reached) is returned. None found: "Failure" (0x74d6d4).
const char *__cdecl RsseScript_EventName(const u8 *script, u32 index, u32 *group_out, u32 *entry_out, u32 *count_out);
}
