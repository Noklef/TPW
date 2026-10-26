#include "soundstream.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"
#include "gameheap.h"
#include "streampool.h"
#include "vcall.h"
#include "vtables.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *next_of(u8 *stream) { return at<u8 *>(stream, 4); }

// fld; fistp (to nearest; out of range 0x80000000, as cvtss2si).
s32 fistp(float x) { return _mm_cvtss_si32(_mm_set_ss(x)); }

}  // namespace

extern "C" {

u32 *__fastcall SoundStream_SeekTime(u8 *stream, void *, u32 *out, u32 ms) {
    if (!next_of(stream)) {
        *out = 0xffffffff;
        return out;
    }
    const u32 rate = vcall<u32>(stream, 0x18);
    const float per_ms = static_cast<float>(X87_MulInt(ms, 0.001f));   // (the float at 0x70ac44)
    float bytes = static_cast<float>(X87_MulInt(rate, per_ms));
    if (vcall<u8>(stream, 0x44) == 2) bytes = bytes + bytes;
    if (vcall<u8>(stream, 0x40) == 0x10) bytes = bytes + bytes;
    u32 offset = static_cast<u32>(fistp(bytes));
    if (offset & 3) offset += offset & 3;   // (sic: not rounded up to the next multiple of 4)
    const u32 length = vcall<u32>(stream, 0x38);
    if (offset <= length) {
        vcall<void>(stream, 0x30, offset);
    } else if (length) {
        vcall<void>(stream, 0x30, offset % length);
    }
    *out = 0;
    return out;
}

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

u8 *__fastcall Stream70a6c8_Dtor(u8 *stream, void *, u32 flags) {
    Vtbl70a778_Set_6c7090(stream);
    if (flags & 1) Stream70a6c8Pool_Delete(stream, 0x1c);
    return stream;
}

}  // extern "C"
