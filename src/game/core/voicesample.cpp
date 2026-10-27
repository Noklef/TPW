#include "voicesample.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../files/handles.h"
#include "../files/substream.h"
#include "gameheap.h"
#include "semaphore.h"
#include "soundsample.h"
#include "soundstream.h"
#include "streampool.h"
#include "vcall.h"
#include "voicecodec.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr u32 kFrame = 0x1b0;

u32 stereo(const u8 *sample) { return ~at<u32>(sample, 0x34) & 1; }

}  // namespace

extern "C" {

u32 __fastcall VoiceSample_Decode(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *done) {
    *done = 0;
    const u32 channels = vcall<u8>(sample, 0x44);
    u32 frames = (bytes >> 1) / channels;
    if (!frames) return 0;
    do {
        if (at<u32>(sample, 0x30) == kFrame) {
            VoiceCodec_DecodeFrame(at<void *>(sample, 0x2c), nullptr, at<u8 *>(sample, 0x38));
            at<u32>(sample, 0x30) = 0;
            if (at<u32>(at<u8 *>(sample, 0x38), 0) >= at<u32>(sample, 0x3c)) return 1;
        }
        for (u32 c = 0, index = 0; c < channels; ++c, index += 0x34d) {
            const float x = at<float>(at<u8 *>(sample, 0x38), 0x688 + 4 * (at<u32>(sample, 0x30) + index));
            u32 v = static_cast<u32>(_mm_cvtss_si32(_mm_set_ss(x))) & 0x1ffff;   // fstp float; fld; fistp
            if (v > 0x7fff && v < 0x18000) v = 0x7fff + (v >= 0x10000 ? 1 : 0);
            *reinterpret_cast<u16 *>(buffer) = static_cast<u16>(v);
            buffer += 2;
            *done += 2;
        }
        ++at<u32>(sample, 0x30);
    } while (--frames);
    return 0;
}

u32 __fastcall VoiceSample_Read(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *done, u32 flag) {
    if (at<u32>(sample, 0x1c)) {
        *done = 0;
        return SoundSample_Drained(sample);
    }
    if (VoiceSample_Decode(sample, nullptr, buffer, bytes, done) == 1 && !vcall<u8>(sample, 0x4c, flag, *done)) return 1;
    return 0;
}

u32 *__fastcall VoiceSample_Query(u8 *sample, void *, u32 *out, u8 *) {
    vcall<void>(sample, 0x50, at<u32>(sample, 0x10));
    *out = 0;
    return out;
}

void __fastcall VoiceSample_Restart(u8 *sample) { vcall<void>(sample, 0x50, at<u32>(sample, 0x10)); }

void __fastcall VoiceSample_Close(u8 *sample) {
    if (void *const decoder = at<void *>(sample, 0x38)) {
        GameHeap_Free(decoder);
        at<void *>(sample, 0x38) = nullptr;
    }
    if (void *const reader = at<void *>(sample, 0x2c)) {
        GameHeap_Free(reader);
        at<void *>(sample, 0x2c) = nullptr;
    }
    SoundStream_Close(sample);
}

void __fastcall VoiceSample_Open(u8 *sample, void *, const u8 *data) {
    if (!at<u8 *>(sample, 0x2c)) {
        u32 *const reader = static_cast<u32 *>(GameHeap_Alloc(8));
        if (reader) {
            reader[0] = 0x70adb8;
            reader[1] = 0;
        }
        at<u32 *>(sample, 0x2c) = reader;
    }
    VoiceCodec_ReadHeader(at<void *>(sample, 0x2c), nullptr, data, at<u32 *>(sample, 0x38), stereo(sample));
    at<u32>(sample, 0x30) = kFrame;
}

void __fastcall VoiceStream_Open(u8 *sample, void *, u32) {
    if (!at<u8 *>(sample, 0x2c)) {
        u8 *const reader = static_cast<u8 *>(GameHeap_Alloc(0x200c));
        if (reader) {
            at<u32>(reader, 4) = 0;
            at<u32>(reader, 0) = 0x70ae28;
            at<u8 *>(reader, 0x2008) = sample + 0x40;   // (the sub-stream; the original tests the sample for null first)
        }
        at<u8 *>(sample, 0x2c) = reader;
    }
    VoiceCodec_ReadHeaderBuffered(at<u8 *>(sample, 0x2c), nullptr, at<u32 *>(sample, 0x38), stereo(sample));
    at<u32>(sample, 0x30) = kFrame;
}

void __fastcall VoiceStream_Restart(u8 *sample) {
    SubStream_Rewind(sample + 0x40);
    vcall<void>(sample, 0x50, at<u32>(sample, 0x10));
}

void __fastcall VoiceSample_Dtor(u8 *sample) { SoundSample_Dtor(sample); }

void __fastcall VoiceStream_Dtor(u8 *sample) {
    u8 *const sub = sample + 0x40;   // (the original tests the sample for null first)
    at<u32>(sub, 0) = 0x70ad50;
    Semaphore_Dtor(reinterpret_cast<Semaphore *>(sub + 0x14));
    StreamBase_Dtor(reinterpret_cast<StreamBase *>(sub + 0xc));
    SoundSample_Dtor(sample);
}

u8 *__fastcall VoiceSample_DeletingDtor(u8 *sample, void *, u32 flags) {
    VoiceSample_Dtor(sample);
    if (flags & 1) VoiceSamplePool_Delete(sample, 0x40);
    return sample;
}

u8 *__fastcall VoiceStream_DeletingDtor(u8 *sample, void *, u32 flags) {
    VoiceStream_Dtor(sample);
    if (flags & 1) VoiceStreamPool_Delete(sample, 0x5c);
    return sample;
}

}  // extern "C"
