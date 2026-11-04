// The game's cursor files loaded: a .ani / .cur by the system, a .tga as a picture. Reimplementation of 0x4895b0 and 0x489670; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the ports by default; the bench's recorders: their own benches have them).
struct CursorFilesEnv {
    s32(__cdecl *sprintf)(char *to, const char *format, ...);             // 0x67a650
    char *(__cdecl *locate)(const char *name, u32 flags, s32 *index);     // Quickload_Locate
    void *(__cdecl *alloc)(u32 size);                                     // Mem_Alloc
    u8 *(__fastcall *systemCtor)(void *self);                             // SysCursor_Ctor
    u8 *(__fastcall *pictureCtor)(void *self);                            // 0x47eab0
    void(__fastcall *pictureLoad)(u8 *cursor, void *, const char *file);  // 0x47ec80
};
void CursorFiles_SetEnv(const CursorFilesEnv *env);

extern "C" {

// 0x4895b0 (cdecl (name)): the file named by the format 0x7488ac, the folder [0x74fc30] and the name, located (flags 1). Found: a system cursor of 0x18 bytes made; its handle from
// LoadCursorFromFileA, if any, given to its slot 1 (handle, 0). Returns the cursor, or 0.
u8 *__cdecl CursorFiles_LoadSystem(const char *name);

// 0x489670 (cdecl (name)): the same with the folder [0x74fc34]: a picture cursor of 0x68 bytes made and the file loaded into it.
u8 *__cdecl CursorFiles_LoadPicture(const char *name);
}
