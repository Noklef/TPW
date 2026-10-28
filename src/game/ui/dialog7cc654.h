// The window at 0x7cc654 (its child 0xee2 a text window showing the buffer 0x7cb4d0, with the scroll bar 0xee3; ui/listrows holds its
// list child's rows). Reimplementation of 0x4bc100. The names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4bc100 (no arguments; called after the buffer 0x7cb4d0 is filled): the text window (child 0xee2 of [0x7cc654]) and its bar (its child
// 0xee3; neither checked). The text window's saved rectangle (Window_GetSavedRect) scaled to the screen (width * [0xfaa5c4] / 2048, height
// * [0xfaa5c0] / 1536, at 0, 0) is where the buffer is measured (gfx/guitext's Measure: font 6, aligned 0 / 0, fit 1); the measured bottom
// scaled back (* 1536 / [0xfaa5c0]) over the rectangle's height shows the bar (Window_SetShown(1)) with the range (the excess, 0)
// (Window705b20_SetRange), else hides it. Then, when the bar's slot 7 (given 0) answers 0 in AX, the text window's slot 9 is given the buffer.
void __cdecl Dialog7cc654_FitText();
}
