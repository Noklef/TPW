// The front-end frame, the object [0xf82884] points to (class 0x7029f0: its constructor, destructor and ticks in src/game/core/vdtor and
// src/game/core/vlist): reimplementation of its close, 0x5d5cf0 (thiscall).
#pragma once

#include "../../../re/types.h"

struct FrameEnv {
    u32(__cdecl *showChild)();                                                      // DialogF82884_ShowChild
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);    // Window_CallDefault
};

void Frame_SetEnv(const FrameEnv *env);

// The callees of Frame_Start (tests/fncmp/framestart.py gives them recorders).
struct FrameStartEnv {
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);  // WindowScript_Build
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                  // Window_SetFocus
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);               // Window_FindChild
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);          // Window_SetHandler
    u8 *(__fastcall *userSlots)(u8 *self);                                 // UserSlots_Ctor
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);             // Window_SendMessage
    void(__cdecl *label)(u8 front);                                        // VersionLabel_Create
    u32(__fastcall *now)(u8 *clock);                                       // Clocks_Now
    void(__cdecl *sound)();                                                // Sound_RegisterGlobalLobbySfx
    void(__fastcall *camera)(u8 *settings);                                // IslandCamera_LoadSettings
    u8 *(__fastcall *make702a60)(u8 *object);                              // Class702a60_Ctor
    u8 *(__fastcall *make702a90)(u8 *object);                              // Class702a90_Ctor
    u8 *(__fastcall *make702b08)(u8 *object);                              // Class702b08_Ctor
    u8 *(__fastcall *make702d78)(u8 *object);                              // Class702d78_Ctor
    u8 *(__fastcall *online)(u8 *session);                                 // Online_Construct
    void(__cdecl *openDialog)();                                           // Dialog7cb3f8_Open
};
void Frame_SetStartEnv(const FrameStartEnv *env);  // nullptr: the game's

extern "C" {

// 0x5d5dd0 (thiscall (window, message, wparam, lparam), pops 16; name invented, unconfirmed): 0x11 with wparam 1: DialogF82884_ShowChild. 0x14: +4 = 0 with a frame at [0xf82884]. 0x22: +0x1c = 0.
// 0x10003: +0x20 and +0x24 = the high and low words of wparam. 0x10004 and 0x10005: wparam 0, 1 or 2 clears and sets bits 1, 2, 4 of +0x1c. Then with a frame the list at +0x18 (as below) gets slot 5 of each
// object with (message, wparam, lparam). Returns Window_CallDefault's answer.
s32 __fastcall Frame_HandleMessage(u8 *frame, void *, u8 *window, u32 message, u32 wparam, u32 lparam);
// 0x5d58b0 (cdecl; name invented, unconfirmed): that on the frame at [0xf82884]; with none 0.
s32 __cdecl Frame_WindowProc(u8 *window, u32 message, u32 wparam, u32 lparam);

// 0x5d5cf0 (thiscall; called from 0x54e7c0 on [0xf82884]; name invented, unconfirmed): over the list whose sentinel is at +0x18 (the first
// node at its +4; nodes {object, next}, the next read first) each object's slot 4 is called; then the objects [0xf83784], [0xf83778],
// [0xf83774], [0xf85614] and +0x10 each deleted (slot 0 with 1) unless 0, and cleared; Sound_ReleaseLobbySfx; the message 4 (0, 0) sent to
// the window +4 (Window_SendMessage). Returns the dword at +0x14.
u32 __fastcall Class7029f0_Close(u8 *frame);

// 0x5d5970 (thiscall; name invented, unconfirmed): +0x14 = 1. With a window maker at [0xfaa5d8]: the window +4 built from the script 0x774c18, focused; its child 0xbf432 gets the handler
// 0x5d58e0 and, unless the user slots' +0xc4, the message 6; the version label. The clock's now at +0x28 and +0x2c, +0x1c = 0, the lobby sounds. The objects +0x10 (0x6c bytes, its settings
// loaded), [0xf83774], [0xf83778], [0xf83784], [0xf85614] made; slot 4 of each object in the list at +0x18. Then slot 7 of a panel of [0xf85614]: online (+0x108) its +0x10; else with [0xf8288c] its
// +0xc, the flag cleared; else its +4, after the dialog 0x7cb3f8 when the user slots' +0x60 is -1. Last slot 7 of its +0x14. Returns +0x14.
u32 __fastcall Frame_Start(u8 *frame);
}
