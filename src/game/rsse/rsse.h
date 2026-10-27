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

// 0x5580a0 (cdecl; vm, a walker slot, from, to, mode, t; called at 0x557e4e, 0x558008 and 0x55802a with the slot's keys +4 and +6;
// name unconfirmed): a walker moved to t thousandths of its way. The places of the parts with keys from (mask 0x80 for mode 1, else
// 0x800) and to (0x80 for mode 2) as Ride_PartPlace has them; t cut to 1000 (signed); x and z are ((to - from) * t - from * -1000) *
// 0.001 (the floats 0x701020, 0x700fd4; the x87's 24 bits, stored as floats), y too when the slot's byte +0x1a has bit 0, else
// View_FloorAt(x, z) (from's own bits when it writes nothing). The person is the thing (0x7cfb90, 20 bytes each) of the slot's word
// +0x10. Unless the slot's word +0x18 is 2 it is placed (Person_PlaceScript) at x, y, z with the slot's s16 +0x14 and entry point 1.
// Else, when vm +0x84 is set, it wobbles: a = 0.4 - (+0x10 & 7) * -0.1, b = 0.4 - (+0x10 & 3) * -0.25, c = 0.5 - s16 vm +0xc0 *
// -0.01, d and e the u16s vm +0x88 and +0x8a times 0.001 (each a stored float); now the game clock's time (Clocks_Now of 0x785970,
// made a float); s the sine (gSinTable, its entry for each float angle times the table's scale, rounded) of (now' - vm +0x80, s32) *
// 3.14159 / vm +0x84 (fidiv); y += (12 - sin(a now c * 0.004) * -3) * s * e, x += sin(a now c (stored) * 0.0033333) * b * s * 3 * d,
// z += sin(b now c * 0.0033333 - -1.5708) * a * s * 3 * d; the slot's +0x14 becomes (s16 (the clock's time / __ftol(b * 300),
// unsigned) + its s16) % 8 (C's, signed), and the person is placed there with entry point 2. With vm +0x84 zero the slot's word +0x16
// places it with entry point 3 (3), 8 (5) or 5 (6), and 4, 1 or 2 release its scripts (Person_ReleaseScripts). The original's
// Debug_Printf (the things' first entry null) is left out.
void __cdecl Rsse_MoveWalker(u8 *vm, u8 *slot, s32 from, s32 to, s32 mode, s32 t);

// 0x5571a0 (cdecl; vm, value; name unconfirmed): the walker sent back. The first busy slot (word +0x18 not 0; of the vm's +0x7c at
// +0x2c, 0x20 bytes each) whose +0x10 is value (none: nothing, the original's empty Debug_Printf left out) walks from its key +4's
// part (Ride_PartPlace, mask 0x80 for kind +0x16 4, else 0x800) to its key +6's (0x800): +0x14 the octant of that heading and +0xc
// the clock's time (Clocks_Now of 0x785970) plus the walk's time, then +8 the time read again (both as Rsse_StartWalker has them).
// Kind 2 puffs (Particle_Spawn(0x13) at 1024 times key +4's place under 0x800, each __ftol'd z first); kind 4 still there (+0x18 2)
// releases its part (the one Model_FindPartByKey4(ride [0x7a4610 + 4 vm +0xc8], 0x80, s16 +2) finds, when found: PartStates_Release
// on the ride's +8). +0x18 then 3.
void __cdecl Rsse_WalkerBack(u8 *vm, u32 value);

// 0x557d80 (cdecl; vm; name unconfirmed): with walkers (vm +0x7c not 0) the ride's model (+8 of [0x7a4610 + 4 vm +0xc8]) made the
// view (Model_BeginView), then each busy slot (the count and +0x2c re-read) by its s16 +0x18: 1 walks out (Rsse_MoveWalker from s16
// +0 to s16 +2, mode 2 for kind +0x16 4, else 0, at its progress (now * 1000 - +8 * 1000) / (+0xc - +8), unsigned) and, at 1000 or
// past, is there (+0x18 2, +8 the clock's time): kind 4 takes its part (Model_FindPartByKey4(ride, 0x80, s16 +2), when found:
// PartStates_Activate on the ride's +8 for the handle +0x10), kind 2 puffs (Particle_Spawn(0x13) at 1024 times key +2's place under
// 0x800, z first). 2 stands: kind 4 Rsse_MoveWalker(+0, +2, 2, 1), kinds 1 and 2 (+0, +2, 0, 1), others face key +2's part's direction
// (Ride_PartPlace under 0x800, its direction's third float shared with the place's first, sic; the octant of atan2(z, x) as
// Rsse_StartWalker's) and stand there (+2, +2, 0, 0). 3 walks back (+4 to +6, mode 1 for kind 4, at its progress; at 1000 or past
// +0x18 4) and 4 stands at +6 (+6, +6, 0, 0). Then with a wobble (+0x84 not 0) that has run out (+0x80 + +0x84 below the clock's
// time, unsigned) +0x84 is cleared. The original's Debug_Printf of each busy slot's +0x10 is left out.
void __cdecl Rsse_UpdateWalkers(u8 *vm);

// 0x5573d0 (cdecl (vm, kind, key, what, extra); name unconfirmed): a script's effect at the place of the ride's part with the key
// (Ride_PartPlace; not found: 0 returned). Kinds 1 and 2 (mask 0x100) start the particle effect what (with its bit 0x8000 the vm's
// thing's (the u16 +0xac; 0x7cfb90) definition's (ObjectDefs_Find of 0x7890a0 by its type +0xe) dword what & 0x7fff of +0x4e0:
// Obj_GetIndexed4e0) at 1024 ([0x700fdc]) times the place: 1 Particle_Spawn, 2 Particle_SpawnWithVelocity with the part's direction
// (__ftol'd as it is); the system's +0xa2 gets extra unless it is 1000 (ParticleSystem_SetA2), and the handle is returned. Kinds
// 3-10 (mask 0x200) play the sound what (Sound_PlayAt, owner 0, at the place __ftol'd) in the category [0x803a3c], [0x803a38],
// [0x803a28], [0x803a24], [0x803a30], [0x803a20], [0x803a2c] or, for 10, the thing's definition's +0x4f0 (Get4f0_4145e0; read before
// the place, 0: nothing, 0 returned), and return its result. Other kinds: 0. Each __ftol'd z first. The original's Debug_Printf calls
// are left out.
s32 __cdecl Rsse_Trigger(u8 *vm, s32 kind, s32 key, u32 what, u32 extra);

// 0x557970 (cdecl (vm, node, what, extra, value); name unconfirmed): a trigger node's effect started: +0xc = Rsse_Trigger(vm, kind +8,
// key +0x10, what, extra); kinds 1 and 2 then find their part under 0x100, 3-10 under 0x200 (Model_FindPartByKey4 of the ride
// [0x7a4610 + 4 vm +0xc8] by the key) into +0x14 (others: untouched; the original's Debug_Printf left out). A handle got: +0x18 =
// value and the handle returned. None: the node unlinked (+0 next, +4 previous; the vm's first +0xb0 moved on when it is the node),
// freed (Mem_GlobalFree) and the count [0x8791b4] decremented and returned (EAX as the original leaves it).
u32 __cdecl Rsse_TriggerNode(u8 *vm, u8 *node, u32 what, u32 extra, u32 value);
}
