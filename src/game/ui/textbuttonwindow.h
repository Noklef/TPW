// The text button object's (Class492d80_Ctor's) window made. Reimplementation of 0x492e80; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: its own bench has them).
struct TextButtonWindowEnv {
    u8 *(__fastcall *ctor)(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);   // Window_Ctor
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);                                                          // Window_SetHandler
    u32(__cdecl *find)(const char *name);                                                                                  // GuiWindows_FindByName
    void(__fastcall *setObject98)(u8 *window, void *, u8 *object);                                                         // Window_SetObject98
    u32(__fastcall *setDword)(u8 *window, void *, u32 index, u32 value);                                                   // Window_SetDwordA0
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                                                              // Window_SetShown
};
void TextButtonWindow_SetEnv(const TextButtonWindowEnv *env);   // nullptr: the game's

extern "C" {

// 0x492e80 (thiscall (handler, second), pops 8; called through the panels' builders): unless the object has its window +0x10: a block of 0x138 (none: +0x10 = 0, the end) built as a root window (flags 1, the id
// rand * rand + rand, 0, 0, 0x7ff, 0x5ff) into +0x10; none: the end. It gets the handler, the object "lolight" as its object +0x98, the object as its dword 0; second, when not 0, as the object's dword 0; and it is hidden.
void __fastcall TextButton_CreateWindow(u8 *object, void *, u32 handler, u32 second);
}
