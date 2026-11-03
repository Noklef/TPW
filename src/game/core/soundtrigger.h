// A sound's start through the engine and mixer (names invented, unconfirmed): the voice chosen by the sound's kind and flags.
// Reimplementations of 0x6b66d0, 0x6b6700, 0x6b87d0, the originals' SEH frames dropped.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders). Every voice maker has the original's arguments.
typedef void(__cdecl *SoundVoiceMaker)(u8 *owner, u8 *voice, u8 *definition, u8 *sound);
struct SoundTriggerEnv {
    u32(__fastcall *addVoice)(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *sound);  // SoundMixer_AddVoice
    u8 *(__fastcall *takeVoice)(u8 *mixer, void *, u32 *out);                            // SoundMixer_TakeVoice
    SoundVoiceMaker makeOne;        // Voice70a8d0_Create 0x6be750
    SoundVoiceMaker makeSequence;   // SequenceVoice_Create 0x6be680
    SoundVoiceMaker makeSwitch;     // SwitchVoice_Create 0x6be610
    SoundVoiceMaker make70a878;     // Voice70a878_Create 0x6be330
    SoundVoiceMaker make70a338;     // Class70a338_Make 0x6be090
    SoundVoiceMaker make70a2e0;     // Voice70a2e0_Create 0x6bdd30
    SoundVoiceMaker make70a390;     // Voice70a390_Create 0x6bdc60
    SoundVoiceMaker make70a3e8;     // Voice70a3e8_Create 0x6bd7b0
    u8 *(__cdecl *category)();                                                           // SoundCategory_Get
    void(__fastcall *advance)(u8 *it, void *, s32 n);                                    // DequeIter_Advance
    u8 *(__fastcall *find)(u8 **self, void *, u8 *group, u8 *sound);                     // SfxSounds_Find
    u8 *(__fastcall *findVoice)(u8 *mixer, void *, u32 id);                              // SoundMixer_FindVoice
};
void SoundTrigger_SetEnv(const SoundTriggerEnv *env);

extern "C" {

// 0x6b66d0 (thiscall; pops 0xc): SoundMixer_AddVoice; it answers nonzero: the import 0x6fd200 on +0x38 with 1, its answer. Otherwise 0.
u32 __fastcall SoundMixer_AddVoiceSignal(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *sound);

// 0x6b6700 (thiscall; pops 0xc): in the section +4: a free voice taken (none: 0). The sound's first method answers 1: 0x6be750; otherwise the definition's flags word +0x10: bit 2 clear 0x6bd7b0, bit 4 0x6be680; with bit 1: bit 10 0x6be610, bit 8 0x6be330, else 0x6be090;
// without it: bit 10 0x6bdd30, else 0x6bdc60. Answers what TakeVoice wrote.
u32 __fastcall SoundMixer_Start(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *sound);

// 0x6b87d0 (thiscall; pops 8): with a mixer +0x20 and player +0x24: a copy of the category's deque iterator (+0x3c, 16 bytes) moved on by the sound's second method, its entry's +8 and the sound find a definition; a definition with flags bits 2, 3 clear must have the channels the player's bytes +0x44 to +0x47
// allow. A voice already at `id`, not flagged 0x10: its priority +0x22 at least the definition's +0xc ends here, otherwise flagged 0x100. The player's method 0x30 must agree; bit 0 of the flags: SoundMixer_AddVoiceSignal, else SoundMixer_Start's answer. Otherwise answers `id`.
u32 __fastcall SoundEngine_Play(u8 *engine, void *, u32 id, u8 *sound);
}
