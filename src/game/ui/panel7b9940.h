// The game object at 0x7b9940 (built by 0x4855d0): a renderable with a virtual base (its vtable pointer +0, the base's offset at the
// vtable's +4) that the renderer holds in its tag 0x20 list while it is shown (0x48a410 adds it, 0x48a520 and 0x48a3d0 take it out).
// Reimplementation of 0x4855d0, 0x489f50, 0x48a290, 0x48a310, 0x48a3d0, 0x48a520 and 0x48a6d0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Panel7b9940_Exit's call (the port by default; the bench's recorder: menuactions' bench has it).
struct Panel7b9940ExitEnv {
    void(__cdecl *closeMenu)();   // Window7cc8bc_Close_4bf8e0
};
void Panel7b9940_SetExitEnv(const Panel7b9940ExitEnv *env);

// Panel7b9940_Open's calls (the ports by default; the bench's recorders: their own benches have them). SceneFlags_Touch and the GuiRect's run.
struct Panel7b9940OpenEnv {
    void(__cdecl *resetQueues)();                                                                           // Gui_ResetQueues
    void(__cdecl *msgTags)();                                                                               // MsgTags_Create
    void *(__cdecl *alloc)(u32 size);                                                                       // Mem_Alloc
    u8 *(__fastcall *modeBase)(u8 *self);                                                                   // MouseModeOnlineBase_Ctor
    void(__stdcall *setMode)(u8 *mode);                                                                     // MouseMode_Set
    u32(__fastcall *add)(u8 *renderer, void *, u8 *object, u32 tag);                                        // Renderer_Add
    u8 *(__fastcall *create)(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);   // Window_CreateChild
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                                               // Window_SetShown
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);                                           // Window_SetHandler
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);                                      // Window_GetRect
    void(__fastcall *setCursor)(u8 *window, void *, u32 id);                                                // Window_SetCursor
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);                                   // WindowScript_Build
    void(__cdecl *build7cb2b8)();                                                                           // Dialog7cb2b8_Build
    void(__cdecl *create7ca1e0)();                                                                          // Dialog7ca1e0_Create
    void(__cdecl *createBrowser)();                                                                         // ThingBrowser_Create
    void(__cdecl *create7ca1bc)();                                                                          // Dialog7ca1bc_Create
    void(__cdecl *create7ca098)();                                                                          // Dialog7ca098_Create
    void(__cdecl *create7ca1c8)();                                                                          // Dialog7ca1c8_Create
    u32(__cdecl *focus)();                                                                                  // Focus_Restore
    u32(__cdecl *slot4)();                                                                                  // GuiManager_CallSlot4
    u32(__cdecl *slot3)();                                                                                  // GuiManager_CallSlot3
};
void Panel7b9940_SetOpenEnv(const Panel7b9940OpenEnv *env);

extern "C" {

// 0x48a3d0 (no arguments; also reached through the jump 0x481410, called with ECX 0x7b51f0 it ignores): the GUI manager's slot 4
// (GuiManager_CallSlot4), then the object's virtual base (0x7b9940 + the offset at its vtable's +4; the original's test of 0x7b9940
// against 0 always passes) taken out of the renderer's ([0x87a6d0]) tag 0x20 list (Renderer_RemoveFromSlot); returns what that returns.
u32 __cdecl Panel7b9940_Detach();

// 0x48a520 (cdecl; name invented, unconfirmed): the window [0x7cc8bc] closed (Window7cc8bc_Close_4bf8e0), message 4 (0, 0) to the window [0x7cb2b0], then as Panel7b9940_Detach; returns what that returns.
u32 __cdecl Panel7b9940_Exit();

// 0x489f50 (cdecl; name invented, unconfirmed): the front end's windows and panels made. The GUI's queues reset, the message tags made; the mouse mode [0x7b05e8] made once (Mem_Alloc(1), with a
// MouseModeOnlineBase of 8 bytes, none when that allocation fails, MouseMode_Set on it; [0x7b05e0] and [0x7b05d4] -1; a failed first allocation stores 0 and leaves those); the object [0x7b05d8]'s
// slot 9 if any. In the object's second base (0x7b9940 + its vbtable's +8, read again after each touch): the scene flags touched, +0x54 -1 and +4..+0x10 the float at 0x6ff148; touched again, +0x2c, +0x30
// and +0x34 -0.3f, -0.5f and 0.8f. The virtual base added to the renderer's tag 0x20 list. The window [0x7cb2ac] (child 1, 0, 0, 0, 0, 0x7ff, 0x5ff of [0xfaa5d8]; none: left): shown, the handler
// 0x4881a0, its rectangle (Window_GetRect, inner) giving the screen floats 0x7a1c54 and 0x7a1c58 (the centre, scaled by [0xfaa5c4] / 0x800 and [0xfaa5c0] / 0x600) and 0x74d104 and 0x74d108 (the half sizes,
// scaled so; every division truncated). The window [0x7cb2b0] (child 1, 0, 1, 0, 0, 0x7ff, 0x5ff; none: left): the handler 0x488a00, the cursor 0x15, the script 0x74fa98 built on it, no handler.
// Then the dialogs made (0x4a1d70, 0x498b50, 0x4b3cf0, 0x497b20, 0x48d8b0, 0x498790), [0x7c24c8] 0, Focus_Restore, and the GUI manager's slot 3 with the byte 0x78d90e set, else slot 4.
void __cdecl Panel7b9940_Open();

// 0x4855d0 (thiscall (complete), pops 4): the constructor. complete (the most derived object) sets the vbtables +0 0x6fdd00 and +0xc
// 0x6fdcf8 and the vtable +8 0x6fdce8, then through +0xc's vbtable the vtable 0x6fdcd8 and its displacement (offset - 0x60) and
// +0x5c 0. Then through +0's vbtable (each offset read again for every use): the vtable 0x6fdcc8 at the first base and its
// displacement (offset - 8) before it, and in the second base: +0x50 2, the scene flags touched (SceneFlags_Touch on 0x8bd508),
// +0x2c and +0x30 0, +0x34 1.0f, touched again, +4..+0x10 1.0f and +0x54 -1. Returns the object.
u8 *__fastcall Panel7b9940_Ctor(u8 *self, void *edx, u32 complete);

// 0x48a290 (no arguments; also reached through the jump 0x4813f0, called with ECX 0x7b51f0 it ignores): the front end's panels closed:
// message 5 to the windows of the 0x7c26b0 records (Windows7c26b0_Message5), the object's virtual base taken out of the renderer's tag
// 0x20 list (as Panel7b9940_Detach, but before the GUI manager's slot 4), GuiManager_CallSlot4, the objects at 0x7ca1e0, 0x7cc2fc..0x7cc304
// and 0x7ca1bc released, Dialog7ca098_Message4IfUp, the one at 0x7ca1c8 released, message 4 (0, 0) to the windows [0x7cb2ac] and
// [0x7cb2b0], and the idle 0x7c26b0 records released (Table7c26b0_ReleaseIdle). The original ends in a jump to the empty 0x5da3c0.
void __cdecl Panel7b9940_Close();

// 0x48a310 (no arguments): the GUI's queues reset (Gui_ResetQueues, ui/window); in the object's second base (0x7b9940 + its vbtable's
// +8, read again after each touch): the scene flags touched (SceneFlags_Touch on 0x8bd508), +0x54 -1 and +4..+0x10 the float at
// 0x6ff148 (1.0f), touched again, +0x2c -0.3f, +0x30 -0.5f and +0x34 0.8f; the first base (the vbtable's +4) added to the renderer's
// ([0x87a6d0]) tag 0x20 list (Renderer_Add); then the GUI manager's slot 3 when the byte at 0x78d90e is set, else its slot 4
// (GuiManager_CallSlot3 / 4), whose result is returned.
u32 __cdecl Panel7b9940_Show();

// 0x48a6d0 (no arguments): the atexit entry of the screen size 0x48a580 remembers at 0x7b9d00 (a pair, registered when its guard bit
// 0x7c20e4 bit 0 is first set): nothing to destroy, a bare RET.
void __cdecl Panel7b9940_SizeAtExit();

}
