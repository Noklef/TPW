// The dialog [0x7cc184] (script 0x7540d8, handler 0x4a7d70) and its three edit fields. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a8c00 (cdecl; name invented, unconfirmed): nothing when [0x7cc184] is set. The session (Online_Instance inlined) restricted: CommandQueue_Post (0x4e, 0) and no more. The setting [0xfb3b7c] (guard 0xfb34bf bit 0,
// Setting_ReadMode, atexit 0x5d1750), read each time, at 1: the parent [0x7cc8c0]; else Window7c24c8_Close and the parent [0x7cb2ac]. The dialog built; none: only the end below. Not at 1: Window7c24c8_Set (dialog). The hover label
// 0x5d9e has the text 0xd0. The edits (each only if found; a new TextPart6ff4d0, none on a failed allocation; font 0xb, white, Window_Notify3000 (a, 0), Window_NotifyC00 (0, 0), the caret of the name 0x74fa84, the text in slot 9):
// 0x5da5 the buffer 0x7cb4d0 of 0x1ff, a = 1, text 0xd3 (its part made before the buffer); 0x5da4 the buffer 0x7cb408 of 0x5f, a = 0, text 0xd1; 0x5da3 the buffer 0x7cbf48 of 0xff, a = 1, text 0xd2. Each buffer zeroed first. The picture
// 0x5d9f (if found): the handler 0x4a7c50, a new RendererPart; its surface (Window_ImplSlot7) with a surface and [0x7cc18c] set: the panel reset, shrunk into it (format 2 at [0x8bd554] = 2, format 1 at 3, else not), Window_ImplSlot6.
// The end: the bytes [0x7cbf40] and [0x7cbf3c] 0; with the setting not at 1 Advisor_PostId (0x133).
void __cdecl Dialog7cc184_Open();
}

// Dialog7cc184_Open's calls (the ports by default; the bench's recorders: their own benches have them). Window_FindChild runs. Two unobserved SurfaceInfo_Ctor of stack objects left out.
struct Dialog7cc184OpenEnv {
    u8 *(__fastcall *readMode)(u8 *object);                                  // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *stub)());                             // crt atexit 0x67a7d0
    void *(__cdecl *alloc)(u32 size);                                        // GameHeap_Alloc
    u8 *(__fastcall *holderCtor)(u8 *memory, void *);                        // 0x5b50b0
    u32(__fastcall *restricted)(const u8 *session);                          // Online_Restricted
    u32(__cdecl *post)(u32 code, u32 param);                                 // CommandQueue_Post
    u32(__cdecl *closeModal)();                                              // Window7c24c8_Close
    u32(__cdecl *setModal)(u8 *window);                                      // Window7c24c8_Set
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);    // WindowScript_Build
    u16 *(__cdecl *text)(u32 index);                                         // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                     // HoverLabel_Attach
    u8 *(__fastcall *editPart)(u8 *part, void *, u32 value);                 // TextPart6ff4d0_Ctor
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);    // Window705a20_SetBuffer
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                 // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                          // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                 // Window_Set4c
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);          // Window_Notify3000
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);           // Window_NotifyC00
    u32(__cdecl *findByName)(const char *name);                              // GuiWindows_FindByName
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);               // Window705a20_SetCaret
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);            // Window_SetHandler
    u8 *(__fastcall *picturePart)(u8 *part, void *, u32 value);              // RendererPart_Ctor
    u32(__fastcall *slot7)(u8 *window, void *, u32 a, u32 b);                // Window_ImplSlot7
    void(__fastcall *reset)(u8 *panel);                                      // Panel_Reset
    u8(__fastcall *shrink)(u8 *panel, void *, u8 *dest, s32 width, s32 height, u32 format);   // Panel_ShrinkInto
    u32(__fastcall *slot6)(u8 *window);                                      // Window_ImplSlot6
    u8(__cdecl *advisor)(u32 id);                                            // Advisor_PostId
};
void Dialog7cc184_SetOpenEnv(const Dialog7cc184OpenEnv *env);
