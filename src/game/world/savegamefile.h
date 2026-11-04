// A saved game's file: its magic and header, then the game's stream (world/gamefile) packed. Reimplementation of 0x414920, 0x414d40, 0x415140, 0x415f50 and 0x416240; SaveGame_Load's name comes from its caller's comment, the others are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"
#include "../files/file.h"

// Their calls by address (the bench's recorders).
void SaveGame_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x414920 (thiscall, pops 0xc): the file opened for writing, the magic at 0x6fd928 and the header (0x415f50 (the stream, header)) written and it closed; the clock's now; the game (0x4164c0 (a memory stream, mode)) written to memory, read back and
// packed (Buffer_Compress) and appended to the file. 1 when all went so; else 0, the file deleted for a failure before the appending, and +4 the cause: 1 no file, 2 no memory, 3 a bad write, 5 the game not written. The unpacked block leaks when the packed one can not be had (sic).
u32 __fastcall SaveGame_Save(u8 *self, void *, const char *name, u32 header, u32 mode);

// 0x414d40 (thiscall, pops 0xc): the file opened, its magic read (mode 2: no more than 0x1f4) and its header (0x416240 (the file, mode, header)). Mode 0 stops there with 1. The rest unpacked into a memory stream; unless mode 1, the game's parts cleared first, and the game read (GameFile_Load),
// and then 0x415140. 1 when that read; else 0 and +4 the cause: 1 no file or no magic, 2 no memory, 3 a bad write to memory, 4 a short read, 6 nothing to read or the game not read, 7 no header, 9 a newer file.
u32 __fastcall SaveGame_Load(u8 *self, void *, const char *name, u32 header, u32 mode);

// 0x415f50 (thiscall, this unused, pops 8): the header: the language's number (a byte; its name from Language_Buffer, 0 when not known), its name in wide letters (0x500 bytes, the table at 0x78a460), the 0x20 bytes at 0x802080 (0x100 bytes),
// the dword at 0x749870 in network order (the import at 0x6fd3fc), whether there is a park, and the park (ParkInfo_Serialize (the stream, 1, 1, 0)). 1 when all was written and the park did not answer 1, else 0.
u32 __fastcall SaveGame_WriteHeader(void *, void *, Stream *stream, u8 *park);

// 0x416240 (thiscall, pops 0xc): the same read: a language whose name differs fails; World_HasRides (the 0x100 bytes, mode 2) not 0 or +4 8, and their 0x20 bytes kept at 0x802080; a version not the dword at 0x749870 (the import at 0x6fd400): +4 6. A park in the file is read into
// park or, when none is given, a local one; a park given when the file has none is left alone. 1 when all was read and the park answered 0, else 0.
u32 __fastcall SaveGame_ReadHeader(u8 *self, void *, Stream *stream, u32 mode, u8 *park);

// 0x415140 (thiscall, this unused): after a loaded game: the parts told, each font of the list at 0x789c1c removed and its file deleted, the list emptied (each one's +0xc (0)), the interface told.
void __fastcall SaveGame_AfterLoad(void *);
}
