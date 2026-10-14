// The sources that wrap another source, and the compressor singleton: see wrapsources.h.
#include "wrapsources.h"

#include <cstdint>
#include <cstring>

#include "../core/semaphore.h"
#include "../core/vcall.h"
#include "refpack.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const WrapSourcesEnv kGameEnv = {
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
    at<void(__cdecl *)()>(0x005f80c0),
    at<u8 *>(0x00fa75f8),
    at<ZlibCodec *>(0x00fa75fc),
};
const WrapSourcesEnv *g_env = &kGameEnv;

void *const *const kTrackedSourceVtable = at<void *const *>(0x00704a90);
void *const *const kSharedSourceVtable = at<void *const *>(0x00703ea0);
void *const *const kRefPackBufferVtable = at<void *const *>(0x00703ed0);
void *const *const kZlibBufferVtable = at<void *const *>(0x00703f18);
void *const *const kSliceSourceVtable = at<void *const *>(0x00703f48);
void *const *const kRefPackCodecVtable = at<void *const *>(0x00703f00);
void *const *const kCodecBaseVtable = at<void *const *>(0x00703148);
void *const *const kZlibCodecVtable = at<void *const *>(0x00703128);
void *const *const kCallbackStreamVtable = at<void *const *>(0x00703160);
void *const *const kCallbackStreamBaseVtable = at<void *const *>(0x00703170);

// The wrapped source's lock, taken for good; released when it goes out of scope.
struct LinkLock {
    SemaphoreLock lock;
    explicit LinkLock(TrackedSource *s) {
        const Semaphore *const sem = vcall<const Semaphore *>(s->base.link, 0x2c);
        lock.handle = sem->handle;
        lock.held = 0;
        lock.kept = 0;
        SemaphoreLock_Acquire(&lock, nullptr, 1);
    }
    ~LinkLock() { SemaphoreLock_Dtor(&lock); }
};

}  // namespace

void WrapSources_SetEnv(const WrapSourcesEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

u32 __fastcall Source_Field0c(const TrackedSource *s) { return s->position; }

TrackedSource *__fastcall TrackedSource_Ctor(TrackedSource *s) {
    RefObject_Ctor(&s->base);
    s->base.vtable = kTrackedSourceVtable;
    s->position = 0;
    return s;
}

void __fastcall TrackedSource_Dtor(TrackedSource *s) { RefObject_Dtor(&s->base); }

TrackedSource *__fastcall TrackedSource_DeletingDtor(TrackedSource *s, void *, u32 flags) {
    TrackedSource_Dtor(s);
    if (flags & 1) g_env->free(s);
    return s;
}

void __fastcall TrackedSource_Clear(TrackedSource *s) { s->position = 0; }

s32 __fastcall TrackedSource_Read(TrackedSource *s, void *, void *buffer, u32 count) {
    const s32 n = vcall<s32>(s->base.link, 4, buffer, count);
    s->position += n;
    return n;
}

s32 __fastcall TrackedSource_Write(TrackedSource *s, void *, const void *buffer, u32 count) {
    const s32 n = vcall<s32>(s->base.link, 8, buffer, count);
    s->position += n;
    return n;
}

s32 __fastcall TrackedSource_Seek(TrackedSource *s, void *, s32 offset, u32 origin) {
    const s32 p = vcall<s32>(s->base.link, 0xc, offset, origin);
    s->position = p;
    return p;
}

u32 __fastcall TrackedSource_Size(TrackedSource *s) { return vcall<u32>(s->base.link, 0x10); }

TrackedSource *__fastcall SharedSource_Ctor(TrackedSource *s) {
    TrackedSource_Ctor(s);
    s->base.vtable = kSharedSourceVtable;
    s->position = 0;
    return s;
}

s32 __fastcall SharedSource_Read(TrackedSource *s, void *, void *buffer, u32 count) {
    LinkLock lock(s);
    SharedSource_Sync(s);
    return TrackedSource_Read(s, nullptr, buffer, count);
}

s32 __fastcall SharedSource_Write(TrackedSource *s, void *, const void *buffer, u32 count) {
    LinkLock lock(s);
    SharedSource_Sync(s);
    return TrackedSource_Write(s, nullptr, buffer, count);
}

s32 __fastcall SharedSource_Seek(TrackedSource *s, void *, s32 offset, u32 origin) {
    LinkLock lock(s);
    SharedSource_Sync(s);
    return TrackedSource_Seek(s, nullptr, offset, origin);
}

u32 __fastcall SharedSource_Size(TrackedSource *s) {
    LinkLock lock(s);
    SharedSource_Sync(s);
    return TrackedSource_Size(s);
}

void __fastcall SharedSource_Sync(TrackedSource *s) {
    const u32 p = s->position;
    if (p != vcall<u32>(s->base.link, 0x14)) vcall<s32>(s->base.link, 0xc, p, 0u);
}

void __fastcall SharedSource_Attach(TrackedSource *s, void *, RefObject *source) {
    RefObject_SetLink(&s->base, nullptr, source);
    s->position = vcall<u32>(s->base.link, 0x14);
}

BufferSource *__fastcall BufferSource_Ctor(BufferSource *b) {
    RefObject_Ctor(&b->base);
    b->base.vtable = kRefPackBufferVtable;
    b->size = 0;
    b->data = nullptr;
    b->cursor = nullptr;
    return b;
}

BufferSource *__fastcall ZlibBufferSource_Ctor(BufferSource *b) {
    RefObject_Ctor(&b->base);
    b->base.vtable = kZlibBufferVtable;
    b->size = 0;
    b->data = nullptr;
    b->cursor = nullptr;
    return b;
}

s32 *__fastcall BufferSource_Open(BufferSource *b, void *, s32 *result, u32 size) {
    b->size = size;
    b->data = static_cast<u8 *>(g_env->alloc(size));
    b->cursor = b->data;
    *result = 0;
    return result;
}

s32 __fastcall BufferSource_Write(BufferSource *, void *, const void *, u32) { return 0; }

u32 __fastcall BufferSource_Read(BufferSource *b, void *, void *buffer, u32 count) {
    const u32 left = static_cast<u32>(b->data + b->size - b->cursor);
    const u32 n = count < left ? count : left;
    if (n) std::memcpy(buffer, b->cursor, n);
    b->cursor += n;
    return n;
}

s32 __fastcall BufferSource_Seek(BufferSource *b, void *, s32 offset, u32 origin) {
    switch (origin) {
    case 0:
        if (offset < 0 || offset > static_cast<s32>(b->size)) return -1;
        b->cursor = b->data + offset;
        return offset;
    case 1: {
        const std::uintptr_t to = reinterpret_cast<std::uintptr_t>(b->cursor) + static_cast<u32>(offset);
        const std::uintptr_t data = reinterpret_cast<std::uintptr_t>(b->data);
        if (static_cast<u32>(to) < static_cast<u32>(data)) return -1;
        if (static_cast<u32>(to) > static_cast<u32>(data + b->size)) return -1;
        b->cursor = reinterpret_cast<u8 *>(to);
        return static_cast<s32>(to - data);
    }
    case 2:
        if (offset > 0 || -offset > static_cast<s32>(b->size)) return -1;
        b->cursor = b->data + offset + b->size;
        return static_cast<s32>(b->cursor - b->data);
    default:
        return -1;
    }
}

u32 __fastcall BufferSource_Tell(const BufferSource *b) { return static_cast<u32>(b->cursor - b->data); }

void __fastcall BufferSource_Clear(BufferSource *b) {
    b->size = 0;
    g_env->free(b->data);
    b->cursor = nullptr;
}

void __fastcall BufferSource_AttachRefPack(BufferSource *b, void *, RefObject *source) {
    RefObject_SetLink(&b->base, nullptr, source);
    const u32 n = vcall<u32>(source, 0x10);
    u8 *const packed = static_cast<u8 *>(g_env->alloc(n));
    vcall<s32>(source, 4, static_cast<void *>(packed), n);
    RefPack_Decode(b->data, packed, 0);  // through a stack codec (0x703f00) that never reads this
    g_env->free(packed);
}

void __fastcall BufferSource_AttachZlib(BufferSource *b, void *, RefObject *source) {
    RefObject_SetLink(&b->base, nullptr, source);
    void *const buffer = g_env->alloc(0x4000);
    Buffer_DecompressStream(b->data, buffer, 0x4000, &BufferSource_ReadCallback, b->base.link, nullptr);
    g_env->free(buffer);
}

s32 __stdcall BufferSource_ReadCallback(void *buffer, u32 count, void *source) {
    vcall<s32>(static_cast<RefObject *>(source), 4, buffer, count);
    return 0;
}

void __fastcall Codec_Dtor(ZlibCodec *c) { c->vtable = kCodecBaseVtable; }

ZlibCodec *__fastcall Codec_DeletingDtor(ZlibCodec *c, void *, u32 flags) {
    Codec_Dtor(c);
    if (flags & 1) g_env->free(c);
    return c;
}

SliceSource *__fastcall SliceSource_Ctor(SliceSource *s) {
    RefObject_Ctor(&s->base);
    s->base.vtable = kSliceSourceVtable;
    s->size = 0;
    s->position = 0;
    s->start = 0;
    s->view = nullptr;
    return s;
}

s32 *__fastcall SliceSource_Open(SliceSource *s, void *, s32 *result, u32 size, u32 start) {
    s->size = size;
    s->start = start;
    s->position = 0;
    s->view = nullptr;
    *result = 0;
    return result;
}

s32 __fastcall SliceSource_Read(SliceSource *s, void *, void *buffer, u32 count) {
    const u32 left = s->size - s->position;
    const u32 n = count < left ? count : left;
    s32 r = 0;
    if (n) r = vcall<s32>(s->base.link, 4, buffer, n);
    s->position += r;
    return r;
}

s32 __fastcall SliceSource_Seek(SliceSource *s, void *, s32 offset, u32 origin) {
    s32 to;
    switch (origin) {
    case 0:
        if (offset < 0 || offset > static_cast<s32>(s->size)) return -1;
        to = offset + static_cast<s32>(s->start);
        break;
    case 1:
        if (s->position + static_cast<u32>(offset) > s->size) return -1;
        to = offset;
        break;
    case 2:
        if (offset > 0 || -offset > static_cast<s32>(s->size)) return -1;
        to = offset + static_cast<s32>(s->start + s->size);
        origin = 0;
        break;
    default:
        return -1;
    }
    const s32 p = vcall<s32>(s->base.link, 0xc, to, origin);
    if (p < 0) return -1;
    s->position = static_cast<u32>(p) - s->start;
    return static_cast<s32>(s->position);
}

void *__fastcall SliceSource_Map(SliceSource *s, void *, u32, u32) {
    if (!s->view) s->view = vcall<void *>(s->base.link, 0x18, s->start, s->size);
    return s->view;
}

void __fastcall SliceSource_Clear(SliceSource *s) { vcall<void>(s->base.link, 0x1c, s->view); }

ZlibCodec *__cdecl Compressor_Get() {
    const u8 guard = *g_env->guard;
    if (!(guard & 1)) {
        *g_env->guard = guard | 1;
        ZlibCodec_Ctor(g_env->compressor);
        g_env->atexit(g_env->atexitFn);
    }
    return g_env->compressor;
}

void __cdecl Compressor_AtExit() { Codec_Dtor(g_env->compressor); }

ZlibCodec *__fastcall ZlibCodec_Ctor(ZlibCodec *c) {
    c->vtable = kZlibCodecVtable;
    return c;
}

ZlibCodec *__fastcall CodecBase_DeletingDtor(ZlibCodec *c, void *, u32 flags) {
    c->vtable = kCodecBaseVtable;
    if (flags & 1) g_env->free(c);
    return c;
}

u32 __cdecl Buffer_Compress(const void *src, void *dst, u32 size, void *work) {
    ZlibCodec *const c = Compressor_Get();
    return vcall<u32>(c, 4, dst, src, size, work);
}

u32 __cdecl Buffer_Decompress(const void *src, void *dst, void *work) {
    ZlibCodec *const c = Compressor_Get();
    return vcall<u32>(c, 8, dst, src, work);
}

u32 __cdecl Buffer_UnpackedSize(const void *header) {
    ZlibCodec *const c = Compressor_Get();
    return vcall<u32>(c, 0x14, header);
}

u32 __cdecl Buffer_DecompressStream(void *dst, void *buffer, u32 capacity, StreamCallback read, void *context,
                                    void *work) {
    CallbackStream reader = {kCallbackStreamVtable, nullptr, read, context};
    ZlibCodec *const c = Compressor_Get();
    return vcall<u32>(c, 0x1c, dst, buffer, capacity, work, static_cast<void *>(&reader));
}

s32 __fastcall CallbackStream_Write(CallbackStream *c, void *, void *buffer, u32 count) {
    return c->write(buffer, count, c->context);
}

s32 __fastcall CallbackStream_Read(CallbackStream *c, void *, void *buffer, u32 count) {
    return c->read(buffer, count, c->context);
}

void __fastcall CallbackStream_Dtor(CallbackStream *c) { c->vtable = kCallbackStreamBaseVtable; }

CallbackStream *__fastcall CallbackStream_DeletingDtor(CallbackStream *c, void *, u32 flags) {
    CallbackStream_Dtor(c);
    if (flags & 1) g_env->free(c);
    return c;
}

CallbackStream *__fastcall CallbackStreamBase_DeletingDtor(CallbackStream *c, void *, u32 flags) {
    c->vtable = kCallbackStreamBaseVtable;
    if (flags & 1) g_env->free(c);
    return c;
}

}  // extern "C"
