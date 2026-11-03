// The park statistics graph (a Gauge at 0x7ca250 whose marks are set from these; the lines shown by the flags byte 0x751fa0, the window by the kind at
// 0x7ca2ec): reimplementation of 0x49a7b0, 0x49a8f0, 0x4a51d0 and 0x49b520.
#pragma once

#include "../../../re/types.h"

// What the redraw hands on that is not yet ported (the benches point it at a recorder).
struct StatsGraphEnv {
    void(__cdecl *lines49aa30)(u8 *gauge);   // 0x49aa30, the graph's lines drawn over the gauge (Gauge_DrawInWindow's after)
    void(__cdecl *lines4a4750)(u8 *gauge);   // 0x4a4750, the same for the graph at 0x7cb318
    void(__cdecl *clearButtons)(u32 mode);   // Window7ca2e8_ClearButtons (its bench has the dialog's buttons)
};
// StatsGraph_Redraw's pointer comes from env (nullptr: the game's).
void StatsGraph_SetEnv(const StatsGraphEnv *env);

extern "C" {

// 0x49a7b0 / 0x49a8f0 (cdecl; names unconfirmed): from 0, the greatest / least sample (CircBufU32_144_MaxInWindow / MinInWindow with the kind [0x7ca2ec]) of
// each shown line: bits 0-7 of 0x751fa0 (re-read for each) select the CircBufU32s of 144 at +0x1f100, +0x1f350, +0x1fc90, +0x1fee0, +0x20130, +0x20380,
// +0x1f5a0 and +0x1f7f0 of the game's record (GameTable7cfb90_Get1da720_519510 of the game object [0x7cf83c]).
s32 __cdecl StatsGraph_ShownMax();
s32 __cdecl StatsGraph_ShownMin();

// 0x4a51d0 (cdecl; name unconfirmed): the same for the graph whose Gauge is at 0x7cb318: from 0, the greatest sample (window kind [0x7cb39c]) of
// each line shown by bits 0-4 of 0x753c48: the CircBufU32s of 144 at +0x20cc0, +0x21160, +0x205d0 and +0x213b0 (CircBufU32_144_MaxInWindow) and,
// for bit 1, the CircBufF32 at +0x20f10 (CircBufF32_144_MaxInWindow), of the game's record.
s32 __cdecl StatsGraph7cb318_ShownMax();

// 0x49b520 (cdecl; called from 0x4f8440; name invented): when the dialog [0x7ca2e8] is open, the Gauge 0x7ca250's marks' y set to the
// shown lines' least and greatest sample and 10 (StatsGraph_ShownMax first, then StatsGraph_ShownMin; Gauge_SetMarksY), then the gauge
// drawn in the dialog's child 0x12272 (Window_FindChild; not checked) with the lines drawn after it by 0x49aa30 (Gauge_DrawInWindow).
void __cdecl StatsGraph_Redraw();

// 0x49a2e0 (cdecl (window, message, a, b): the window of the graph whose Gauge is at 0x7ca250; name invented): a GuiRect on the stack around it. Message 0x15: the
// saved rectangle (Window_GetSavedRect) sets the gauge up (Gauge_Setup: least and greatest as 0x49b520, 10, then 0, 0xb, 0xc), drawn. 0x100: a -2: the dialog [0x7ca2e8] sent 4
// (0, 0), answer 0 at once. The boxes a = 0x12277 to 0x1227e: bits 0, 3, 2, 6, 1, 4, 5, 7 of the flags 0x751fa0: b 1 sets it, first clearing the other group where any of it is set:
// bits 0 and 1 against 0xfc, the rest against 3 (env's buttons cleared, the bits off); else it clears the bit. Then the marks' y (StatsGraph_ShownMax first; Gauge_SetMarksY) and
// drawn. 0x101, a = 0x12274, 0x12275, 0x12276: the window kind [0x7ca2ec] 2, 0, 1; with b 1 the marks' x (0, 0xb, 0xc; 0, 0x23, 0x24; 0, 0x8f, 0x90 for kinds 0, 1, 2), y and
// drawn. Drawn is Gauge_DrawInWindow with 0x49aa30 after it. Every message but the one a -2 then goes to Window_CallDefault, whose answer is returned.
u32 __cdecl StatsGraph_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a4b90 (cdecl (window, message, a, b): the handler Dialog7cb398_Create gives its child 0x4a68bb; name invented): the same for the Gauge at 0x7cb318, flags 0x753c48 and kind
// [0x7cb39c]. 0x15: Gauge_Setup with 0, the greatest (StatsGraph7cb318_ShownMax), 10, 0, 0xb, 0xc and the marks' x; 0x100, a = 0x4a68bc to 0x4a68c0: b 1 sets bits 0, 1, 2, 3, 4, else
// clears it, then the marks' y (0, the greatest, 10); 0x101, a = 0x4a68c2, 0x4a68c3, 0x4a68c4: kinds 2, 0, 1, as 0x49a2e0. Drawn with 0x4a4750 after it.
u32 __cdecl StatsGraph7cb318_Handler(u8 *window, u32 message, u32 a, u32 b);
}
