// The view controls dialog's handler: its buttons work the view objects. Reimplementation of 0x4c53f0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct ViewButtonsEnv {
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
    void(__fastcall *useSecond)(u8 *self);                                         // ViewCtl_UseSecondVector
    void(__fastcall *useFirst)(u8 *self);                                          // ViewCtl_UseFirstVector
    void(__fastcall *lower48)(u8 *self);                                           // ViewCtl_Lower48
    void(__fastcall *raise48)(u8 *self);                                           // ViewCtl_Raise48
    void(__fastcall *raise50)(u8 *self);                                           // ViewCtl_Raise50
    void(__fastcall *lower50)(u8 *self);                                           // ViewCtl_Lower50
    u8 *(__cdecl *online)();                                                       // Online_Instance
    u32(__fastcall *hasActivity)(const u8 *object);                                // Obj_HasActivity
    void(__cdecl *openChat)(u32 value);                                            // Dialog7cc944_Open
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);                 // Class702da0_Push
};
void ViewButtons_SetEnv(const ViewButtonsEnv *env);

extern "C" {

// 0x4c53f0 (cdecl (window, message, a, b), the handler 0x4c4fb0 gives the dialog [0x7cdb0c]). The window's dword +0xa0 0 picks the view object: set the game's +0x10, else
// its +0xc. Message 0x14: [0x7cdb0c] 0. Message 0x100: button 0x16000 calls the game's +0xc's slot 8 and +4's slot 7, pushes the scene 0x258 (1) and answers 0 without the
// default; 0x16003, unless the online session is active or its objects +0xe0, +0xf8, +0xfc are without their +0x28, calls +0xc's slot 0x12 and opens the chat (0x4c4fa0);
// 0x16006 sets the object's vector mode (second when b is 1). Messages 0x10008 and 0x10009: the buttons 0x16001, 0x16002, 0x16004, 0x16005 move the object (Lower50,
// Lower48, Raise48, Raise50). All else, and after the above, Window_CallDefault.
s32 __cdecl ViewButtons_Handler(u8 *window, u32 message, u32 a, u32 b);
}
