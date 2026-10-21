#include "voicecodec.h"

#include <cstdint>
#include <cstring>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 Mask(u32 n) { return at<u32>(nullptr, 0x00783b80u + 4 * n); }

}  // namespace

extern "C" {

u32 __fastcall VoiceBits_Read(void *, void *, u32 *state, u32 n) {
    const u32 value = Mask(n) & state[1];
    const u32 bits = state[1] >> (n & 31);
    const u32 count = state[2] - n;
    state[2] = count;
    state[1] = bits;
    if (static_cast<s32>(count) < 8) {
        const u8 *const p = reinterpret_cast<const u8 *>((uintptr_t)state[0]);
        state[1] = static_cast<u32>(*p) << (count & 31) | bits;
        state[0] = static_cast<u32>((uintptr_t)(p + 1));
        state[2] = count + 8;
    }
    return value;
}

u32 *__fastcall VoiceBits_Skip(void *, void *, u32 *state, u32 n) {
    const u32 bits = state[1] >> (n & 31);
    const u32 count = state[2] - n;
    state[2] = count;
    state[1] = bits;
    if (static_cast<s32>(count) < 8) {
        const u8 *const p = reinterpret_cast<const u8 *>((uintptr_t)state[0]);
        state[1] = static_cast<u32>(*p) << (count & 31) | bits;
        state[0] = static_cast<u32>((uintptr_t)(p + 1));
        state[2] = count + 8;
    }
    return state;
}

u32 __fastcall VoiceBitsBuffered_Read(u8 *self, void *, u32 *state, u32 n) {
    const u32 value = Mask(n) & state[1];
    VoiceBitsBuffered_Skip(self, nullptr, state, n);
    return value;
}

void __fastcall VoiceBitsBuffered_Skip(u8 *self, void *, u32 *state, u32 n) {
    const u32 bits = state[1] >> (n & 31);
    const u32 count = state[2] - n;
    state[2] = count;
    state[1] = bits;
    if (static_cast<s32>(count) >= 8) return;
    const u8 *const p = reinterpret_cast<const u8 *>((uintptr_t)state[0]);
    state[1] = static_cast<u32>(*p) << (count & 31) | bits;
    state[0] = static_cast<u32>((uintptr_t)(p + 1));
    if (p + 1 == self + 0x2008) {
        vcall<u32>(at<u8 *>(self, 0x2008), 4, self + 8, 0x2000u);
        state[0] = static_cast<u32>((uintptr_t)(self + 8));
    }
    state[2] += 8;
}

void __fastcall VoiceCodec_InitChannel(void *, void *, void *, u8 *channel, u32 first, u32 scale, float factor) {
    at<u32>(channel, 0) = first;
    at<u32>(channel, 4) = scale;   // (its bits: a signalling NaN stays one)
    u8 *const powers = channel + 4;
    __asm {   // x87, as the original (SSE rounds denormal products once and picks other NaNs)
        mov eax, powers
        mov ecx, 0x3f
    next:
        fld dword ptr factor
        fmul dword ptr [eax]
        add eax, 4
        dec ecx
        fstp dword ptr [eax]
        jnz next
    }
    std::memset(channel + 0x104, 0, 0x570);
}

}  // extern "C"
