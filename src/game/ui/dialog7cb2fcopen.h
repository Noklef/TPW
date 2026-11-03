// The options dialog (the window [0x7cb2fc], the options 0x78d8d8): opened, its handler, its slider and button answers. Reimplementation of 0x4a3a30,
// 0x4a2bf0, 0x4a2e90 and 0x4a3480. The names are invented, unconfirmed. The rows' dressing is src/game/ui/dialog7cb2fc.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x4092a0, 0x409300, 0x4237f0, 0x5996d0 agent B's and 0x662420 by address until ported; the bench's recorders: their own
// benches have the ports).
struct Dialog7cb2fcOpenEnv {
    u16 *(__cdecl *text)(u32 index);                                          // Language_WideText
    void(__cdecl *valueRow)(u32 row, u32 label, s32 value);                   // Dialog7cb2fc_SetValueRow
    void(__cdecl *toggle)(u32 row, u32 label, u8 on, u8 other, u8 select, u32 button);   // Dialog7cb2fc_SetToggle
    void(__cdecl *soundAbc)(u32 value);                                       // Sound_Set803abc
    void(__cdecl *soundAc0)(u32 value);                                       // Sound_Set803ac0
    void(__cdecl *soundAc4)(u32 value);                                       // Sound_Set803ac4
    void(__cdecl *f5996d0)(u32 value);                                        // 0x5996d0 (agent B's)
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                // Window_SendMessage
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                      // HoverLabel_Attach
    void(__cdecl *dressRow)(u32 row);                                         // Dialog7cb2fc_DressRow
    void(__cdecl *dressSlider)(u32 row, u32 value);                           // Dialog7cb2fc_DressSlider
    u32(__cdecl *focus)();                                                    // Focus_Restore
    u32(__cdecl *slot3)();                                                    // GuiManager_CallSlot3
    u32(__cdecl *slot4)();                                                    // GuiManager_CallSlot4
    s32(__cdecl *particles)();                                                // ParticlePool2_Finish
    u32(__cdecl *f662420)();                                                  // 0x662420
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);       // Window705a50_SetState
    void(__fastcall *f4092a0)(u8 *self, void *, u32 a, u32 b);                // 0x4092a0 (agent B's)
    void(__fastcall *f409300)(u8 *self);                                      // 0x409300 (agent B's)
    u32(__fastcall *flags)(const u8 *renderer);                               // Renderer_StateFlags
    void(__fastcall *setFlags)(u8 *renderer, void *, u32 flags);              // Renderer_SetStateFlags
    void(__fastcall *release)(u8 *cache, void *, u32 mode);                   // TexCache_ReleaseItems
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);     // WindowScript_Build
    void(__fastcall *setDepth)(u8 *window, void *, u32 depth);                // Window_SetDepth
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
    void(__fastcall *f4237f0)(u8 *options);                                   // 0x4237f0 (agent B's)
    void(__fastcall *revert)(u8 *options);                                    // Options_Revert
};
void Dialog7cb2fcOpen_SetEnv(const Dialog7cb2fcOpenEnv *env);

extern "C" {
// 0x4a2e90 (cdecl (id, value)): a slider's value shown. A text row is the row's child 0x1d4c9 (Window_FindChild twice, not checked) given by its slot 9 the label's
// text (widened to the buffer 0x7ba058) with a text after it (copied to the stack by wcsncpy of 0x3f units, its last never set). 0x1d4d5: Options_Set4 (value * 2 / 100),
// label 0x13f, then 0x155 + [0x78d8dc] (of 0-2; else the text 0). 0x1d4d6: Options_Set8 (value * (1 + ([0x78d8d8] != 0)) / 100), label 0x13e, 0x150 + [0x78d8e0].
// 0x1d4e0: Options_Set30, SetValueRow (0x140). 0x1d4d8, 0x1d4da, 0x1d4dc: the value stored at +0x14, +0x1c, +0x24 of the options; -1 unless the byte +0x10, +0x18, +0x20
// is set; SetValueRow (0x141, 0x142, 0x143); the volume (Sound_Set803abc, 803ac0, 803ac4) that, 0 when below 0. 0x1d4de: as those at +0x2c, byte +0x28, label 0x144,
// no sound. Others nothing.
void __cdecl Dialog7cb2fc_OnSlider(u32 id, s32 value);

// 0x4a3480 (cdecl (id, state)): a button pressed; off is state != 1. 0x1d4c1-0x1d4c5: the byte 0x78d90c, 0x78d90d, 0x78d90e, 0x78d90f, 0x78d911 = off; SetToggle (row
// 0x1d4cb-0x1d4cf, label 0x145-0x148 or 0x14b, off, 1, 0, 0). 0x1d4c6: [0x78d912] flipped, a text row 0x1d4d0 (label 0x15f; 0x160 + it). 0x1d4c7: [0x78d910] flipped, 0x1d4d1 (0x162;
// 0x163 + it). 0x1d4d2: the options' +0 flipped; the slider 0x1d4d6 given +8 * 50 by its slot 7, whose answer's low word 0 shows it (OnSlider); the row 0x1d4c8's 0x1d4c9 the text 0x13c (+0 set)
// or 0x13d. 0x1d4d3: Options_SetC (0 from set; 1 from clear when [[0x879a24] + 0x788] is above 1), a text row 0x1d4ca (0x15c; 0x15d + (+0xc set)). 0x1d4d7, 0x1d4d9, 0x1d4db, 0x1d4dd:
// the byte +0x10, +0x18, +0x20, +0x28 of the options = off; OnSlider (id + 1, the dword after it).
void __cdecl Dialog7cb2fc_OnButton(u32 id, u32 state);

// 0x4a2bf0 (cdecl (window, message, a, b)): 0x14: the render flags back from [0x7cb2f4], the dialog forgotten; with [0x786ba4] 0 and [0x7cb2ec] clear, [0x786ba4] = 1, 0x409300 of 0x786b68,
// 0x662420; the frame's window ([0xf82884] + 4; none: [0x7cb2ac]) sent 6 (1, 0); Focus_Restore; with [0x78d90e] GuiManager_CallSlot3, else Slot4; ParticlePool2_Finish. 0x100: a -2 Options_Revert, the
// dialog sent 4 (0, 0), answer 0; a -1 0x4237f0 of the options, answer 0; else OnButton (a, b). 0x800: OnSlider (a, b). 0x801: a 0x1d4d8, 0x1d4da, 0x1d4dc with their byte set the volume (as
// OnSlider) of b; a another of 0x1d4d9, 0x1d4db or outside 0x1d4d8-0x1d4e0: [0x7cb300] = 0. 0x802: [0x7cb300] = 0. All but those end in Window_CallDefault, its answer returned.
s32 __cdecl Dialog7cb2fc_Handler(u8 *window, u32 message, u32 a, s32 b);

// 0x4a3a30 (cdecl): with the dialog [0x7cb2fc] missing: Options_Keep; [0x7cb300] = 0; with [0x786ba4] 1 0x4092a0 of 0x786b68 (0, 0), [0x786ba4] = 0, Fn_662411, [0x7cb2ec] = 0, else [0x7cb2ec] = 1;
// 0x5996d0 (1); [0x7cb2f4] = the render flags, set to 0x21; the texture cache [0x8c4b44] released (0); the frame's window sent 6 (0, 0); the dialog built (script 0x752f30, handler 0x4a2bf0), its
// label 0x1d4d4 attached (the text 0x13b); the children's depth set to the first's +0xe0 + 2; the rows and sliders dressed; each shown from the options, as OnSlider / OnButton show them.
void __cdecl Dialog7cb2fc_Open();
}
