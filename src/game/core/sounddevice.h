// The sound library's DirectSound device and the COM initialisation it takes (the class names are invented): the device +0 the
// IDirectSound, +4 the primary IDirectSoundBuffer; a singleton at 0xfb2030 (guard 0xfb2028 bit 0, its atexit 0x6c4540 a bare RET: never
// released). The COM object a byte at 0xfb271c (CoInitialize answered S_OK), guard 0xfb2720 bit 0, atexit 0x6d3b80 (the destructor).
// GUIDs in the exe: 0x70aa40 CLSID_EAXDirectSound {4FF53B81-1CE0-11D3-AAB8-00A0C9C95949} (EAX 2.0's, tried first), 0x707f50 CLSID_DirectSound, 0x707f70 IID_IDirectSound.
// Reimplementation of 0x6c42e0-0x6c4532 and 0x6d3b10-0x6d3b8a; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

struct SoundDeviceEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());  // crt_atexit (0x67a7d0)
};
// The singletons' atexit goes to env (nullptr: the game's).
void SoundDevice_SetEnv(const SoundDeviceEnv *env);

extern "C" {

// 0x6d3b10 (thiscall): the byte = CoInitialize (null) answered 0 (S_FALSE gives 0 too). Returns self.
u8 *__fastcall ComInit_Ctor(u8 *self);
// 0x6d3b30 (thiscall): CoUninitialize when the byte is set (a tail call).
void __fastcall ComInit_Dtor(u8 *self);
// 0x6d3b40: the singleton (0xfb271c), constructed on the first call (guard 0xfb2720 bit 0, its atexit 0x6d3b80).
u8 *__cdecl ComInit_Get();
// 0x6d3b80: the singleton's atexit: ComInit_Dtor on it.
void __cdecl ComInit_SingletonDtor();

// 0x6c42e0 (thiscall): +0 = +4 = 0, then SoundDevice_Open (its status a local). Returns self.
u8 *__fastcall SoundDevice_Ctor(u8 *device);
// 0x6c4300 (thiscall (out), pops 4): the IDirectSound's CreateSoundBuffer (a DSBUFFERDESC {0x14, 0x11: the primary buffer with volume
// control, 0, 0, 0}) into +4; negative: *out = -1, else SoundDevice_SetFormat (out, stereo, 16 bits, 22050). Returns out.
u32 *__fastcall SoundDevice_CreatePrimary(u8 *device, void *, u32 *out);
// 0x6c4370 (thiscall (out), pops 4): SoundDevice_CreatePrimary. Returns out.
u32 *__fastcall SoundDevice_CreatePrimaryOf(u8 *device, void *, u32 *out);
// 0x6c4390 (thiscall (out, channels, bits, rate): a byte, a byte and a word; pops 0x10): without a primary buffer (+4) *out = -1; else a
// wave format holder of them (WaveFormat_Build) and the buffer's SetFormat: *out = -1 when negative, else 0. Returns out.
u32 *__fastcall SoundDevice_SetFormat(u8 *device, void *, u32 *out, u32 channels, u32 bits, u32 rate);
// 0x6c4410 (thiscall (out), pops 4): ComInit_Get, then SoundDevice_CoCreate, the IDirectSound's Initialize (null) and its
// SetCooperativeLevel (GetForegroundWindow, DSSCL_PRIORITY); each negative fails: SoundDevice_Release and *out = -1; else *out = 0.
// Returns out.
u32 *__fastcall SoundDevice_Open(u8 *device, void *, u32 *out);
// 0x6c4480 (thiscall): +4 then +0 Released (slot 2) and cleared when not null.
void __fastcall SoundDevice_Release(u8 *device);
// 0x6c44b0 (thiscall (out), pops 4): CoCreateInstance (0x70aa40, no outer, CLSCTX_INPROC_SERVER, IID_IDirectSound, into +0): not
// negative: *out = 0; else *out = CoCreateInstance's answer for CLSID_DirectSound (0x707f50), the rest the same. Returns out.
u32 *__fastcall SoundDevice_CoCreate(u8 *device, void *, u32 *out);
// 0x6c4500: the device singleton (0xfb2030), constructed on the first call (guard 0xfb2028 bit 0, its atexit 0x6c4540).
u8 *__cdecl SoundDevice_Get();

}  // extern "C"
