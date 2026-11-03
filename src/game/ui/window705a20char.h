// A character typed into the edit window 0x705a20. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the typing calls (the benches point these at recorders): the ports.
struct Window705a20CharEnv {
    void(__fastcall *notifyText)(u8 *window, void *, u32 post);                               // Window705a20_NotifyText 0x66671d
    void(__fastcall *setSelection)(u8 *window, void *, u32 cursor, u32 mark, u32 anchor);     // Window705a20_SetSelection 0x6668e9
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                                // Window_SendMessage 0x658d9f
    void(__fastcall *typeChar)(u8 *window, void *, u32 ch);                                   // Window705a20_TypeChar 0x667833
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

// 0x667a72 (thiscall (key: the low word, sign-extended; b), pops 8; name invented): nothing without the buffer. The modifiers are b's third byte; 3 moves the anchor (cursor -1, 1, code)
// and anything else the cursor (code, 1, -1). 0x2700 right: code -3 (with the byte 0xfaa56c -2); 0x2500 left: -2 (-3); 0x2300 end: -5; 0x2400 home: -4. 0x2600, 0x2800 up, down in a
// window with flag 0x80 of +0x48: (-6, 1, -1), (-7, 1, -1); else, as every other key, the parent +0x60 sent 0x805 (the id +0x8c, the key). 0x2e00 delete, 8 backspace: a selection gives way
// (signed loops, the rest of the capacity +0x13c zeroed; delete then the cursor, backspace notifying first and then its start, (place, 1, -1)); else one character goes: after the cursor if
// it is below the length, before it if above 0 (backspace, then (-2, 1, -1)); Window705a20_NotifyText(1). 0xd in a window with the flag: the character 0xa typed. EAX not kept.
void __fastcall Window705a20_KeyDown(u8 *window, void *, u32 key, u32 b);
}
