#include "mp2decoder.h"

#include <cstdint>
#include <cstring>
#include <mmintrin.h>

#include "cpuinfo.h"
#include "initializers.h"
#include "mp2layer1.h"
#include "mp2layer2.h"
#include "mp2pool.h"
#include "trivial.h"
#include "vcall.h"

namespace {

template <typename T> T Get(u32 address) {
    T v;
    std::memcpy(&v, reinterpret_cast<const void *>(static_cast<uintptr_t>(address)), sizeof v);
    return v;
}
template <typename T> void Put(u32 address, T v) { std::memcpy(reinterpret_cast<void *>(static_cast<uintptr_t>(address)), &v, sizeof v); }
u32 Addr(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

constexpr u32 kFixedPoint = 0xfb2704;  // (a dword: nonzero, the MMX decoder)
constexpr u32 kHalfRate = 0xfb2708;    // (a dword: nonzero, half the samples)

// A thiscall virtual call with no arguments / one argument: slot k of the object's vtable.
u32 Call0(u32 object, u32 k) {
    return reinterpret_cast<u32(__fastcall *)(u32, void *)>(static_cast<uintptr_t>(Get<u32>(Get<u32>(object) + 4 * k)))(object, nullptr);
}
u32 Call1(u32 object, u32 k, u32 a) {
    return reinterpret_cast<u32(__fastcall *)(u32, void *, u32)>(static_cast<uintptr_t>(Get<u32>(Get<u32>(object) + 4 * k)))(object, nullptr, a);
}

// The opens' shared start: closes the decoder if open (slot 2), sets +0x2428 (and what the caller set before), marks it open, reads the
// header (slot 5).
bool Start(u32 d, u32 buffer) {
    if (Get<u8>(d + 0x243c)) Call0(d, 2);
    Put<u32>(d + 0x2428, buffer);
    Put<u8>(d + 0x243c, 1);
    return Call0(d, 5) == 0;
}

// The frames of the whole (bytes + granules) (rate / 100) / ((frame bytes (rate / 100) + rest) granules), 32-bit products.
s32 Frames(u32 d, s32 bytes) {
    u8 *decoder = reinterpret_cast<u8 *>(static_cast<uintptr_t>(d));
    Mp2Decoder_ClearBuffers(decoder, nullptr);
    Mp2Decoder_SetupOutput(decoder, nullptr);
    Fn_6d1a50(decoder);
    const u32 rate100 = Get<u32>(d + 0x2460), granules = Get<u32>(d + 0x2454);
    const u32 per = (Get<u32>(d + 0x2458) * rate100 + Get<u32>(d + 0x245c)) * granules;
    return Mp2_MulDiv(static_cast<s32>(static_cast<u32>(bytes) + granules), static_cast<s32>(rate100), static_cast<s32>(per));
}

}  // namespace

extern "C" {

void __fastcall Mp2Decoder_ClearBuffers(u8 *decoder, void *) {
    std::memset(decoder + 4, 0, Get<u32>(kFixedPoint) ? 0x240 * 4 : 0x480 * 4);
}

s32 __fastcall Mp2Decoder_SetupOutput(u8 *decoder, void *) {
    const u32 d = Addr(decoder);
    const s32 rate = Get<s32>(d + 0x2424);
    const s32 rate100 = rate / 100;
    Put<s32>(d + 0x2460, rate100);
    const u32 channels = Get<u32>(d + 0x244c);
    if (Get<u8>(d + 0x2442) == 1) {
        const u32 bitrate = Get<u32>(d + 0x2420);
        Put<u32>(d + 0x246c, channels * 3 << 8);
        Put<u32>(d + 0x2454, 4);
        Put<s32>(d + 0x2458, static_cast<s32>(bitrate * 12000u) / rate + 1);  // (idiv: faults, sic)
        Put<s32>(d + 0x245c, static_cast<s32>(bitrate * 12u) % rate100);
    } else {
        Put<u32>(d + 0x2454, 1);
        Put<u32>(d + 0x246c, channels * 9 << 8);
        const s32 bytes = static_cast<s32>(Get<u32>(d + 0x2420) * 144000u);
        Put<s32>(d + 0x2458, bytes / rate);
        Put<s32>(d + 0x245c, bytes % rate / 100);
    }
    Put<u8>(d + 0x243d, 1);
    Put<s32>(d + 0x2474, Get<u32>(kHalfRate) ? Get<s32>(d + 0x246c) / 2 : Get<s32>(d + 0x246c));
    return 0;
}

u32 __cdecl Mp2_HasMmx() {
    u32 r[4];
    CpuInfo_GetEnv()->cpuid(1, r);
    if (!(r[3] & 0x800000)) return 0;
    _mm_empty();
    return 1;
}

void __cdecl Mp2_InitFixedPoint() { *reinterpret_cast<u32 *>(static_cast<uintptr_t>(kFixedPoint)) = Mp2_HasMmx(); }

s32 __cdecl Mp2_MulDiv(s32 a, s32 b, s32 c) {
    s32 q;
    __asm {
        mov eax, a
        imul b
        idiv c
        mov q, eax
    }
    return q;
}

u32 __fastcall Mp2StreamDecoder_Seek(u8 *decoder, void *, u32 position) {
    const u32 d = Addr(decoder);
    const u32 r = Call1(Get<u32>(d + 0x2c7c), 2, position);
    Put<u32>(d + 0x242c, 0);
    return r;
}

s32 __fastcall Mp2Decoder_Close(u8 *decoder, void *) {
    if (decoder[0x243c]) {
        decoder[0x243c] = 0;
        Zero_5d60c0();
    }
    return 0;
}

s32 __fastcall Mp2BufferDecoder_Open(u8 *decoder, void *, u32 buffer, s32 bytes) {
    const u32 d = Addr(decoder);
    if (!Start(d, buffer)) return -1;
    const s32 frames = Frames(d, bytes);
    Put<s32>(d + 0x2414, frames);
    Put<u32>(d + 0x240c, Get<u32>(d + 0x246c) * static_cast<u32>(frames));
    return 0;
}

s32 __fastcall Mp2StreamDecoder_Open(u8 *decoder, void *, u32 stream, s32 bytes) {
    const u32 d = Addr(decoder);
    if (Get<u8>(d + 0x243c)) Call0(d, 2);
    Put<u32>(d + 0x2c7c, stream);
    Put<u8>(d + 0x243c, 1);
    Put<u32>(d + 0x2428, d + 0x247c);
    if (Call0(d, 5)) return -1;
    const s32 frames = Frames(d, bytes);
    Put<u32>(d + 0x240c, Get<u32>(d + 0x246c) * static_cast<u32>(frames));
    Put<s32>(d + 0x2414, frames);
    Call1(Get<u32>(d + 0x2c7c), 2, 0);
    return 0;
}

void __fastcall Mp2Decoder_Dtor(u8 *decoder, void *) {
    *reinterpret_cast<u32 *>(decoder) = 0x007118e0;
    if (decoder[0x243c]) Mp2Decoder_Close(decoder, nullptr);
}

u8 *__fastcall Mp2BufferDecoder_DeletingDtor(u8 *decoder, void *, u32 flags) {
    Mp2Decoder_Dtor(decoder, nullptr);
    if (flags & 1) Mp2BufferPool_Delete(decoder, 0x2478);
    return decoder;
}

u8 *__fastcall Mp2StreamDecoder_DeletingDtor(u8 *decoder, void *, u32 flags) {
    Mp2Decoder_Dtor(decoder, nullptr);
    if (flags & 1) Mp2StreamPool_Delete(decoder, 0x2c80);
    return decoder;
}

s32 __fastcall Mp2Decoder_DecodeFrame(u8 *decoder, void *, s16 *pcm) {
    if (vcall<u32>(decoder, 0x10)) return 0;
    const u8 layer = decoder[0x2442];
    if (layer == 2) return Mp2Decoder_DecodeLayer2(decoder, nullptr, pcm);
    if (layer == 1) return Mp2Decoder_DecodeLayer1(decoder, nullptr, pcm);
    return 0;
}

}  // extern "C"
