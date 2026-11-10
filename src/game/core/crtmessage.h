// The C library's fatal messages and path splitting (MSVC6's own names): _amsg_exit and its banner, the message box, _mbsnbcpy, _splitpath.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x67efc0, 0x67eff0: _amsg_exit, _fast_error_exit: with [0xfb135c] 1 the banner, the message, then exit(0xff) by [0x77ea80] or ExitProcess(0xff).
void __cdecl CRT_amsg_exit(s32 code);
void __cdecl CRT_purecall();  // 0x67b0c0
void __cdecl CRT_fptrap();    // 0x68c310
void __cdecl CRT_fast_error_exit(s32 code);

// 0x68a170: _FF_MSGBANNER: on a console (see _NMSG_WRITE) message 0xfc, the hook [0xfb1888] when set, message 0xff.
void __cdecl CRT_FF_MSGBANNER();

// 0x68a1b0: _NMSG_WRITE: the text of the table 0x781680 (18 pairs; sic: a code past them is compared with the word after the table and then uses that entry). On a console ([0xfb135c] 1, or 0 with
// [0x77ea84] 1) written to the handle of fd 2 or, without one, STD_ERROR_HANDLE. Otherwise, but for 0xfc, a message box with the program's path, its last 0x3b characters when longer, "..." on their first three.
void __cdecl CRT_NMSG_WRITE(s32 code);

// 0x68efb0: __crtMessageBoxA: MessageBoxA of user32.dll, loaded on first use with GetActiveWindow and GetLastActivePopup for its owner. Not found: 0.
s32 __cdecl CRT_crtMessageBoxA(const char *text, const char *title, u32 type);

// 0x68a390: _mbsnbcpy: strncpy without a multibyte code page ([0xfb1870] 0). With one, under lock 0x19, by characters: a lead byte cut off or followed by the end leaves a zero in its place; padded. Returns to.
char *__cdecl CRT_mbsnbcpy(char *to, const char *from, u32 n);

// 0x67f020: _splitpath: the parts that are asked for, at most 0xff bytes each.
void __cdecl CRT__splitpath(const char *path, char *drive, char *dir, char *name, char *ext);

}

// user32's functions __crtMessageBoxA finds on first use, and the hook _FF_MSGBANNER calls first.
using CrtMessageBoxFn = s32(__stdcall *)(u32, const char *, const char *, u32);
using CrtGetActiveWindowFn = u32(__stdcall *)();
using CrtGetLastActivePopupFn = u32(__stdcall *)(u32);
using CrtMessageHookFn = void(__cdecl *)();

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const char[0x28], CRT_text_runtime_library_709034, 0x00709034, 0x28);
GAME_DATA(const char[0x14], CRT_text_GetLastActivePopup_7096f4, 0x007096f4, 0x14);
GAME_DATA(const char[0x10], CRT_text_GetActiveWindow_709708, 0x00709708, 0x10);
GAME_DATA(const char[0xc], CRT_text_user32_709724, 0x00709724, 0xc);
GAME_DATA(u32, CRT_app_type, 0x0077ea84, 0x4);
GAME_DATA(u32, CRT_error_mode_fb135c, 0x00fb135c, 0x4);
GAME_DATA(CrtMessageHookFn, CRT_msgbanner_hook_fb1888, 0x00fb1888, 0x4);
GAME_DATA(CrtMessageBoxFn, CRT_pfnMessageBox, 0x00fb1900, 0x4);
GAME_DATA(CrtGetActiveWindowFn, CRT_pfnGetActiveWindow, 0x00fb1904, 0x4);
GAME_DATA(CrtGetLastActivePopupFn, CRT_pfnGetLastActivePopup, 0x00fb1908, 0x4);
