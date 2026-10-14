// The fixed-buffer text writer (vtable 0x703e08): reimplementation of 0x636b60-0x636e00.
//
// Writes text into a caller's buffer of `capacity` bytes, keeping it NUL-terminated: a length
// that does not fit is cut to capacity - 1 and sets the overflow flag. RefObject2's path
// writer (vtable +0x14) uses one on the stack. Every method sizes through SetLength (vtable
// +4), then copies with the game's strncpy/strncat wrappers.
//
// The originals' quirks are kept: clearing a zero-capacity buffer writes its NUL at
// buffer[-1] (length -1, overflow set); Format stores _vsnprintf's result as the length
// as is (-1 when it did not fit, the buffer then unterminated) and leaves the overflow flag.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"

struct TextWriter {
    void *const *vtable;
    u32 length;
    char *data;
    u32 capacity;
    u32 overflow;
};
static_assert(sizeof(TextWriter) == 0x14, "TextWriter is 0x14 bytes");

extern "C" {
// 0x636e00, cdecl (writer, format, ...; vtable +0): _vsnprintf into the buffer; no format:
// SetLength(0).
void __cdecl TextWriter_Format(TextWriter *w, const char *format, ...);
// __thiscall. 0x636b80 (length; +4): the length (cut to capacity - 1, flagged), NUL there.
void __fastcall TextWriter_SetLength(TextWriter *w, void *edx, u32 length);
// 0x636b60 (flags; +8) / 0x636c70: destructors (only the vtable).
TextWriter *__fastcall TextWriter_DeletingDtor(TextWriter *w, void *edx, u32 flags);
void __fastcall TextWriter_Dtor(TextWriter *w);
// 0x636bc0 (buffer, capacity, clear): over buffer, emptied when clear, else holding its text.
TextWriter *__fastcall TextWriter_Ctor(TextWriter *w, void *edx, char *buffer, u32 capacity, u32 clear);
// 0x636c80 (+0xc) / 0x636cc0 (+0x10) / 0x636d10 (+0x14): assign a char (by pointer; its NUL
// empties), a C string (null empties) or a String.
TextWriter *__fastcall TextWriter_AssignChar(TextWriter *w, void *edx, const char *c);
TextWriter *__fastcall TextWriter_AssignStr(TextWriter *w, void *edx, const char *text);
TextWriter *__fastcall TextWriter_Assign(TextWriter *w, void *edx, const String *s);
// 0x636d40 (+0x18) / 0x636d80 (+0x1c) / 0x636dc0 (+0x20): append a C string (null: nothing),
// a char (its NUL: nothing) or a String.
TextWriter *__fastcall TextWriter_AppendStr(TextWriter *w, void *edx, const char *text);
TextWriter *__fastcall TextWriter_AppendChar(TextWriter *w, void *edx, const char *c);
TextWriter *__fastcall TextWriter_Append(TextWriter *w, void *edx, const String *s);
}
