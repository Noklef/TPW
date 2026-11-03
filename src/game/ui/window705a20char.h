// A character typed into the edit window 0x705a20. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the typing calls (the benches point these at recorders): the ports.
struct Window705a20CharEnv {
    void(__fastcall *notifyText)(u8 *window, void *, u32 post);                               // Window705a20_NotifyText 0x66671d
    void(__fastcall *setSelection)(u8 *window, void *, u32 cursor, u32 mark, u32 anchor);     // Window705a20_SetSelection 0x6668e9
};
// The typing's calls go to env (nullptr: the defaults).
void Window705a20Char_SetEnv(const Window705a20CharEnv *env);

// 0x6687e0 (cdecl (list, ch: the low word); name invented): the character's place in the zero-ended list (wcschr 0x67c2f0, inlined here; the end for ch 0), else null.
const u16 *__cdecl CharList_Find(const u16 *list, u32 ch);

// 0x667833 (thiscall (ch: the low word), pops 4; name invented): nothing without the buffer +0x138, for 0xd, 9, 8, 0x1b, a character not in the list +0x140 or one in the list +0x144
// (either list when set). With the anchor +0x14c set (not -1) the selection between it and the cursor +0x14a (signed) gives way: the text after it comes down (signed loop, up to
// the length less the gap), the rest of the capacity +0x13c zeroed (unsigned) and the character goes at its start. Without one, when the length is below the capacity, the text
// from the cursor moves up (unsigned compare with the sign-extended cursor) and the character goes at the cursor. A place below 0 ends it; else Window705a20_NotifyText(1) and
// Window705a20_SetSelection(place + 1, 1, -1). EAX not kept.
void __fastcall Window705a20_TypeChar(u8 *window, void *, u32 ch);
}
