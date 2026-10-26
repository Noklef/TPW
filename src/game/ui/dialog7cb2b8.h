// The dialog at 0x7cb2b8 (the message panel: its child 0x21 the box of message tags, the handler 0x4a14f0 set by 0x4a238f). Reimplementation
// of 0x4a1440 and 0x4a14f0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a1440 (cdecl (window, limit); called by itself, 0x4a16b4 and 0x4a1ad9): a window (when not 0) shown when the right edge of its inner
// rectangle (Window_GetRect 1) is past the limit's low word (signed words), else hidden (Window_SetShown); then the same for each of its
// children (Fn_65d547's first, then +0x54), depth first. For no window its children are read all the same (a fault, sic). EAX not kept.
void __cdecl Dialog7cb2b8_ShowRightOf(u8 *window, u32 limit);

// 0x4a14f0 (cdecl (window, message, a, b); the handler of the dialog's child 0x21, the box the tags slide out of, set by 0x4a238f; name
// unconfirmed). Its dword +0x130 counts the slide's steps (10), its byte +0x134 is the state (0 idle, 2 sliding in, 3 / 4 sliding out),
// its +0xa0 dwords 0 the step, 1 the tag shown, 2 a tag waiting; the slide runs on its timer 0x80080 (Window_SetTimer at 20, 0 in AX
// ending the message there). Then (always) Window_CallDefault's answer.
// 0xa (a the tag): with a state, the tag kept waiting (dword 2) when not 0; else the count 10 and the tag made the box's last child (depth
// 1, words 1: Window_AddChild; refused, nothing more): its depth word less 3 (Window_SetDepth), its rectangle set (Window_SetRectTo) to
// its own size (GuiRect_GetSize of its inner rectangle) at 50 left of the box's inner left and centred on the box's height (signed words,
// halves rounded to 0), shown, and it and its children shown by their inner right edges past the box's inner left (as
// Dialog7cb2b8_ShowRightOf); the step (the dialog [0x7cb2b8]'s inner right - the box's inner left + the tag's width + 100, a signed word)
// / the count, a signed word; dword 1 the tag, the timer started, the state 2, the child 0x24's slot 5 with 1, the sound 0x1c of [0x803a2c]
// (Sound_PlayAt) and the tag sent 0x1001d9.
// 0xb (a -1 or not): with a tag shown and the timer set, the tag sent 0x1001d8, the count 10, the step (the box's inner left - the window
// [0x7cb2d0]'s, signed words) / the count, a signed word; the timer started, the state 4 for a -1, else 3, the child 0x24's slot 5 with
// 0, the sound.
// 0x10 (a 0x80080): with a count left, one less and the box's inner rectangle widened on the right by the step's word (Window_SetRect);
// else the timer stopped, the box's rectangle +0x14 kept (Window_GetRect14) and, with a tag shown: for the state 3 or 4 the tag hidden and
// unlinked (Window_Unlink), dword 1 cleared, the box's rectangle set to the kept one, then for 3 the tag sent 0x1d (0, 0) and the sound
// 0xc2 (SoundUi_Play), for 4 the box's parent posted 0xb; for another state the tag sent 0x1d (1, 0). The state 0, and a tag waiting
// posted to the box as 0xa, the dword cleared.
// 0x12 (a the box's new left): with a tag shown, it moved across by a less the box's old width (GuiRect_GetSize of the rectangle +0x20; sic:
// the width word written into a's stack slot and read back as a dword, a's upper half with it), then it and its children shown by their
// inner right edges past the parent's inner right (as Dialog7cb2b8_ShowRightOf).
// 0x100 (a 0x24, the close button): Dialog7cb2b8_ResetChild21(1)'s work (inlined in the original). EAX the default handler's.
s32 __cdecl Dialog7cb2b8_BoxHandler(u8 *window, u32 message, u32 a, u32 b);
}
