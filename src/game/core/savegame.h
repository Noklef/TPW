// The save game's field announcer (reimplementation of 0x416e10) and field readers. Every module's serializer announces each field it saves or
// loads (its size and name) before the stream call; the shipped build compiled the announcer to an empty debug stub.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x416e10 (stdcall; size, name, isLoad): 0, nothing else.
u32 __stdcall Save_FieldName(u32 size, const char *name, u32 isLoad);

// 0x4d37e0 / 0x4fae10 / 0x510240 / 0x5134d0 (cdecl; stream, destination, name, quiet): read a field of 2, 4, 8 or 12 bytes from the stream
// (Stream_Read), announced (Save_FieldName, loading) unless quiet; 0 when the whole field came, else 1.
u32 __cdecl Save_ReadU16(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_ReadU32(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_Read8Bytes(Stream *stream, void *dst, const char *name, u32 quiet);
u32 __cdecl Save_Read12Bytes(Stream *stream, void *dst, const char *name, u32 quiet);

// 0x416e20 / 0x416e40 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x78aa2c = 1 and 0x78b11c = 1 / 2^23 (as
// camera's).
void __cdecl Save_InitUnusedOne();
void __cdecl Save_InitUnusedEpsilon();
}
