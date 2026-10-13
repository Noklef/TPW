// The sprite script VM: reimplementation of 0x475010-0x475260 (the run loop, its stack and
// block skipping), the opcodes 0x476020-0x476a10 and Sprite_Show (0x540b90).
//
// A script is threaded code: an array of opcode function addresses, each followed by its
// operands, all in one table in .data (0x74dab8-0x74f7c0, 1858 words; the 0x74f7c8 table
// lists a few entry points). A running script is a 0x118-byte object:
//   +4 slot, +8 pc (word index from the code base), +0xc start pc, +0x10 pc of the opcode
//   running, +0x14 code base, +0x18 state (1 running, 2 yielded, 3 held, 4 ended),
//   +0x1c stack pointer (counts down from 20), +0x20 stack (20 words), +0x70 IF depth,
//   +0x74 GOSUB depth, +0x78 REPEAT depth, +0x7c next run time, +0x80 period (ms),
//   +0x84 registers (r1-r6 floats, e.g. the position; r10 sprite set, r11 frame,
//   r12 animation, r13 the animation played), +0x114 shown.
// The opcodes work on the current script through globals: 0x7b4a18 the script, 0x7b4a00
// its code base, 0x7b49f8 its registers. Register operands 1-6 compare and add as floats.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x475010 (script): run the script until it yields, holds or ends, or has run the step
// limit (0x7b4a10) of opcodes; the END opcode (0x5da3c0, the empty Debug_Printf) sets
// state 4 and hides it. State 3 runs nothing. Returns the opcodes run.
u32 __cdecl SpriteScript_Run(u8 *script);

// 0x475230 (value) / 0x475260: push / pop the current script's stack (full: no push;
// empty: pop returns 0).
void __cdecl SpriteScript_Push(u32 value);
u32 __cdecl SpriteScript_Pop();

// 0x4750c0: skip to the ENDIF closing the current block (nested IFs counted), or the end
// of the code; the ENDIF is left to run. 0x475130: the same, but stopping at an ELSE of
// this block too, which is stepped over.
void __cdecl SpriteScript_SkipBlock();
void __cdecl SpriteScript_SkipToElse();

// The opcodes, operands inline (r = register index, n = immediate):
void __cdecl SpriteOp_Goto();        // 0x476020 (address)
void __cdecl SpriteOp_If();          // 0x476050 (r, test, r or n): tests 1-4 ==, !=, >, <
                                     // against a register, 5-8 against n; false: skip
void __cdecl SpriteOp_EndIf();       // 0x4762f0
void __cdecl SpriteOp_Else();        // 0x476320: skip to the ENDIF
void __cdecl SpriteOp_Gosub();       // 0x476340 (address): pushes the return pc
void __cdecl SpriteOp_Repeat();      // 0x4763b0: pushes the pc
void __cdecl SpriteOp_Until();       // 0x4763d0 (r, test, r or n): until the test holds
void __cdecl SpriteOp_Yield();       // 0x4766c0: state 2
void __cdecl SpriteOp_Set();         // 0x4766d0 (r, n)
void __cdecl SpriteOp_Add();         // 0x476750 (r, n)
void __cdecl SpriteOp_Sub();         // 0x4767e0 (r, n)
void __cdecl SpriteOp_Move();        // 0x476870 (r, r)
void __cdecl SpriteOp_SetAnim();     // 0x4768b0 (n): r12
void __cdecl SpriteOp_PlayReg();     // 0x4768d0 (r): r13 = the register; shown unless -1; yields
void __cdecl SpriteOp_Play();        // 0x476940 (n): r13 = n; shown unless -1; yields
void __cdecl SpriteOp_PlayAlways();  // 0x4769b0 (n): r13 = n; shown; yields
void __cdecl SpriteOp_Hide();        // 0x4769f0: +0xf8 and shown cleared
void __cdecl SpriteOp_StepR9();      // 0x476a10: r9 -= -0.01 (0x6feda0)

// 0x540b90 (script): shown (+0x114) = 1.
void __cdecl Sprite_Show(u8 *script);
}

// SpriteScript_Run with the opcode call given (the game: call the address).
u32 SpriteScript_RunWith(void(__cdecl *op)(u32 address), u8 *script);
