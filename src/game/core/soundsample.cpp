#include "soundsample.h"

#include <cstdint>

#include "../files/file.h"
#include "gameheap.h"
#include "soundstream.h"
#include "vcall.h"
#include "vtables.h"

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

u8 *__fastcall SoundSample_Ctor(u8 *sample, void *, u8 *record, u8 *source) {
    u32 *const s = reinterpret_cast<u32 *>(sample);
    s[1] = 0;
    s[2] = 0;
    s[3] = 0;
    s[0] = 0x0070aca0;
    s[5] = 0;
    s[4] = static_cast<u32>(reinterpret_cast<uintptr_t>(record + 0x28));
    s[6] = *reinterpret_cast<const u32 *>(record + 4);
    s[7] = 0;
    s[9] = static_cast<u32>(reinterpret_cast<uintptr_t>(source));
    vcall<u32>(source, 4);
    s[0] = 0x0070ac50;
    SoundStream_SetFormat(sample, nullptr, record);
    return sample;
}

u8 *__fastcall SoundSample_DeletingDtor(u8 *sample, void *, u32 flags) {
    SoundSample_Dtor(sample);
    if (flags & 1) GameHeap_Free(sample);
    return sample;
}

u8 *__fastcall SoundSampleBase_DeletingDtor(u8 *stream, void *, u32 flags) {
    Vtbl70a778_Set_6c7090(stream);
    if (flags & 1) GameHeap_Free(stream);
    return stream;
}

u32 __fastcall SoundSample_ReadMemory(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *out, u32 flag) {
    if (at<u32>(sample, 0x1c)) {
        *out = 0;
        return SoundSample_Drained(sample);
    }
    const u32 left = at<u32>(sample, 0x18) - at<u32>(sample, 0x14);
    *out = bytes < left ? bytes : left;
    u8 *const copier = *reinterpret_cast<u8 **>(static_cast<uintptr_t>(0x00fb2018));
    vcall<void>(copier, 0, buffer, at<u32>(sample, 0x10) + at<u32>(sample, 0x14), *out);
    at<u32>(sample, 0x14) += *out;
    if (at<u32>(sample, 0x14) < at<u32>(sample, 0x18)) return 0;
    return vcall<u8>(sample, 0x4c, flag, *out) ? 0 : 1;
}

}  // extern "C"
