#include "soundsample.h"

#include <cstdint>

#include "../files/file.h"
#include "soundstream.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall SoundSample_Describe(u8 *sample, void *, u8 *record) { SoundStream_Describe(sample, nullptr, record); }

u8 __fastcall SoundSample_ScheduleEnd(u8 *sample, void *, u32 flag, u32 bytes) {
    if (static_cast<u8>(flag)) return 0;
    const u8 format = at<u8>(sample, 0x20);
    if (format & 0x40) bytes >>= 1;
    if (format & 2) bytes >>= 1;
    const u32 ms = bytes * 1000 / vcall<u32>(sample, 0x18);
    at<u32>(sample, 0x1c) = ms + Time_GetMs() + 200;
    return 1;
}

u32 __fastcall SoundSample_Drained(u8 *sample) { return Time_GetMs() >= at<u32>(sample, 0x1c) ? 1 : 0; }

void __fastcall SoundSample_Dtor(u8 *sample) {
    at<u32>(sample, 0) = 0x70ac50;
    if (u8 *const source = at<u8 *>(sample, 0x24)) vcall<void>(source, 0);
    at<u32>(sample, 0) = 0x70a778;
}

}  // extern "C"
