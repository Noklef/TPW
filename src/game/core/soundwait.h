// The sound threads' waits. Reimplementation of 0x6c6180, 0x6d3800, 0x6b9ec0 and 0x6b9ed0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct SoundWaitEnv {
    u32(__fastcall *refillVoice)(u8 *voice);                     // ChannelVoice_Refill
    void(__fastcall *refillPosition)(u8 *owner, void *, u32 position);   // SoundWorker_RefillVoice
};

void SoundWait_SetEnv(const SoundWaitEnv *env);

// 0x6c6180 (thiscall; called from 0x6ba227): bit 1 of +0x14 set; until bit 0 of +0x14 is (asked before each wait), WaitForMultipleObjects (+8 handles at +0x10, any, forever): the
// answer +0xc ends the loop, any other refills the voice (ChannelVoice_Refill) of that index among the 0x8c-byte ones at +4. Bit 1 cleared.
void __fastcall ChannelVoices_WaitLoop(u8 *pool);

// 0x6d3800 (thiscall; called from 0x6d2d64): bit 7 of +8 set; while bit 6 of +8 is, WaitForMultipleObjects (+0x30 handles at +0x2c, any, forever): the last handle's answer
// (+0x30 - 1) ends the loop, any other refills that position (SoundWorker_RefillVoice). Bit 7 cleared.
void __fastcall SoundSource_WaitLoop(u8 *source);

// 0x6b9ec0: the atexit of the singleton at 0xfb1f10 (made by 0x6b9e80): nothing.
void __cdecl SoundHandshake_AtExit();

// 0x6b9ed0 (thiscall; called from 0x6b9fd2, 0x6ba06a): byte +4 set to 1; a spin until another thread sets byte +5; dword +0 = 0.
void __fastcall SoundHandshake_Wait(u8 *handshake);
