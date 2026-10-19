// The save game's field announcer (reimplementation of 0x416e10) and field readers. Every module's serializer announces each field it saves or
// loads (its size and name) before the stream call; the shipped build compiled the announcer to an empty debug stub.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x416e10 (stdcall; size, name, isLoad): 0, nothing else.
u32 __stdcall Save_FieldName(u32 size, const char *name, u32 isLoad);

// 0x424c20 / 0x4d37e0 / 0x4fae10 / 0x510240 / 0x5134d0 (cdecl; stream, destination, name, quiet): read a field of 1, 2, 4, 8 or 12 bytes
// from the stream (Stream_Read), announced (Save_FieldName, loading) unless quiet; 0 when the whole field came, else 1.
u32 __cdecl Save_ReadU8(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_ReadU16(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_ReadU32(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_Read8Bytes(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_Read12Bytes(Stream *stream, void *dst, const char *name, u32 quiet);

// 0x4cee50 / 0x4ceec0 / 0x4ced70 (cdecl; stream, source, name, quiet; names unconfirmed): the writers to match: a field of 1, 2
// or 4 bytes written to the stream (Stream_Write), announced (Save_FieldName, saving) unless quiet; 0 when the whole field went, else 1
// (reported to the empty Debug_Printf).
u32 __cdecl Save_WriteU8(Stream *stream, const void *src, const char *name, u32 quiet);
u32 __cdecl Save_WriteU16(Stream *stream, const void *src, const char *name, u32 quiet);
u32 __cdecl Save_WriteU32(Stream *stream, const void *src, const char *name, u32 quiet);

// 0x416e20 / 0x416e40 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x78aa2c = 1 and 0x78b11c = 1 / 2^23 (as
// camera's).
void __cdecl Save_InitUnusedOne();
void __cdecl Save_InitUnusedEpsilon();
}

// A serializer's field (a helper, not a function of the game): announced (Save_FieldName; loading unless save) with the name's address, then
// written (Stream_Write) or read (Stream_Read) whole; true when the stream's result is not negative and it moved size bytes. The serializers
// (thiscall; stream, save, version, unused) do nothing for versions other than 0 and 1 and return 0; else their fields in turn, 1 at the first that
// fails (the print is left out), else 0.
bool Save_Field(Stream *stream, u32 save, void *at, s32 size, u32 name);

// A serializer's run of fields (a helper): each field of the table (offset in the object, size, name's address) in turn through Save_Field;
// false at the first that fails. The serializers whose load path calls the readers (0x424c20 ...) announce and read the same way.
struct SaveSpec {
    u16 at, size;
    u32 name;
};
bool Save_Fields(Stream *stream, u32 save, u8 *object, const SaveSpec *spec, u32 count);
