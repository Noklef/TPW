// The narrow file stream buffer (std::basic_filebuf<char> shape; names invented, unconfirmed): +0x20 a pointer to the put position, +0x30 a pointer to the room left, +0x38 the converter (0: none),
// +0x40 the conversion state, +0x44 a std::string kept for the converter's output, +0x50 the FILE.
#pragma once

#include "../../../re/types.h"

// The C library's file calls (each the game's, A's range).
struct FileBufEnv {
    s32(__cdecl *fputc)(s32 c, void *file);                                   // 0x67e570
    u32(__cdecl *fwrite)(const void *data, u32 size, u32 count, void *file);  // CRT_fwrite
    s32(__cdecl *fsetpos)(void *file, const void *position);                  // 0x67cf70
    s32(__cdecl *fseek)(void *file, s32 offset, s32 origin);                  // CRT_fseek
    s32(__cdecl *fgetpos)(void *file, void *position);                        // 0x67cf40
};
void FileBuf_SetEnv(const FileBufEnv *env);  // nullptr: the game's

extern "C" {

// 0x5cedd0 (thiscall (c), pops 4; name invented, unconfirmed): -1 answers 0. Room in the put area: c stored. No file: -1. No converter: fputc of c sign-extended. Else the string has its first
// [0x6fe5ac] characters (all, as a rule) erased and 8 zeros added, the converter's slot 5 (+0x14) turns c into it; 0 (or 1, unless no input was taken) writes what it made, by fwrite(data, n, 1, file) compared with
// n (sic: only n = 1 can pass); 3 is fputc of c. Answers c, or -1.
s32 __fastcall FileBuf_Overflow(u8 *self, void *, s32 c);

// 0x5cf490 (thiscall (out, offset, 1 dword, position's two dwords, state, 2 dwords), pops 0x20; name invented, unconfirmed): fsetpos to the position, fseek by a non-zero offset from the current, fgetpos back. The string,
// when there is one: +0x40 = state, its shared block made its own, its first [0x6fe5ac] characters erased. out = {0, (not written), position, +0x40}; any failure, or no file: Obj_Init4f1520(out, -1). Returns out.
u32 *__fastcall FileBuf_SeekPos(u8 *self, void *, u32 *out, s32 offset, u32 unused2, u32 low, u32 high, u32 state, u32 unused6, u32 unused7);
}
