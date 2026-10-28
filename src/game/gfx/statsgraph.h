// The park statistics graph (a Gauge at 0x7ca250 whose marks are set from these; the lines shown by the flags byte 0x751fa0, the window by the kind at
// 0x7ca2ec): reimplementation of 0x49a7b0, 0x49a8f0, 0x4a51d0 and 0x49b520.
#pragma once

#include "../../../re/types.h"

// What the redraw hands on that is not yet ported (the benches point it at a recorder).
struct StatsGraphEnv {
    void(__cdecl *lines49aa30)(u8 *gauge);   // 0x49aa30, the graph's lines drawn over the gauge (Gauge_DrawInWindow's after)
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
}
