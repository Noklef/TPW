// The screen's window and DirectDraw object (0x5fa030-0x5fa641): the Bullfrog library's screen
// window class ("_BullfrogLibScreen") and window procedure, the DirectDraw object and the list of
// display modes it offers, and letting go of the screen's surfaces.
//
// Screen_Open makes a borderless topmost window the size of the desktop unless the caller gives
// one, and lists the display modes (through a throwaway DirectDraw object; with exclusive set,
// after taking exclusive mode on a window, a hidden 1280x1024 "ModeList" one unless given).
// Setting a mode (elsewhere) creates the surfaces and the DirectDraw object kept at +8.
//
// The originals' quirks are kept: the window procedure counts the window as active only for
// wParam 1 or 2 exactly (a minimized activation is not), and swallows SC_KEYMENU so Alt does not
// open the system menu; listing modes with exclusive set leaks the DirectDraw object when the
// "ModeList" window cannot be made; when listing fails, the window Screen_Open made is never
// destroyed (Screen_Close destroys it only once open).
#pragma once

#include <cstddef>

#include "../../../re/types.h"
#include "../core/fpu.h"
#include "../core/list.h"
#include "surface.h"

struct ScreenState {     // 0xfa7948
    void *coopWindow;    // the window given exclusive mode, handed back to normal on release
    void *window;        // +4 (0xfa794c): the window Screen_Open made (0: the caller's)
    void *directDraw;    // +8 (0xfa7950)
    u32 unknown0c;       // zeroed on release
    void *unknown10;     // +0x10 (0xfa7958): a COM object, released on release
    s32 active;          // +0x14 (0xfa795c): Screen_SetActive
    u32 unknown18;       // zeroed on release
    const void *guid;    // +0x1c (0xfa7964): the DirectDraw driver Screen_Open was given
    u32 unknown20;
    s32 classRegistered; // +0x24 (0xfa796c)
};
static_assert(offsetof(ScreenState, classRegistered) == 0x24, "ScreenState::classRegistered is at +0x24");

struct ScreenMode {      // a node of the mode list (0xfa77d0)
    ListNode link;
    u32 width;           // +8
    u32 height;          // +0xc
    u32 bitCount;        // +0x10
};
static_assert(sizeof(ScreenMode) == 0x14, "ScreenMode is 0x14 bytes");

extern "C" {
void __cdecl Screen_SetGuid(const void *guid);          // 0x5fa030
void *__cdecl Screen_GetDirectDraw(s32 addRef);         // 0x5fa040: AddRef'd when asked
// 0x5fa060: when open, releases the surfaces and the DirectDraw object and destroys the window it
// made; then empties the mode list (always).
void __cdecl Screen_Close();
// 0x5fa0e0: the FPU guard restored, inactive; when there is a primary surface (0xfa7780), posts
// event {5, 0}, hands the window back to normal mode and releases the surfaces and +0x10.
void __cdecl Screen_ReleaseSurfaces();
// 0x5fa1b0: RestoreDisplayMode and Release; Surface_SetDirectDraw(0).
void __cdecl Screen_ReleaseDirectDraw();
// 0x5fa1e0 (&out, guid, exclusive, window or 0 for one of its own, window for exclusive listing):
// the window and the mode list; 0 straight away when already open.
s32 *__cdecl Screen_Open(s32 *out, const void *guid, s32 exclusive, void *window, void *listWindow);
// 0x5fa2b0 (&out, guid, &directDraw, unused): DirectDrawCreate; then the application's window is
// the one Screen_Open made, if any.
s32 *__cdecl Screen_CreateDirectDraw(s32 *out, const void *guid, void **directDraw, void *unused);
// 0x5fa2f0 (&out, exclusive, window, guid): the mode list rebuilt from EnumDisplayModes (the old
// nodes are not freed).
s32 *__cdecl Screen_ListModes(s32 *out, s32 exclusive, void *window, const void *guid);
// 0x5fa440: the EnumDisplayModes callback: a node at the end of the list; 0 (stop) when out of
// memory.
s32 __stdcall Screen_AddMode(const u8 *desc, List *modes);
void __cdecl Screen_RegisterClass();  // 0x5fa490: once
// 0x5fa540: the screen window's procedure. WM_ACTIVATE (for the application's window) sets the
// active state and posts event {4, active}; WM_CLOSE asks the application to quit;
// WM_ACTIVATEAPP sets the application's active flag; the rest go to the application's window
// procedure (App_GetWndProc) or DefWindowProc.
s32 __stdcall Screen_WndProc(void *window, u32 message, u32 wParam, s32 lParam);
void __cdecl Screen_SetActive(s32 active);  // 0x5fa630: and hides the cursor (SetCursor(0))
}

struct ScreenEnv {
    ScreenState *state;    // 0xfa7948
    s32 *open;             // 0xfa9f3c
    List *modes;           // 0xfa77d0
    char *className;       // 0xfa77e0
    u8 *lockFlag;          // 0xfa78e8: cleared on release
    FpuControl *fpu;       // 0xfa78ec
    Surface *surfaceA;     // 0xfa78f8
    Surface *surfaceB;     // 0xfa7730
    Surface *primary;      // 0xfa7780
    void *queue;           // 0xfa7cb0: the event queue
    void(__fastcall *post)(void *queue, void *edx, u32 unknown, const u32 *event);  // 0x5fbf80
    void *(__cdecl *alloc)(u32 size);  // 0x67acc0
    void(__cdecl *free)(void *p);      // 0x67acb0
    const char *listTitle;             // 0x775840: "ModeList"
    const char *classSource;           // 0x77584c: "_BullfrogLibScreen"
    void *wndProc;                     // 0x5fa540
    void *(__stdcall *const *createWindowEx)(u32, const char *, const char *, u32, s32, s32, s32, s32, void *, void *,
                                             void *, void *);  // 0x6fd398
    s32(__stdcall *const *getSystemMetrics)(s32);                // 0x6fd338
    s32(__stdcall *const *destroyWindow)(void *);                // 0x6fd33c
    void *(__stdcall *const *loadIcon)(void *, const char *);    // 0x6fd364
    void *(__stdcall *const *getStockObject)(s32);               // 0x6fd054
    u16(__stdcall *const *registerClass)(const void *);          // 0x6fd394
    s32(__stdcall *const *defWindowProc)(void *, u32, u32, s32); // 0x6fd340
    void *(__stdcall *const *setCursor)(void *);                 // 0x6fd334
    s32(__stdcall *const *directDrawCreate)(const void *, void **, void *);  // 0x6fd018
};
// The state and calls go through env (nullptr: the game's).
void Screen_SetEnv(const ScreenEnv *env);
const ScreenEnv *Screen_GetEnv();
