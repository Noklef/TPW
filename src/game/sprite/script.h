// The sprite script VM: reimplementation of 0x475010-0x475260 (the run loop, its stack and
// block skipping), the script list 0x475290-0x475f40, the opcodes 0x476020-0x476a10,
// Sprite_Show (0x540b90) and the sprite lookups the list uses (0x540c60, 0x540c70,
// 0x541f60). tools/spritescript.py lists the scripts.
//
// A script is threaded code: an array of opcode function addresses, each followed by its
// operands, all in one table in .data (0x74dab8-0x74f7c0, 1858 words; the 0x74f7c8 table
// lists a few entry points). A running script is a 0x118-byte object:
//   +4 slot, +8 pc (word index from the code base), +0xc start pc, +0x10 pc of the opcode
//   running, +0x14 code base, +0x18 state (1 running, 2 yielded, 3 held, 4 ended),
//   +0x1c stack pointer (counts down from 20), +0x20 stack (20 words), +0x70 IF depth,
//   +0x74 GOSUB depth, +0x78 REPEAT depth, +0x7c next run time, +0x80 period (ms),
//   +0x84 registers r0-r18: r1-r6 floats (r1-r3 the position), r7 0xff, r8 and r9 1.0f
//   at the start, r10 the sprite folder (gSpriteFolders, 0x763f88), r11 the sprite in it,
//   r12 the animation, r13 the frame shown, r14 the animation's frame count, r15, r16 flags,
//   r17 and r18 from the sprite's special action; +0xd0 what is drawn, +0x114 shown.
// The list (0x7b49f0) holds the running scripts by slot; slot 0 is never used.
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

// The script list. Slots are 1 to the table size - 1; an invalid or empty slot is ignored
// (CopyFrom and Clone read it anyway). Nothing runs before SpriteScripts_Init.
// 0x475290: a 100-slot table and the step limit (50).
void __cdecl SpriteScripts_Init();
// 0x4752e0: frees the table. The ready flag is cleared before the loop that frees the
// scripts, which then frees none: they leak.
void __cdecl SpriteScripts_Shutdown();
// 0x475360: runs each script due (now > +0x7c) and schedules its next run a period
// (+0x80) on; frees those ended (state 4) instead. Returns the opcodes run.
u32 __cdecl SpriteScripts_Update();
// 0x475430: draws every script (0x542010) between 0x540ab0 and 0x540b30 (always called).
void __cdecl SpriteScripts_Draw();
// 0x475480 (slot, r1, r2, r3, r15) / 0x4754e0 (slot, &r1, &r2, &r3, &r15; each may be null).
void __cdecl SpriteScript_SetPosition(s32 slot, u32 x, u32 y, u32 z, u32 r15);
void __cdecl SpriteScript_GetPosition(s32 slot, u32 *x, u32 *y, u32 *z, u32 *r15);
// 0x475550 (slot): frees the script.
void __cdecl SpriteScript_Kill(s32 slot);
// 0x475590 / 0x4755d0 (slot, register < 19, value): the second loads and stores a float
// (a signalling NaN comes out quiet). 0x475610 (slot, ms): the period.
void __cdecl SpriteScript_SetReg(s32 slot, u32 index, u32 value);
void __cdecl SpriteScript_SetRegF(s32 slot, u32 index, u32 bits);
void __cdecl SpriteScript_SetPeriod(s32 slot, u32 ms);
// 0x475650 (stream): "TPCS", 0x118, the table size, the table, then each script.
void __cdecl SpriteScripts_Save(void *stream);
// 0x475730 (stream): frees the list and reads one saved; 0 when it was read, 1 (and an
// empty 100-slot table) when the header does not match or the list is not ready. Scripts
// get the code base 0x74dab8.
u32 __cdecl SpriteScripts_Load(void *stream);
// 0x4758f0 (script, code address, folder, sprite, animation, r1, r2, r3, r17, r18): a
// script about to start at the address (pc as a word index from 0x74dab8): state 1, empty
// stack, period 62 ms from now, r7 0xff, r8 and r9 1.0f, r14 the animation's frame count.
void __cdecl SpriteScript_Init(u8 *script, u32 code, u32 folder, u32 sprite, u32 anim, u32 x, u32 y, u32 z,
                               u32 r17, u32 r18);
// 0x475a10 (code or special action 0-3, folder, sprite, animation, r1, r2, r3): a new
// script in the first free slot (the table grows by 50); an action is looked up on the
// sprite (Sprite_GetAction), which picks the code (0x74f7c8 table), the animation, r17 and
// r18. Returns the slot (0: not ready).
s32 __cdecl SpriteScript_Start(u32 code, u32 folder, u32 sprite, u32 anim, u32 x, u32 y, u32 z);
// 0x475b80 (slot, code or action): restarts the script there (an action also sets the
// animation and r14).
void __cdecl SpriteScript_Jump(s32 slot, u32 code);
// 0x475c50 (slot, code or action): 1 when the script started at that code. An action
// sets the script's animation as it is looked up.
u32 __cdecl SpriteScript_IsAt(s32 slot, u32 code);
// 0x475cf0 (slot, r1, r2, r3, r8): a copy of the script in a new slot, flags (r16)
// without 0x200, with 0x50000. Returns the slot.
s32 __cdecl SpriteScript_Clone(s32 slot, u32 x, u32 y, u32 z, u32 r8);
// 0x475e10 (slot, from): the code, pc and sprite registers of another script; flags as
// Clone.
void __cdecl SpriteScript_CopyFrom(s32 slot, s32 from);
// 0x475ef0 (drawn): the slot of the shown script whose +0xd0 that is; 0 for none.
s32 __cdecl SpriteScript_FindShown(const u8 *drawn);
// 0x475f40: every script's sprite (r11) modulo its folder's sprite count.
void __cdecl SpriteScripts_WrapSprites();

// 0x540c60 (__thiscall on a sprite; animation): its frame count (byte +0x222 + 4 * anim).
u32 __fastcall Sprite_GetAnimFrameCount(const u8 *sprite, void *edx, u32 anim);
// 0x540c70 (__thiscall; action 0-3, &index, &anim, &r17, &r18): the action's entry in
// the 0x74f7c8 table and animation (bytes +0x20e/+0x20f + 4 * action, less 1) and two
// more bytes. Other actions write nothing.
void __fastcall Sprite_GetAction(const u8 *sprite, void *edx, u32 action, u32 *index, u32 *anim, u32 *r17,
                                 u32 *r18);
// 0x541f60 (folder): the folder's sprite count (gSpriteFolders, 0x763f88: 12-byte
// entries, name, first index in gSprites (0x876b48), count).
s32 __cdecl SpriteFolder_Count(u32 folder);

// 0x475fa0 / 0x475fc0 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x7b4a1c = 1 and 0x7b4a20 = 1 / 2^23 (as
// camera's).
void __cdecl SpriteScripts_InitUnusedOne();
void __cdecl SpriteScripts_InitUnusedEpsilon();
}

// SpriteScript_Run with the opcode call given (the game: call the address).
u32 SpriteScript_RunWith(void(__cdecl *op)(u32 address), u8 *script);

// What the script list calls in the game, so that it can run outside it.
struct SpriteScriptEnv {
    void *(__cdecl *alloc)(u32 size);  // 0x5f5660
    void(__cdecl *free)(void *block);  // 0x5f5680
    s32(__cdecl *now)();               // the time of the game clock (0x785970), 0x402d70
    u32(__cdecl *run)(u8 *script);     // SpriteScript_Run (0x475010)
    void(__cdecl *drawBegin)();        // 0x540ab0
    void(__cdecl *draw)(u8 *script);   // 0x542010
    void(__cdecl *drawEnd)();          // 0x540b30
    // Stream_Write (0x5f5e50) / Stream_Read (0x5f5e20) (&result, stream, data, size, 0)
    void(__cdecl *write)(u32 *result, void *stream, const void *data, u32 size, u32 flags);
    void(__cdecl *read)(u32 *result, void *stream, void *data, u32 size, u32 flags);
};
// The list's calls go to env (nullptr: the game's).
void SpriteScripts_SetEnv(const SpriteScriptEnv *env);
