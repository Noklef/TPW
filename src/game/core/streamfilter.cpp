#include "streamfilter.h"

#include <cstdint>

#include "fieldops.h"
#include "streampool.h"
#include "vtables.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *inner(u8 *stream) { return vcall<u8 *>(stream, 0x48); }

}  // namespace

extern "C" {

u32 __fastcall StreamFilter_SampleRate(u8 *stream) { return vcall<u32>(inner(stream), 0x18); }

u32 __fastcall StreamFilter_Restart(u8 *stream) { return vcall<u32>(inner(stream), 0x20); }

u32 __fastcall StreamFilter_Position(u8 *stream) { return vcall<u32>(inner(stream), 0x34); }

u32 __fastcall StreamFilter_Length(u8 *stream) { return vcall<u32>(inner(stream), 0x38); }

u32 __fastcall StreamFilter_Bits(u8 *stream) { return vcall<u32>(inner(stream), 0x40); }

u32 __fastcall StreamFilter_Channels(u8 *stream) { return vcall<u32>(inner(stream), 0x44); }

u32 __fastcall StreamFilter_Seek(u8 *stream, void *, u32 position) { return vcall<u32>(inner(stream), 0x30, position); }

u32 __fastcall StreamFilter_SetFormat(u8 *stream, void *, u32 format) { return vcall<u32>(inner(stream), 0x3c, format); }

u32 *__fastcall StreamFilter_Query(u8 *stream, void *, u32 *out, u8 *record) {
    vcall<void>(inner(stream), 0x24, out, record);
    return out;
}

u32 __fastcall StreamFilter_Describe(u8 *stream, void *, u8 *record) {
    const u32 answer = vcall<u32>(inner(stream), 0x14, record);
    if (at<u8>(record, 0x18) & 0x40) return SetThenFlag44_14_6c5110(record, nullptr, at<u32>(stream, 0x14));
    return answer;
}

u32 *__fastcall StreamFilter_QueryLoops(u8 *stream, void *, u32 *out, u8 *record) {
    u32 status = static_cast<u32>(reinterpret_cast<uintptr_t>(record));
    if (static_cast<s32>(*StreamFilter_Query(stream, nullptr, &status, record)) < 0) {
        *out = 0xffffffff;
        return out;
    }
    if (at<u8>(record, 0x14) & 0x40) at<u32>(stream, 0x14) = at<u32>(record, 0x44);
    *out = 0;
    return out;
}

u32 __fastcall StreamFilter_Read(u8 *stream, void *, u8 *buffer, u32 count, u32 *done, u32) {
    *done = 0;
    if (!count) return 0;
    do {
        u32 got = 0;
        if (vcall<u32>(at<u8 *>(stream, 4), 0x1c, buffer + *done, count - *done, &got, 1u) == 1) {
            u32 loops = at<u32>(stream, 0x14);
            if (loops != 9999) at<u32>(stream, 0x14) = --loops;
            if (!loops) return 1;
            vcall<void>(inner(stream), 0x20);
        }
        *done += got;
    } while (*done < count);
    return 0;
}

u8 *__fastcall StreamFilter_Dtor(u8 *stream, void *, u32 flags) {
    Vtbl70a778_Set_6c7090(stream);
    if (flags & 1) StreamFilterPool_Delete(stream, 0x18);
    return stream;
}

u8 *__fastcall Stream70abf8_Dtor(u8 *stream, void *, u32 flags) {
    Vtbl70a778_Set_6c7090(stream);
    if (flags & 1) Stream70abf8Pool_Delete(stream, 0x14);
    return stream;
}

void __fastcall Stream70abf8_Describe(u8 *stream, void *, u8 *record) {
    u8 *const next = vcall<u8 *>(stream, 0x48);
    vcall<void>(next, 0x14, record);
}

}  // extern "C"
