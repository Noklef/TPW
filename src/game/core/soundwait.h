// The sound threads' waits. Reimplementation of 0x6c6180, 0x6d3800, 0x6b9e80, 0x6b9ec0, 0x6b9ed0 and 0x6b9fa0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct SoundWaitEnv {
    u32(__fastcall *refillVoice)(u8 *voice);                     // ChannelVoice_Refill
    void(__fastcall *refillPosition)(u8 *owner, void *, u32 position);   // SoundWorker_RefillVoice
    s32(__cdecl *atexit)(void *fn);                              // CRT_atexit
    u32 *(__fastcall *open)(u8 *object, void *, u32 *out, u32 value);   // SoundObject_Open
    u32(__cdecl *beginThread)(void *proc, u32 stack, void *arg);  // CRT_beginthread
};

void SoundWait_SetEnv(const SoundWaitEnv *env);

// 0x6c6180 (thiscall; called from 0x6ba227): bit 1 of +0x14 set; until bit 0 of +0x14 is (asked before each wait), WaitForMultipleObjects (+8 handles at +0x10, any, forever): the
// answer +0xc ends the loop, any other refills the voice (ChannelVoice_Refill) of that index among the 0x8c-byte ones at +4. Bit 1 cleared.
void __fastcall ChannelVoices_WaitLoop(u8 *pool);
// 0x6ba220 (cdecl (arg); _beginthread's entry in ChannelVoices_Start; name invented, unconfirmed): ChannelVoices_WaitLoop on the arg's +0x10, by JMP.
void __cdecl ChannelVoices_ThreadStart(u8 *arg);

// 0x6d3800 (thiscall; called from 0x6d2d64): bit 7 of +8 set; while bit 6 of +8 is, WaitForMultipleObjects (+0x30 handles at +0x2c, any, forever): the last handle's answer
// (+0x30 - 1) ends the loop, any other refills that position (SoundWorker_RefillVoice). Bit 7 cleared.
void __fastcall SoundSource_WaitLoop(u8 *source);

// 0x6b9ec0: the atexit of the singleton at 0xfb1f10 (made by 0x6b9e80): nothing.
void __cdecl SoundHandshake_AtExit();

// 0x6b9ed0 (thiscall; called from 0x6b9fd2, 0x6ba06a): byte +4 set to 1; a spin until another thread sets byte +5; dword +0 = 0.
void __fastcall SoundHandshake_Wait(u8 *handshake);

// 0x6b9e80 (name invented): the singleton's address 0xfb1f10; made the first time (bit 0 of the guard byte 0xfb1f08): dword +0 and bytes +4, +5 cleared, atexit SoundHandshake_AtExit.
u8 *__cdecl SoundHandshake_Instance();

// 0x6b9fa0 (thiscall; an SEH frame, dropped; called from 0x6b5e23; name invented, unconfirmed; the sound engine's destructor): vtable 0x70a2a0, the handshake waited for, vtable 0x70a588;
// +0x24's slot 0 (no arguments) and +0x20's slot 0 (1), each unless null.
void __fastcall SoundEngine_Dtor(u8 *engine);

// 0x6b5e20 (thiscall (flags), pops 4; names invented, as for all below): SoundEngine_Dtor, the engine freed with flag 1. Returns the engine.
u8 *__fastcall SoundEngine_DeletingDtor(u8 *engine, void *, u32 flags);

// 0x6ba080 (thiscall): SoundMixer_TouchFlagged on the mixer +0x20.
u32 __fastcall SoundEngine_TouchFlagged(u8 *engine);

// 0x6b9e30 (thiscall; the thread 0x6b9ef0 starts, through the jump at 0x6b9d90): until the handshake's byte +4 is set, the mixer touched and Sleep (100 when it touched any, else 250). Then byte +5 = 1.
void __fastcall SoundEngine_ThreadLoop(u8 *engine);
// 0x6b9d90 (cdecl (engine); _beginthread's entry; name invented, unconfirmed): SoundEngine_ThreadLoop on the engine, by JMP.
void __cdecl SoundEngine_ThreadStart(u8 *engine);

// 0x6b9ef0 (thiscall (out, engine), pops 8): with no thread at +0, bytes +4, +5 cleared and _beginthread (0x6b9d90, 0, engine) into +0; 0 fails (*out = -1), otherwise SetThreadPriority (the thread, -1).
// *out = 0 otherwise. Returns out.
u32 *__fastcall SoundHandshake_Start(u8 *handshake, void *, u32 *out, u8 *engine);

// 0x6b9f40 (thiscall (out, value), pops 8): SoundObject_Open with value's own slot as its out; a negative answer: *out = -1. Otherwise with a mixer +0x20: its byte +0x50 cleared, the thread started
// (out again value's slot; its answer not read). *out = 0. Returns out.
u32 *__fastcall SoundEngine_Open(u8 *engine, void *, u32 *out, u32 value);

// 0x6ba020 (thiscall (out, on), pops 8): Target24_SetBit400 (on's own slot, on). On: the thread started (the same slot); a negative answer: Target24_SetBit400 (out, 0), and its status stands. Off: the
// handshake waited for. Otherwise *out = 0. Returns out.
u32 *__fastcall SoundEngine_SetThreaded(u8 *engine, void *, u32 *out, u32 on);

// 0x6b7190 (thiscall): DeleteCriticalSection on this (the voices' constructors' unwinding jumps here).
void __fastcall SoundLock_Delete(void *lock);
