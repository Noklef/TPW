// The fixed-buffer text writer: see textwriter.h.
#include "textwriter.h"

#include <cstdarg>
#include <cstdint>

#include "../core/libc.h"
#include "../core/vcall.h"
#include "handles.h"

namespace {

void *const *const kTextWriterVtable = reinterpret_cast<void *const *>(0x00703e08);

void set_length(TextWriter *w, u32 length) { vcall<void>(w, 4, length); }

// SetLength's body (the constructor has it inline).
void clamp(TextWriter *w, u32 length) {
    if (length < w->capacity) {
        w->length = length;
        w->overflow = 0;
        w->data[length] = 0;
        return;
    }
    w->overflow = 1;
    w->length = w->capacity - 1;
    w->data[static_cast<s32>(w->length)] = 0;
}

}  // namespace

extern "C" {

void __cdecl TextWriter_Format(TextWriter *w, const char *format, ...) {
    if (!format) {
        set_length(w, 0);
        return;
    }
    va_list args;
    va_start(args, format);
    w->length = static_cast<u32>(Lib_VSNPrintf(w->data, w->capacity, format, args));
    va_end(args);
}

void __fastcall TextWriter_SetLength(TextWriter *w, void *, u32 length) { clamp(w, length); }

TextWriter *__fastcall TextWriter_DeletingDtor(TextWriter *w, void *, u32 flags) {
    w->vtable = kTextWriterVtable;
    if (flags & 1) Handles_GetEnv()->free(w);
    return w;
}

void __fastcall TextWriter_Dtor(TextWriter *w) { w->vtable = kTextWriterVtable; }

TextWriter *__fastcall TextWriter_Ctor(TextWriter *w, void *, char *buffer, u32 capacity, u32 clear) {
    w->data = nullptr;
    w->length = 0;
    w->vtable = kTextWriterVtable;
    w->capacity = capacity;
    w->data = buffer;
    if (clear) {
        if (capacity) {
            w->length = 0;
            w->overflow = 0;
            buffer[0] = 0;
        } else {
            w->overflow = 1;
            w->length = capacity - 1;
            buffer[-1] = 0;
        }
        return w;
    }
    clamp(w, Lib_StrLen(buffer));
    return w;
}

TextWriter *__fastcall TextWriter_AssignChar(TextWriter *w, void *, const char *c) {
    if (!*c) {
        set_length(w, 0);
        return w;
    }
    set_length(w, 1);
    if (!w->overflow) w->data[0] = *c;
    return w;
}

TextWriter *__fastcall TextWriter_AssignStr(TextWriter *w, void *, const char *text) {
    if (!text) {
        set_length(w, 0);
        return w;
    }
    set_length(w, Lib_StrLen(text));
    Lib_StrNCpy(w->data, text, w->length);
    return w;
}

TextWriter *__fastcall TextWriter_Assign(TextWriter *w, void *, const String *s) {
    set_length(w, s->length);
    Lib_StrNCpy(w->data, s->data, w->length);
    return w;
}

TextWriter *__fastcall TextWriter_AppendStr(TextWriter *w, void *, const char *text) {
    if (text) {
        const u32 old = w->length;
        set_length(w, Lib_StrLen(text) + w->length);
        Lib_StrNCat(w->data, text, w->length - old);
    }
    return w;
}

TextWriter *__fastcall TextWriter_AppendChar(TextWriter *w, void *, const char *c) {
    if (*c) {
        set_length(w, w->length + 1);
        if (!w->overflow) w->data[w->length - 1] = *c;
    }
    return w;
}

TextWriter *__fastcall TextWriter_Append(TextWriter *w, void *, const String *s) {
    const u32 old = w->length;
    set_length(w, s->length + old);
    Lib_StrNCat(w->data, s->data, w->length - old);
    return w;
}

}  // extern "C"
