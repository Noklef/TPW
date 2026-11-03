// The dialog [0x7cc188]'s address box: what the Enter key does with the address typed into the buffer 0x7cb8d8. Reimplementation of 0x4a77f0 and 0x4a77c0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct StdString;

// The calls (the ports by default; 0x5cd510 and 0x47f020, agent B's, by address; the bench's recorders: their own benches have them).
struct Dialog7cc188AddressEnv {
    void(__fastcall *addName)(u8 *list, void *, const StdString *name);                  // 0x5cd510
    void(__cdecl *fillList)(u32 clear);                                                  // Dialog7cc188_FillList
    void(__fastcall *scrollTo)(u8 *window, void *, s32 position);                        // Window7059f0_ScrollTo
    u16 *(__cdecl *wideText)(u32 index);                                                 // Language_WideText
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const void *arg);  // Charset_FormatWide
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    s32(__fastcall *handler)(u8 *window, void *, u32 message, u32 a, u32 b);             // 0x65f66e
};
void Dialog7cc188Address_SetEnv(const Dialog7cc188AddressEnv *env);   // nullptr: the game's

extern "C" {

// 0x4a77f0 (cdecl). The buffer 0x7cb8d8 needs a '@' past its start and a '.' more than one place past that (wcschr, the first of each), else: the text 0xd5 of [0x78ba94] formatted with it as the argument 3 (Charset_FormatWide) in a box
// (0x47f020: colour ff ff ff, kind 4; 1, then zeros), nothing else. Otherwise: it as narrow text ("%.63S") in a std::string told to the user slots' list +0x100 (0x5cd510, UserSlots_Instance inlined), the list refilled (clear 1), its child 0x26 scrolled to -1.
// Then unless it is already in the recipients 0x7cb4d0 (wcsstr): the recipients become ";it" (";%s"), when they were the text 0xd3 the whole, else added at the end, and go to the child 0x5da5 of [0x7cc184] by its slot 9.
void __cdecl Dialog7cc188_AddAddress();

// 0x4a77c0 (cdecl (window, message, a, b), a window handler): the message 0x1000b with a 0xd (Enter) adds the address first; the answer is 0x65f66e's (thiscall on the window).
s32 __cdecl Dialog7cc188_AddressHandler(u8 *window, u32 message, u32 a, u32 b);
}
