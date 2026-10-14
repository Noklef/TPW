// The in-memory stream source: see memsource.h.
#include "memsource.h"

#include <cstring>

namespace {

void *const *const kMemorySourceVtable = reinterpret_cast<void *const *>(0x00703868);

// REP MOVSD then REP MOVSB, forward.
void copy(u8 *dst, const u8 *src, u32 n) {
    for (u32 i = 0; i < n / 4; ++i, dst += 4, src += 4) {
        u32 v;
        std::memcpy(&v, src, 4);
        std::memcpy(dst, &v, 4);
    }
    for (u32 i = 0; i < n % 4; ++i) *dst++ = *src++;
}

// After a chunk is used up: the next one, or none.
void next_chunk(MemorySource *m) {
    m->chunk = m->chunk->next;
    m->cursor = m->chunk ? m->chunk->data : nullptr;
}

}  // namespace

extern "C" {

MemorySource *__fastcall MemorySource_Ctor(MemorySource *m, void *, u32 chunkSize) {
    RefObject_Ctor(&m->base);
    Semaphore_Ctor(&m->lock);
    m->chunkSize = chunkSize;
    m->head = nullptr;
    m->tail = nullptr;
    m->chunk = nullptr;
    m->cursor = nullptr;
    m->position = 0;
    m->size = 0;
    m->base.vtable = kMemorySourceVtable;
    return m;
}

void __fastcall MemorySource_Dtor(MemorySource *m) {
    Semaphore_Dtor(&m->lock);
    RefObject_Dtor(&m->base);
}

MemorySource *__fastcall MemorySource_DeletingDtor(MemorySource *m, void *, u32 flags) {
    MemorySource_Dtor(m);
    if (flags & 1) Handles_GetEnv()->free(m);
    return m;
}

s32 __fastcall MemorySource_Read(MemorySource *m, void *, void *buffer, s32 count) {
    u8 *dst = static_cast<u8 *>(buffer);
    s32 read = 0;
    while (count) {
        const MemoryChunk *const c = m->chunk;
        const s32 avail = c ? static_cast<s32>((c == m->tail ? m->size : c->end) - m->position) : 0;
        if (!avail) break;
        if (avail <= count) {
            copy(dst, m->cursor, static_cast<u32>(avail));
            dst += avail;
            read += avail;
            count -= avail;
            m->position += avail;
            next_chunk(m);
        } else {
            copy(dst, m->cursor, static_cast<u32>(count));
            read += count;
            m->position += count;
            m->cursor += count;
            count = 0;
        }
    }
    return read;
}

s32 __fastcall MemorySource_Write(MemorySource *m, void *, const void *buffer, u32 count) {
    const u8 *src = static_cast<const u8 *>(buffer);
    s32 written = 0;
    while (count) {
        if (!m->chunk) {
            s32 r;
            if (*MemorySource_AddChunk(m, nullptr, &r) < 0) break;
            m->chunk = m->tail;
            m->cursor = m->tail->data;
        }
        const u32 avail = m->chunkSize + static_cast<u32>(m->chunk->data - m->cursor);
        if (avail <= count) {
            copy(m->cursor, src, avail);
            count -= avail;
            src += avail;
            written += avail;
            m->position += avail;
            next_chunk(m);
        } else {
            copy(m->cursor, src, count);
            written += count;
            m->position += count;
            m->cursor += count;
            count = 0;
        }
        if (m->position > m->size) m->size = m->position;
    }
    return written;
}

s32 __fastcall MemorySource_Seek(MemorySource *m, void *, s32 offset, u32 origin) {
    s32 position;
    switch (origin) {
    case 0: position = offset; break;
    case 1: position = static_cast<s32>(m->position) + offset; break;
    case 2: position = static_cast<s32>(m->size) + offset; break;
    default: return static_cast<s32>(m->position);
    }
    if (position > static_cast<s32>(m->size)) position = static_cast<s32>(m->size);
    if (position < 0) position = 0;
    m->position = static_cast<u32>(position);
    m->chunk = nullptr;
    m->cursor = nullptr;
    for (MemoryChunk *c = m->head; c && !m->chunk; c = c->next) {
        if (c->end > m->position) {
            m->chunk = c;
            m->cursor = c->data - c->start + m->position;
        }
    }
    return position;
}

u32 __fastcall MemorySource_Size(MemorySource *m) { return m->size; }

u32 __fastcall MemorySource_Tell(MemorySource *m) { return m->position; }

s32 *__fastcall MemorySource_AddChunk(MemorySource *m, void *, s32 *result) {
    MemoryChunk *const c = static_cast<MemoryChunk *>(Handles_GetEnv()->alloc(m->chunkSize + sizeof(MemoryChunk)));
    if (!c) {
        *result = -1;
        return result;
    }
    c->prev = nullptr;
    c->next = nullptr;
    c->start = 0;
    c->end = 0;
    c->data = reinterpret_cast<u8 *>(c + 1);
    if (!m->head) {
        m->head = c;
        m->tail = c;
        c->start = 0;
    } else {
        m->tail->next = c;
        c->prev = m->tail;
        m->tail = c;
        c->start = c->prev->end;
    }
    c->end = c->start + m->chunkSize;
    *result = 0;
    return result;
}

void __fastcall MemorySource_Clear(MemorySource *m) {
    for (MemoryChunk *c = m->head; c;) {
        MemoryChunk *const next = c->next;
        Handles_GetEnv()->free(c);
        c = next;
    }
    m->head = nullptr;
    m->tail = nullptr;
    m->chunk = nullptr;
    m->cursor = nullptr;
    m->position = 0;
    m->size = 0;
}

void __fastcall MemorySource_Teardown(MemorySource *m) { MemorySource_Clear(m); }

Semaphore *__fastcall MemorySource_GetLock(MemorySource *m) { return &m->lock; }

}  // extern "C"
