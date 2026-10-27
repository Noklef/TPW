#include "soundstream.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"
#include "fieldops.h"
#include "gameheap.h"
#include "initializers.h"
#include "silencestream.h"
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

u32 *__fastcall SoundStream_QueryUser(u8 *stream, void *, u32 *out, u8 *record) {
    u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(record));   // (the record's own stack slot)
    if (static_cast<s32>(*SoundStream_Query(stream, nullptr, &slot, record)) < 0) {
        *out = 0xffffffff;
        return out;
    }
    if (at<u32>(record, 0x14) & 0x1000) at<u32>(stream, 0x10) = at<u32>(record, 0x48);
    *out = 0;
    return out;
}

void __fastcall SoundStream_DescribeUser(u8 *stream, void *, u8 *record) {
    if (at<u32>(record, 0x18) & 0x1000) SetThenFlag48_14_6c5090(record, nullptr, at<u32>(stream, 0x10));
}

u32 *__fastcall Stream70a6c8_Query(u8 *stream, void *, u32 *out, u8 *record) {
    u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(record));   // (the record's own stack slot)
    if (static_cast<s32>(*SoundStream_QueryUser(stream, nullptr, &slot, record)) < 0) {
        *out = 0xffffffff;
        return out;
    }
    if (at<u32>(record, 0x14) & 0x800) at<u32>(stream, 0x18) = at<u32>(record, 0x4c);
    *out = 0;
    return out;
}

u32 __fastcall Stream70a6c8_NextSource(u8 *stream) {
    u8 *const old = at<u8 *>(stream, 4);
    at<u32>(stream, 4) = 0;
    vcall<void>(at<u8 *>(stream, 0x18), 0, at<u32>(stream, 0xc), at<u32>(stream, 0x10));
    if (old) vcall<void>(old, 8);
    return at<u32>(stream, 4) == 0;
}

u32 __fastcall Stream70a6c8_Read(u8 *stream, void *, u8 *buffer, u32 count, u32 *done, u32) {
    bool switched = false;
    if (!at<u8 *>(stream, 4)) return 1;
    *done = 0;
    while (*done < count) {
        u32 got = 0;   // (the done argument's own stack slot)
        if (vcall<u32>(at<u8 *>(stream, 4), 0x1c, buffer + *done, count - *done, &got, 1u) == 1) {
            if (Stream70a6c8_NextSource(stream) == 1) {
                *done += got;
                if (switched) vcall<void>(stream, 0xc);
                return 1;
            }
            switched = true;
        }
        *done += got;
    }
    if (switched) vcall<void>(stream, 0xc);
    return 0;
}

void __fastcall Stream70a6c8_Describe(u8 *stream, void *, u8 *record) {
    SoundStream_DescribeUser(stream, nullptr, record);
    if (at<u32>(record, 0x18) & 0x800) SetThenFlag4c_14_6c50f0(record, nullptr, at<u32>(stream, 0x18));
}

u8 *__fastcall NotifyStream_Create(void *, void *, u8 *record) {
    const u32 flags = at<u32>(record, 0x14);
    if (!(flags & 0x2000)) return nullptr;
    at<u32>(record, 0x14) = flags & ~0x2000u;
    u8 *const stream = static_cast<u8 *>(NotifyStreamPool_New(0x1c));
    if (!stream) return nullptr;
    at<u32>(stream, 4) = 0;
    at<u32>(stream, 8) = 0;
    at<u32>(stream, 0xc) = 0;
    at<u32>(stream, 0x10) = at<u32>(record, 0x48);
    at<u32>(stream, 0x18) = at<u32>(record, 0x50);
    at<u32>(stream, 0) = 0x0070a748;
    return stream;
}

u8 *__fastcall NotifyStream_Dtor(u8 *stream, void *, u32 flags) {
    Vtbl70a778_Set_6c7090(stream);
    if (flags & 1) NotifyStreamPool_Delete(stream, 0x1c);
    return stream;
}

u32 __fastcall NotifyStream_Read(u8 *stream, void *, u32 buffer, u32 count, u32 done, u32 flag) {
    u8 *const next = next_of(stream);
    return next ? vcall<u32>(next, 0x1c, buffer, count, done, flag) : 0;
}

u32 *__fastcall NotifyStream_Query(u8 *stream, void *, u32 *out, u8 *record) {
    u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(record));   // (the record's own stack slot)
    if (static_cast<s32>(*SoundStream_QueryUser(stream, nullptr, &slot, record)) < 0) {
        *out = 0xffffffff;
        return out;
    }
    if (at<u32>(record, 0x14) & 0x2000) at<u32>(stream, 0x18) = at<u32>(record, 0x50);
    *out = 0;
    return out;
}

void __fastcall NotifyStream_Send(u8 *stream, void *, u32 value) {
    u8 *const callback = at<u8 *>(stream, 0x18);
    if (!callback) return;
    vcall<void>(callback, 4, at<u32>(stream, 0xc), at<u32>(stream, 0x10), value & 0xff);
    at<u32>(stream, 0x18) = 0;
}

void __fastcall SoundStream_SetFormat(u8 *stream, void *, const u8 *format) {
    stream[0x20] = (format[0x1b] & 1) ? 0x40 : 0x20;
    stream[0x20] |= format[0x1a] == 0x10 ? 2 : 1;
    // (AX, the rate's word; its high half the format's address's, sic)
    vcall<void>(stream, 0x3c, static_cast<u32>((reinterpret_cast<uintptr_t>(format) & 0xffff0000u) | at<u16>(format, 0x18)));
}

u8 *__stdcall SoundStream_BuildChain(u8 *record, const u8 *format, u8 *stream) {
    u32 &flags = *reinterpret_cast<u32 *>(record + 0x14);
    if (flags & 0x4000) {
        u8 *const block = static_cast<u8 *>(Stream70abf8Pool_New(0x14));
        u8 *const seeker = block ? Init_6c6d30(block) : nullptr;
        if (stream) {
            vcall<u8 *>(seeker, 4, stream);
            u32 status = 1;
            const s32 sought = static_cast<s32>(*SoundStream_SeekTime(seeker, nullptr, &status, *reinterpret_cast<const u32 *>(record + 0x54)));
            vcall<void>(seeker, 0xc);
            if (sought < 0) return stream;
        }
    }
    const u32 loops = *reinterpret_cast<const u32 *>(record + 0x44);
    if ((flags & 0x40) && loops > 1) {
        u8 *filter;
        if (*reinterpret_cast<const u32 *>(format + 0x20) && *reinterpret_cast<const u32 *>(format + 0x24)) {
            u8 *const block = static_cast<u8 *>(LoopStreamPool_New(0x24));
            filter = block ? reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_6c6a00(block, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(format)), loops)))
                           : nullptr;
        } else {
            filter = static_cast<u8 *>(StreamFilterPool_New(0x18));
            if (filter) {
                u32 *const f = reinterpret_cast<u32 *>(filter);
                f[1] = 0;
                f[2] = 0;
                f[3] = 0;
                f[5] = loops;
                f[0] = 0x0070a678;
            }
        }
        stream = SoundStream_Chain(stream, filter);
        flags &= ~0x40u;
    }
    if ((flags & 0x200000) && *reinterpret_cast<const u32 *>(record + 0x58)) {
        u8 *const block = static_cast<u8 *>(SilenceStreamPool_New(0x30));
        u8 *const silence = block ? SilenceStream_Ctor(block, nullptr, *reinterpret_cast<const u32 *>(record + 0x58), format) : nullptr;
        stream = SoundStream_Chain(stream, silence);
    }
    return stream;
}

}  // extern "C"
