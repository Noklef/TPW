#include "soundstream.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *next_of(u8 *stream) { return at<u8 *>(stream, 4); }

}  // namespace

extern "C" {

u8 *__fastcall SoundStream_Dtor(u8 *stream, void *, u32 flags) {
    at<u32>(stream, 0) = 0x70a778;
    if (flags & 1) GameHeap_Free(stream);
    return stream;
}

u8 *__fastcall SoundStream_SetNext(u8 *stream, void *, u8 *next) {
    if (next) {
        if (u8 *const old = next_of(stream)) vcall<void>(old, 0x8);
        at<u8 *>(stream, 4) = next;
        at<u8 *>(next, 8) = stream;
    }
    return next;
}

u8 *__cdecl SoundStream_Chain(u8 *next, u8 *stream) {
    vcall<u8 *>(stream, 0x4, next);
    return stream;
}

void __fastcall SoundStream_Close(u8 *stream) {
    if (u8 *const next = next_of(stream)) {
        vcall<void>(next, 0x8);
        at<u8 *>(stream, 4) = nullptr;
    }
    if (stream) vcall<void>(stream, 0x0, 1u);
}

void __fastcall SoundStream_Unlink(u8 *stream) {
    if (u8 *const prev = at<u8 *>(stream, 8)) {
        if (u8 *const next = next_of(stream)) {
            at<u8 *>(next, 8) = prev;
            at<u8 *>(at<u8 *>(stream, 8), 4) = next_of(stream);   // (both re-read after the store)
        } else {
            at<u8 *>(prev, 4) = nullptr;
        }
    }
    if (stream) vcall<void>(stream, 0x0, 1u);
}

u8 *__fastcall SoundStream_Find(u8 *stream, void *, u32 id) {
    if (at<u32>(stream, 0xc) == id) return stream;
    if (u8 *const next = next_of(stream)) return vcall<u8 *>(next, 0x10, id);
    return nullptr;
}

void __fastcall SoundStream_Describe(u8 *stream, void *, u8 *record) {
    if (u8 *const next = next_of(stream)) vcall<void>(next, 0x14, record);
}

void __fastcall SoundStream_Send(u8 *stream, void *, u32 value) {
    if (u8 *const next = next_of(stream)) vcall<void>(next, 0x2c, value);
}

u32 __fastcall SoundStream_SampleRate(u8 *stream) {
    if (u8 *const next = next_of(stream)) return vcall<u32>(next, 0x18);
    return 0;
}

u32 *__fastcall SoundStream_Query(u8 *stream, void *, u32 *out, u8 *record) {
    u32 local = 0;
    if (u8 *const next = next_of(stream)) {
        local = static_cast<u32>(reinterpret_cast<uintptr_t>(record));
        local = *vcall<u32 *>(next, 0x24, &local, record);
    }
    *out = local;
    return out;
}

}  // extern "C"
