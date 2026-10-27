#include "silencestream.h"

#include <cstdint>
#include <cstring>

#include "fieldops.h"
#include "soundstream.h"
#include "streampool.h"
#include "vcall.h"
#include "voicesample.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall SilenceStream_Read(u8 *stream, void *, u8 *buffer, u32 bytes, u32 *done, u32 flag) {
    if (!stream[0x2c]) {
        if (!vcall<u32>(at<u8 *>(stream, 4), 0x1c, buffer, bytes, done, flag)) return 0;
        stream[0x2c] = 1;
    }
    const u32 left = at<u32>(stream, 0x18) - at<u32>(stream, 0x14);
    const u32 want = bytes - *done;
    const u32 n = want < left ? want : left;
    memset(buffer + *done, 0, n);
    *done += n;
    at<u32>(stream, 0x14) += n;
    return at<u32>(stream, 0x14) >= at<u32>(stream, 0x18) ? 1 : 0;
}

u8 *__fastcall SilenceStream_DeletingDtor(u8 *stream, void *, u32 flags) {
    VoiceSample_Dtor(stream);
    if (flags & 1) SilenceStreamPool_Delete(stream, 0x30);
    return stream;
}

u8 *__fastcall SilenceStream_Ctor(u8 *stream, void *, u32 ms, const u8 *format) {
    at<u32>(stream, 4) = 0;
    at<u32>(stream, 8) = 0;
    at<u32>(stream, 0xc) = 0;
    at<u32>(stream, 0x14) = 0;
    at<u32>(stream, 0x24) = 0;
    stream[0x2c] = 0;
    at<u32>(stream, 0) = 0x70ab58;
    SoundStream_SetFormat(stream, nullptr, format);
    const u8 flags = stream[0x20];
    at<u32>(stream, 0x18) = ms;
    if (flags & 0x40) at<u32>(stream, 0x18) = ms + ms;
    if (flags & 2) at<u32>(stream, 0x18) <<= 1;
    at<u32>(stream, 0x18) = Obj_SampleRate20(stream) * at<u32>(stream, 0x18) / 1000 & ~1u;
    return stream;
}

}  // extern "C"
