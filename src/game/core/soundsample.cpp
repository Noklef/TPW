#include "soundsample.h"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#include "../files/file.h"
#include "../files/handles.h"
#include "../files/substream.h"
#include "gameheap.h"
#include "misc.h"
#include "fieldops.h"
#include "semaphore.h"
#include "sounddevice.h"
#include "soundstream.h"
#include "streampool.h"
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

u32 __fastcall SoundSample_ReadFile(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *out, u32 flag) {
    if (at<u32>(sample, 0x1c)) {
        *out = 0;
        return SoundSample_Drained(sample);
    }
    const u32 left = at<u32>(sample, 0x18) - at<u32>(sample, 0x30);
    *out = bytes < left ? bytes : left;
    vcall<u32>(sample + 0x2c, 4, buffer, *out);
    if (at<u32>(sample, 0x30) < at<u32>(sample, 0x18)) return 0;
    return vcall<u8>(sample, 0x4c, flag, *out) ? 0 : 1;
}

u32 __fastcall SoundSample_RewindFile(u8 *sample) { return SubStream_Rewind(sample + 0x2c); }

u8 *__fastcall FileSample_Ctor(u8 *sample, void *, const u8 *stream, u8 *record, u32 start, u8 *source) {
    SoundSample_Ctor(sample, nullptr, record, source);
    at<u32>(sample, 0) = 0x0070a6f8;
    SubStream_CtorAt(sample + 0x2c, nullptr, stream, start);
    at<u32>(sample, 0x2c) = 0x0070ad40;
    at<u32>(sample, 0) = 0x0070acf0;
    return sample;
}

void __fastcall FileSample_Dtor(u8 *sample) {
    u8 *const sub = sample + 0x2c;   // (the original tests the sample for null first)
    at<u32>(sub, 0) = 0x0070ad50;
    Semaphore_Dtor(reinterpret_cast<Semaphore *>(sub + 0x14));
    StreamBase_Dtor(reinterpret_cast<StreamBase *>(sub + 0xc));
    SoundSample_Dtor(sample);
}

u8 *__fastcall FileSample_DeletingDtor(u8 *sample, void *, u32 flags) {
    FileSample_Dtor(sample);
    if (flags & 1) FileSamplePool_Delete(sample, 0x48);
    return sample;
}

u8 *__fastcall SampleCopier_Install(u8 *self) {
    const bool mmx = *reinterpret_cast<u32 *>(static_cast<uintptr_t>(0x00fb2704)) != 0;
    u32 *const copier = static_cast<u32 *>(GameHeap_Alloc(4));
    if (copier) *copier = mmx ? 0x0070aa38 : 0x0070aa34;
    *reinterpret_cast<u32 **>(static_cast<uintptr_t>(0x00fb2018)) = copier;
    return self;
}

void __fastcall SampleCopier_MmxCopy(void *, void *, u8 *dest, const u8 *source, u32 count) {
    for (u32 blocks = count >> 5; blocks; --blocks, source += 32, dest += 32) {
        uint64_t q[4];   // (the four qwords read before any is written)
        std::memcpy(q, source, sizeof q);
        std::memcpy(dest, q, sizeof q);
    }
    __movsb(dest, source, count & 31);
}

u8 *__fastcall SoundSample70a548_Ctor(u8 *self) {
    Class703770_Ctor(self);
    for (u32 offset = 8; offset <= 0x20; offset += 4) at<u32>(self, offset) = 0;
    SampleCopier_Install(self + 0x24);
    at<u32>(self, 0) = 0x0070a548;
    return self;
}

void __fastcall SoundSample70a548_Choose3DVoices(u8 *, void *, u8 *settings) {
    u8 caps[8];   // (a caps holder of no sound: DirectSoundCreate makes one)
    SoundCaps_Ctor(caps, nullptr, nullptr);
    u32 status;
    u32 voices;
    const u32 max = static_cast<s32>(*SoundCaps_Query(caps, nullptr, &status)) >= 0 ? SoundCaps_Max3DStreaming(caps) : 0;
    if (max && SoundCaps_Free3DStreaming(caps) >= at<u32>(settings, 0x40) && max >= 5)
        voices = at<u8>(settings, 0x4c) & 0x60 ? max - 1 : max;
    else
        voices = at<u32>(settings, 0x40);
    SetThenFlag44_4c_6b9d10(settings, nullptr, voices);
    SoundCaps_Dtor(caps);
}

}  // extern "C"
