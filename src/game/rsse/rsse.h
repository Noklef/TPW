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

// 0x557160 (cdecl; context, ms, two words; name unconfirmed): a timer started: +0x80 = the game clock's time (Clocks_Now of 0x785970), +0x84 = ms, the
// words +0x88 and +0x8a the low halves of the other two. Its one caller (0x555bc5, an opcode taking seconds) passes seconds * 1000. Returns EAX as the
// original leaves it: ms's high half with the last word.
u32 __cdecl RSSE_StartTimer(RsseContext *ctx, u32 ms, u32 a, u32 b);

// 0x556a80 (cdecl; frame, x, y, z): the three dwords at +0x30, +0x34 and +0x38 of the record that the
// frame's +8 object's +4 object's +0x78 points to, plus 0x10, written through the pointers; returns the third.
u32 __cdecl Rsse_GetVector(const u8 *frame, u32 *x, u32 *y, u32 *z);

// 0x557110 (cdecl; vm): over the +0x7c entries of 0x20 bytes at +0x2c, the dword at +0x10 of the first whose
// word at +0x18 is 4 is returned and the entry's word +0x18 and dword +0x10 are cleared; 0 for none.
u32 __cdecl Rsse_TakeFinished(u8 *vm);

// 0x556f40 (cdecl; vm, value, from, to, two words, kind, a word; called at 0x555af9 by a walk opcode; name unconfirmed): a walker
// started between two of the ride's parts. The places of the parts with keys from (mask 0x800) and to (mask 0x80 when kind is 4, else
// 0x800) as Ride_PartPlace has them (no direction); the walk's time is 100 times the distance between them (each difference, its
// square, and the sums (x + y) + z at the x87's 24 bits, x and z squared through their stored floats; its root through __ftol), 100
// when that is 0 (as u32). The first of the +0x7c walker slots (0x20 bytes at +0x2c) whose word +0x18 is 0 gets the words from (+0),
// to (+2), the two words (+4, +6), kind (+0x16) and the last word (+0x1a), value (+0x10), the game clock's time (Clocks_Now of
// 0x785970; +8) and, read again, that plus the walk's time (+0xc), +0x18 = 1, and as +0x14 the octant (C's % 8, signed) of __ftol(0.5
// - (6.28318 - (atan2(to z - from z, to x - from x) - 1.570795)) * -1.27324...) (the doubles 0x700fe8..0x701000), which it returns.
// With no free slot it returns the slot count (0 when it is not above 0). The original's Debug_Printf calls (empty) are left out, as
// is its overwriting of the stack argument to with the stored floats.
s32 __cdecl Rsse_StartWalker(u8 *vm, u32 value, s32 from, s32 to, u32 a, u32 b, u32 kind, u32 last);
}
