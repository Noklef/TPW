// The dialog at 0x7cb2b8 (the message panel: its child 0x21 the box of message tags, the handler 0x4a14f0 set by 0x4a238f). Reimplementation
// of 0x4a1440. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a1440 (cdecl (window, limit); called by itself, 0x4a16b4 and 0x4a1ad9): a window (when not 0) shown when the right edge of its inner
// rectangle (Window_GetRect 1) is past the limit's low word (signed words), else hidden (Window_SetShown); then the same for each of its
// children (Fn_65d547's first, then +0x54), depth first. For no window its children are read all the same (a fault, sic). EAX not kept.
void __cdecl Dialog7cb2b8_ShowRightOf(u8 *window, u32 limit);
}
