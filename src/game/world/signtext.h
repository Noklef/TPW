// A sign's two lines of text (names invented, unconfirmed): +0x3c8 and +0x40a, wide strings of up to 0x21 characters; the owner's new text goes to the save, every change to the network (messages 0x50 and 0x55).
// Reimplementations of 0x4e2780, 0x4e2950, 0x4e2a50.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct SignTextEnv {
    u16 *(__cdecl *copy)(u16 *to, const u16 *from, u32 count);          // 0x67b0d0
    u8 *(__fastcall *readMode)(u8 *setting);                            // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *f)());                           // 0x67a7d0
    u16 *(__fastcall *handle)(const u8 *thing, void *, u16 *out);       // Thing_GetHandle
    void(__fastcall *saveLineA)(u8 *save, void *, const u16 *text);     // GlobalSave_SetSignNameA
    void(__fastcall *saveLineB)(u8 *save, void *, const u16 *text);     // GlobalSave_SetSignNameB
    u8 *(__cdecl *save)();                                              // GlobalSave_Instance
    void(__fastcall *send)(u8 *net, void *, u32 message, u32 cell, u32 length, const u16 *text);  // 0x403fd0
    s32(__cdecl *format)(u16 *to, u32 count, u32 format, const u16 *a);               // CRT_snwprintf
    s32(__cdecl *format2)(u16 *to, u32 count, u32 format, const u16 *a, const u16 *b);  // CRT_snwprintf
};
void SignText_SetEnv(const SignTextEnv *env);

extern "C" {

// 0x4e2780 (thiscall; pops 8): the text in one string. No first line: the second copied. First line ending '-': without the '-' (at most 0x20 characters, in the buffer 0x7cdc10) and the second by the format 0x749850. No second line: the first copied. Otherwise the format 0x75c0c0.
void __fastcall SignText_Format(const u8 *sign, void *, u16 *to, u32 count);

// 0x4e2950 (thiscall; pops 4): the first line set. Unless the mode setting 0xfb3b7c is 1: the player's own [[0x80239c] + 0x1da732] sign's goes to the save. Message 0x50 to the network 0x785a08, with cell 0xea60 for the player's own, otherwise the sign's cell.
void __fastcall SignText_SetFirst(u8 *sign, void *, const u16 *text);
// 0x4e2a50 (thiscall; pops 4): the second line; message 0x55.
void __fastcall SignText_SetSecond(u8 *sign, void *, const u16 *text);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u16[6], gText_Signtext_75c0c0, 0x0075c0c0, 0xc);
GAME_DATA(u16[4], gText_Signtext_75c0cc, 0x0075c0cc, 0x8);
