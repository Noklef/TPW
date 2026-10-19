#include "savegame.h"

extern "C" {

u32 __stdcall Save_FieldName(u32, const char *, u32) { return 0; }

// The readers' shared body: announce the field unless quiet, read it; 0 when the whole field came, else 1 (the print is left out).
static u32 read_field(Stream *stream, void *dst, const char *name, u32 quiet, s32 size) {
    if (quiet == 0)
        Save_FieldName((u32)size, name, 1);
    s32 result, done = 0;
    if (*Stream_Read(&result, stream, dst, size, &done) < 0 || done != size)  // sic: the result is 0 or 1, never negative
        return 1;
    return 0;
}

u32 __cdecl Save_ReadU8(Stream *stream, void *dst, const char *name, u32 quiet) { return read_field(stream, dst, name, quiet, 1); }

u32 __cdecl Save_ReadU16(Stream *stream, void *dst, const char *name, u32 quiet) { return read_field(stream, dst, name, quiet, 2); }

u32 __cdecl Save_ReadU32(Stream *stream, void *dst, const char *name, u32 quiet) { return read_field(stream, dst, name, quiet, 4); }

u32 __cdecl Save_Read8Bytes(Stream *stream, void *dst, const char *name, u32 quiet) { return read_field(stream, dst, name, quiet, 8); }

u32 __cdecl Save_Read12Bytes(Stream *stream, void *dst, const char *name, u32 quiet) { return read_field(stream, dst, name, quiet, 12); }

// The writers' shared body: announce the field unless quiet, write it; 0 when the whole field went, else 1 (after the empty print of 0x747120).
static u32 write_field(Stream *stream, const void *src, const char *name, u32 quiet, s32 size) {
    if (quiet == 0)
        Save_FieldName((u32)size, name, 0);
    s32 result, done = 0;
    if (*Stream_Write(&result, stream, src, size, &done) < 0 || done != size)
        return 1;
    return 0;
}

u32 __cdecl Save_WriteU8(Stream *stream, const void *src, const char *name, u32 quiet) { return write_field(stream, src, name, quiet, 1); }

u32 __cdecl Save_WriteU16(Stream *stream, const void *src, const char *name, u32 quiet) { return write_field(stream, src, name, quiet, 2); }

u32 __cdecl Save_WriteU32(Stream *stream, const void *src, const char *name, u32 quiet) { return write_field(stream, src, name, quiet, 4); }

void __cdecl Save_InitUnusedOne() { *reinterpret_cast<float *>(0x0078aa2c) = 1.0f; }

void __cdecl Save_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x0078b11c) = 1.0f / 8388608.0f; }
}

bool Save_Field(Stream *stream, u32 save, void *at, s32 size, u32 name) {
    Save_FieldName((u32)size, reinterpret_cast<const char *>(name), save != 0 ? 0 : 1);
    s32 result, done = 0;
    return *(save != 0 ? Stream_Write(&result, stream, at, size, &done) : Stream_Read(&result, stream, at, size, &done)) >= 0 && done == size;
}

bool Save_Fields(Stream *stream, u32 save, u8 *object, const SaveSpec *spec, u32 count) {
    for (u32 i = 0; i < count; ++i)
        if (!Save_Field(stream, save, object + spec[i].at, spec[i].size, spec[i].name))
            return false;
    return true;
}
