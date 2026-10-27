#include "mp2sample.h"

#include <cstdint>
#include <cstring>
#include <emmintrin.h>

#include "../files/substream.h"
#include "../math/x87.h"
#include "initializers.h"
#include "mp2decoder.h"
#include "mp2pool.h"
#include "soundsample.h"
#include "soundstream.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 *answer(u32 *out, u32 value) {
    *out = value;
    return out;
}

// After the open: the length, the rate check and the position check.
u32 *opened(u8 *sample, u32 *out) {
    if (!(sample[0x2c] & 1)) at<u32>(sample, 0x30) = at<u32>(at<u8 *>(sample, 0x34), 0x240c);
    const float rate = static_cast<float>(static_cast<double>(vcall<u32>(sample, 0x18)));   // (fild qword; fstp float)
    const s32 decoded = at<s32>(at<u8 *>(sample, 0x34), 0x2408);
    if (static_cast<double>(decoded) != static_cast<double>(rate)) {
        const u16 word = static_cast<u16>(decoded);
        vcall<void>(sample, 0x3c, static_cast<u32>(word));
        if (!word) return answer(out, 0xffffffff);
        const float ratio = static_cast<float>(X87_DivR(X87Reg(static_cast<double>(word)), X87Reg(rate)));   // (fdivr: rate / word)
        const u32 divisor = static_cast<u32>(_mm_cvtsd_si32(_mm_set_sd(ratio)));   // (fistp: to nearest; 0x80000000 past the range)
        if (!divisor) return answer(out, 0xffffffff);
        at<u32>(sample, 0x30) = at<u32>(sample, 0x30) / divisor & ~1u;
    }
    return answer(out, at<u32>(sample, 0x14) < at<u32>(sample, 0x30) ? 0 : 0xffffffff);
}

}  // namespace

extern "C" {

u32 *__fastcall Mp2BufferSample_Open(u8 *sample, void *, u32 *out) {
    if (at<u32>(sample, 0x34)) return answer(out, 0);
    u8 *const decoder = static_cast<u8 *>(Mp2BufferPool_New(0x2478));
    if (decoder) Init_6c9260(decoder);
    at<u8 *>(sample, 0x34) = decoder;
    if (vcall<s32>(decoder, 4, at<u32>(sample, 0x10), at<u32>(sample, 0x18)) == -1) return answer(out, 0xffffffff);
    return opened(sample, out);
}

u32 *__fastcall Mp2StreamSample_Open(u8 *sample, void *, u32 *out) {
    if (at<u32>(sample, 0x34)) return answer(out, 0);
    u8 *const decoder = static_cast<u8 *>(Mp2StreamPool_New(0x2c80));
    if (decoder) {
        Init_6c9260(decoder);
        at<u32>(decoder, 0) = 0x0070af00;
    }
    at<u8 *>(sample, 0x34) = decoder;
    if (Mp2StreamDecoder_Open(decoder, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(sample + 0x40)), at<s32>(sample, 0x18)) == -1)
        return answer(out, 0xffffffff);
    vcall<void>(sample, 0x50);
    return opened(sample, out);
}

u32 __fastcall Mp2Sample_Read(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *done, u32 flag) {
    if (at<u32>(sample, 0x1c)) {
        *done = 0;
        return SoundSample_Drained(sample);
    }
    const u8 *decoder = at<u8 *>(sample, 0x34);
    if (sample[0x3c] && decoder[0x2442] == 2 && at<u32>(decoder, 0x2420) == 0x30) {
        sample[0x3c] = 0;
        memset(buffer, 0, bytes);
        *done = bytes;
        return 0;
    }
    const u32 left = at<u32>(sample, 0x30) - at<u32>(sample, 0x14);
    const u32 n = bytes < left ? bytes : left;
    *done = n;
    vcall<void>(sample, 0x5c, buffer, n);
    at<u32>(sample, 0x14) += *done;
    if (at<u32>(sample, 0x14) >= at<u32>(sample, 0x30) && !vcall<u8>(sample, 0x4c, flag, *done)) return 1;
    return 0;
}

u32 *__fastcall Mp2Sample_Query(u8 *sample, void *, u32 *out, const u8 *record) {
    u32 status;
    if (static_cast<s32>(*vcall<u32 *>(sample, 0x54, &status)) < 0) {
        *out = 0xffffffff;
        return out;
    }
    if (at<u8 *>(sample, 0x34)[0x2442] == 1 && (at<u32>(record, 0x14) & 0x840)) vcall<void>(sample, 0x5c, 0x00fb23d8u, 0x300u);
    *out = 0;
    return out;
}

void __fastcall Mp2Sample_Close(u8 *sample) {
    vcall<void>(sample, 0x58);
    SoundStream_Close(sample);
}

void __fastcall Mp2BufferSample_Restart(u8 *sample) {
    at<u32>(sample, 0x14) = 0;
    Fn_6d1a50(at<u8 *>(sample, 0x34));
    if (at<u8 *>(sample, 0x34)[0x2442] == 1) vcall<void>(sample, 0x5c, 0x00fb20d0u, 0x300u);
}

void __fastcall Mp2StreamSample_Restart(u8 *sample) {
    at<u32>(sample, 0x14) = 0;
    SubStream_Rewind(sample + 0x40);
    Fn_6d1a50(at<u8 *>(sample, 0x34));
}

void __fastcall Mp2Sample_FreeDecoder(u8 *sample) {
    if (!at<u8 *>(sample, 0x34)) return;
    vcall<void>(at<u8 *>(sample, 0x34), 8);
    if (u8 *const decoder = at<u8 *>(sample, 0x34)) vcall<void>(decoder, 0, 1);
    at<u8 *>(sample, 0x34) = nullptr;
}

}  // extern "C"
