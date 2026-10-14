// The buffered source: see bufsource.h.
#include "bufsource.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const BufferedSourceEnv kGameEnv = {
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
};
const BufferedSourceEnv *g_env = &kGameEnv;

void *const *const kBufferedSourceVtable = at<void *const *>(0x00703f78);

}  // namespace

void BufferedSource_SetEnv(const BufferedSourceEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

BufferedSource *__fastcall BufferedSource_Ctor(BufferedSource *s) {
    RefObject_Ctor(&s->base);
    s->base.vtable = kBufferedSourceVtable;
    s->buffer = nullptr;
    s->window = nullptr;
    s->capacity = 0;
    s->position = 0;
    s->start = 0;
    s->sourcePos = 0;
    return s;
}

s32 *__fastcall BufferedSource_Open(BufferedSource *s, void *, s32 *result, u32 capacity) {
    s->capacity = capacity;
    s->buffer = static_cast<u8 *>(g_env->alloc(capacity));
    if (!s->buffer) {
        *result = -1;
        return result;
    }
    s->window = s->buffer;
    s->sourcePos = 0;
    s->position = 0;
    s->start = 0;
    s->end = 0;
    *result = 0;
    return result;
}

void __fastcall BufferedSource_Free(BufferedSource *s) { g_env->free(s->buffer); }

u32 __fastcall BufferedSource_Read(BufferedSource *s, void *, void *buffer, u32 count) {
    if (!s->base.link) return 0;
    if ((s->position < s->start || s->position >= s->end) && count < s->capacity) BufferedSource_Fill(s);
    const u32 copied = BufferedSource_CopyWindow(s, nullptr, buffer, count);
    const u32 rest = count - copied;
    if (!rest) return copied;
    return BufferedSource_ReadDirect(s, nullptr, static_cast<u8 *>(buffer) + copied, rest) + copied;
}

s32 __fastcall BufferedSource_Seek(BufferedSource *s, void *, s32 offset, u32 origin) {
    if (!s->base.link) return -1;
    const u32 size = vcall<u32>(&s->base, 0x10);
    switch (origin) {
        case 0: s->position = static_cast<u32>(offset); break;
        case 1: s->position += static_cast<u32>(offset); break;
        case 2: s->position = size + static_cast<u32>(offset); break;
        default: break;
    }
    if (s->position > size) {
        s->position = size;
        return -1;
    }
    return static_cast<s32>(s->position);
}

u32 __fastcall BufferedSource_Write(BufferedSource *s, void *, const void *buffer, u32 count) {
    if (s->position != s->sourcePos) {
        s->sourcePos = vcall<u32>(s->base.link, 0xc, static_cast<s32>(s->position - s->sourcePos), 1u);
        if (s->sourcePos != s->position) return 0;
    }
    const u32 written = vcall<u32>(s->base.link, 0x8, buffer, count);
    s->sourcePos += written;
    const u32 from = s->position;
    const u32 to = s->sourcePos;
    if (from >= s->start && from < s->end) {
        s->end = from;
    } else if (to >= s->start && to < s->end) {
        const u32 moved = to - s->start;
        s->start += moved;
        s->window += moved;
    } else if (to >= s->end && from < s->start) {
        s->start = s->end;
    }
    s->position = to;
    return written;
}

s32 __fastcall BufferedSource_Tell(const BufferedSource *s) {
    return s->base.link ? static_cast<s32>(s->position) : -1;
}

u32 __fastcall BufferedSource_Attach(BufferedSource *s, void *, RefObject *source) {
    RefObject_SetLink(&s->base, nullptr, source);
    const u32 tell = vcall<u32>(s->base.link, 0x14);
    s->sourcePos = tell;
    s->position = tell;
    return tell;
}

u32 __fastcall BufferedSource_ReadDirect(BufferedSource *s, void *, void *buffer, u32 count) {
    const u32 read = BufferedSource_ReadRaw(s, nullptr, buffer, count);
    s->position += read;
    return read;
}

u32 __fastcall BufferedSource_CopyWindow(BufferedSource *s, void *, void *buffer, u32 count) {
    if (s->position >= s->end || s->position < s->start) return 0;
    const u32 left = s->end - s->position;
    const u32 n = count < left ? count : left;
    std::memcpy(buffer, s->window - s->start + s->position, n);
    s->position += n;
    return n;
}

u32 __fastcall BufferedSource_Fill(BufferedSource *s) {
    const u32 read = BufferedSource_ReadRaw(s, nullptr, s->buffer, s->capacity);
    s->window = s->buffer;
    s->end = s->sourcePos;
    s->start = s->sourcePos - read;
    return read;
}

u32 __fastcall BufferedSource_ReadRaw(BufferedSource *s, void *, void *buffer, u32 count) {
    if (s->position != s->sourcePos) {
        s->sourcePos = vcall<u32>(s->base.link, 0xc, static_cast<s32>(s->position - s->sourcePos), 1u);
        if (s->sourcePos != s->position) return 0;
    }
    const u32 read = vcall<u32>(s->base.link, 0x4, buffer, count);
    s->sourcePos += read;
    return read;
}

}  // extern "C"
