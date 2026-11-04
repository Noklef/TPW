// The window's name: 16 wide characters at +0x6c. Reimplementation of 0x65d40c and 0x65d521. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the game's by default; the bench's recorders: the library's own).
struct WindowNameEnv {
    void *(__cdecl *wcsncpy)(void *to, const void *from, u32 count);   // CRT_wcsncpy
    void *(__cdecl *wcscpy)(void *to, const void *from);               // CRT_wcscpy
};
void WindowName_SetEnv(const WindowNameEnv *env);

extern "C" {

// 0x65d40c (thiscall (name), pops 4): 15 wide characters of the name into +0x6c (wcsncpy: no terminator past them).
void __fastcall Window_SetName(u8 *self, void *, const void *name);

// 0x65d521 (thiscall (out), pops 4): with an out the name copied there (wcscpy).
void __fastcall Window_GetName(u8 *self, void *, void *out);
}
