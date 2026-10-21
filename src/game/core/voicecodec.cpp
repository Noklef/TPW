#include "voicecodec.h"

#include <cstdint>
#include <cstring>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 Mask(u32 n) { return at<u32>(nullptr, 0x00783b80u + 4 * n); }

// The reader's slots: 0 skip, 1 read (state, n).
void Skip(void *reader, u32 *state, u32 n) { vcall<u32>(static_cast<u8 *>(reader), 0, state, n); }
u32 Read(void *reader, u32 *state, u32 n) { return vcall<u32>(static_cast<u8 *>(reader), 4, state, n); }

// The header's two floats (x87, as the original: the integer loaded as a qword, the products rounded as the control word says).
u32 Level(u32 n) {
    u32 q[2] = {n, 0};
    u32 out;
    __asm {
        fild qword ptr q
        fmul dword ptr ds:[0x00711bec]
        fstp dword ptr out
    }
    return out;
}

u32 Decay(u32 n) {
    u32 q[2] = {n, 0};
    u32 out;
    __asm {
        fild qword ptr q
        fmul dword ptr ds:[0x00711bf0]
        fsubr dword ptr ds:[0x00711bf4]
        fstp dword ptr out
    }
    return out;
}

// 0x6d5260's and 0x6d5350's part after the first byte.
void Header(void *reader, u32 *state, u32 stereo) {
    state[4] = stereo ? Read(reader, state, 1) : 0;
    state[3] = Read(reader, state, 1);
    const u32 first = 0x20 - Read(reader, state, 4);
    const u32 level = Level(Read(reader, state, 4) + 1);
    const u32 decay = Decay(Read(reader, state, 6));
    u8 *const self = reinterpret_cast<u8 *>(state);
    VoiceCodec_InitChannel(reader, nullptr, state, self + 0x14, first, level, decay);
    if (state[4]) VoiceCodec_InitChannel(reader, nullptr, state, self + 0xd48, first, level, decay);
}

// 0x6d57e0's order of the products (by history slot) in each output's sum.
const u8 kSynthesisOrder[12] = {7, 8, 5, 11, 9, 10, 1, 2, 0, 6, 3, 4};

// One output of 0x6d57e0: the 12 products k[(j + n) % 12] * slot[j] summed in kSynthesisOrder (x87, on its stack), plus *x, stored to
// slot[11 - n] and *x.
void SynthesisOutput(const float *k, float *slot, u32 n, float *x) {
    __asm {
        mov esi, k
        mov edi, slot
        mov ecx, n
        xor ebx, ebx
    term:
        movzx eax, byte ptr kSynthesisOrder[ebx]
        lea edx, [eax + ecx]
        cmp edx, 12
        jb inside
        sub edx, 12
    inside:
        fld dword ptr [esi + edx * 4]
        fmul dword ptr [edi + eax * 4]
        test ebx, ebx
        jz next
        faddp st(1), st(0)
    next:
        inc ebx
        cmp ebx, 12
        jb term
        mov eax, x
        fadd dword ptr [eax]
        mov edx, 11
        sub edx, ecx
        fst dword ptr [edi + edx * 4]
        fstp dword ptr [eax]
    }
}

// 0x6d5450's x87 pieces, as the original (each value rounded to a float where it stores one).
// A quarter of the step from the coefficient at old to the table's value at entry: (*entry - *old) * 0.25.
u32 Quarter(u32 entry, const u8 *old) {
    u32 out;
    __asm {
        mov eax, entry
        mov ecx, old
        fld dword ptr [eax]
        fsub dword ptr [ecx]
        fmul dword ptr ds:[0x00711bf8]
        fstp dword ptr out
    }
    return out;
}

// The long-term gain: n (loaded as a qword) / 15 (times 0x711bfc).
u32 Gain(u32 n) {
    u32 q[2] = {n, 0};
    u32 out;
    __asm {
        fild qword ptr q
        fmul dword ptr ds:[0x00711bfc]
        fstp dword ptr out
    }
    return out;
}

// A float through the x87 (a signalling NaN comes out quiet).
u32 Loaded(const u8 *p) {
    u32 out;
    __asm {
        mov eax, p
        fld dword ptr [eax]
        fstp dword ptr out
    }
    return out;
}

// x * 0.5 (0x711c0c).
u32 Half(u32 x) {
    u32 out;
    __asm {
        fld dword ptr x
        fmul dword ptr ds:[0x00711c0c]
        fstp dword ptr out
    }
    return out;
}

// The interpolated sample between the coded ones: (m[-5] + m[5]) * c0 - (m[-3] + m[3]) * c1 - (m[1] + m[-1]) * c2 (0x711c00 to 0x711c08).
void Interpolate(float *m) {
    __asm {
        mov eax, m
        fld dword ptr [eax - 0x14]
        fadd dword ptr [eax + 0x14]
        fmul dword ptr ds:[0x00711c00]
        fld dword ptr [eax - 0xc]
        fadd dword ptr [eax + 0xc]
        fmul dword ptr ds:[0x00711c04]
        fsubp st(1), st(0)
        fld dword ptr [eax + 4]
        fadd dword ptr [eax - 4]
        fmul dword ptr ds:[0x00711c08]
        fsubp st(1), st(0)
        fstp dword ptr [eax]
    }
}

// A subframe's 108 samples: amp * excitation[i] + gain * past[i].
void Subframe(u32 gain, u32 amp, const float *excitation, const u8 *past, u8 *out) {
    __asm {
        mov ecx, excitation
        mov edx, past
        mov eax, out
        mov ebx, 0x6c
    sample:
        fld dword ptr gain
        fld dword ptr amp
        fmul dword ptr [ecx]
        fxch st(1)
        fmul dword ptr [edx]
        faddp st(1), st(0)
        fstp dword ptr [eax]
        add ecx, 4
        add edx, 4
        add eax, 4
        dec ebx
        jnz sample
    }
}

// The 12 coefficients stepped by delta: coefficient + delta, or (deltaFirst) delta + coefficient, the second the memory operand.
void Step(u8 *coefficients, const float *delta, bool deltaFirst) {
    if (!deltaFirst) {
        __asm {
            mov eax, coefficients
            mov ecx, delta
            mov edx, 12
        plain:
            fld dword ptr [eax]
            fadd dword ptr [ecx]
            fstp dword ptr [eax]
            add eax, 4
            add ecx, 4
            dec edx
            jnz plain
        }
        return;
    }
    __asm {
        mov eax, coefficients
        mov ecx, delta
        mov edx, 12
    other:
        fld dword ptr [ecx]
        fadd dword ptr [eax]
        fstp dword ptr [eax]
        add eax, 4
        add ecx, 4
        dec edx
        jnz other
    }
}

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

void __fastcall VoiceCodec_InitChannel(void *, void *, void *, u8 *channel, u32 first, u32 scale, u32 factor) {
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

void __fastcall VoiceCodec_Excitation(void *reader, void *, u32 *state, u32 coded, u32 *out, s32 stride) {
    if (!coded) {
        for (s32 i = 0; i < 0x6c; i += stride, out += stride) {
            switch (state[1] & 3) {
                case 1:
                    *out = 0xc0000000u;   // -2.0f
                    Skip(reader, state, 2);
                    break;
                case 3:
                    *out = 0x40000000u;   // 2.0f
                    Skip(reader, state, 2);
                    break;
                default:
                    *out = 0;
                    Skip(reader, state, 1);
                    break;
            }
        }
        return;
    }
    u32 last = 0;
    for (s32 i = 0; i < 0x6c;) {
        const u32 code = at<u8>(nullptr, 0x00783ca8u + (last << 8) + (state[1] & 0xff));
        const u32 *const entry = &at<u32>(nullptr, 0x00783ea8u + 12 * code);
        last = entry[0];
        Skip(reader, state, entry[1]);
        if (static_cast<s32>(code) > 3) {
            out[i] = entry[2];
        } else if (static_cast<s32>(code) > 1) {
            s32 run = static_cast<s32>(Read(reader, state, 6) + 7);
            if (run * stride + i > 0x6c) run = (0x6c - i) / stride;
            if (run > 0) {
                for (s32 j = 0; j < run; ++j) out[i + j * stride] = 0;
                i += run * stride;
            }
            continue;
        } else {
            s32 size = 7;
            while (Read(reader, state, 1) == 1) ++size;
            const float value = static_cast<float>(Read(reader, state, 1) == 1 ? size : -size);
            std::memcpy(&out[i], &value, 4);
        }
        i += stride;
    }
}

void __fastcall VoiceCodec_ReadHeader(void *reader, void *, const u8 *data, u32 *state, u32 stereo) {
    state[0] = static_cast<u32>((uintptr_t)(data + 1));
    state[1] = *data;
    state[2] = 8;
    Header(reader, state, stereo);
}

void __fastcall VoiceCodec_ReadHeaderBuffered(u8 *reader, void *, u32 *state, u32 stereo) {
    vcall<u32>(at<u8 *>(reader, 0x2008), 4, reader + 8, 0x2000u);
    state[1] = reader[8];
    state[2] = 8;
    state[0] = static_cast<u32>((uintptr_t)(reader + 9));
    Header(reader, state, stereo);
}

void __cdecl VoiceCodec_Impulse(const float *k, float *out) {
    float lattice[12];   // [0] the input (1, then each step's output), [1] to [11] the lattice's state (k[0] to k[10] at first)
    float s[12];         // each step's output
    at<u32>(lattice, 0) = 0x3f800000u;
    std::memcpy(lattice + 1, k, 11 * 4);
    __asm {   // x87, as the original (the sum kept on the stack between the stages)
        mov esi, k
        mov ebx, out
        xor edi, edi
    step:
        lea ecx, lattice
        fld dword ptr [ecx + 0x2c]
        fmul dword ptr [esi + 0x2c]
        fchs
        lea eax, [esi + 0x28]
        add ecx, 0x28
        mov edx, 11
    stage:
        fld dword ptr [ecx]
        fmul dword ptr [eax]
        fsubp st(1), st(0)
        fld st(0)
        fmul dword ptr [eax]
        fadd dword ptr [ecx]
        fstp dword ptr [ecx + 4]
        sub ecx, 4
        sub eax, 4
        dec edx
        jnz stage
        lea ecx, lattice
        fst dword ptr [ecx]
        lea edx, s
        fst dword ptr [edx + edi * 4]
        test edi, edi
        jz store
        lea edx, [edx + edi * 4 - 4]
        xor eax, eax
    tail:
        fld dword ptr [edx]
        fmul dword ptr [ebx + eax * 4]
        fsubp st(1), st(0)
        sub edx, 4
        inc eax
        cmp eax, edi
        jl tail
    store:
        fstp dword ptr [ebx + edi * 4]
        inc edi
        cmp edi, 12
        jl step
    }
}

void __cdecl VoiceCodec_Synthesize(u8 *channel, u32 first, s32 blocks) {
    float k[12];
    VoiceCodec_Impulse(reinterpret_cast<const float *>(channel + 0x104), k);
    float *slot = reinterpret_cast<float *>(channel + 0x134);
    float *x = reinterpret_cast<float *>(channel + 0x674) + first;
    for (; blocks > 0; --blocks, x += 12) {
        for (u32 n = 0; n < 12; ++n) SynthesisOutput(k, slot, n, x + n);
    }
}

void __fastcall VoiceCodec_DecodeChannel(void *reader, void *, u8 *decoder, u8 *channel) {
    u32 *const state = reinterpret_cast<u32 *>(decoder);
    u32 delta[12];
    u32 v = Read(reader, state, 6);
    delta[0] = Quarter(0x00783ba8u + 4 * v, channel + 0x104);
    const u32 coded = v < at<u32>(channel, 0);
    for (u32 i = 1; i < 4; ++i) {
        v = Read(reader, state, 6);
        delta[i] = Quarter(0x00783ba8u + 4 * v, channel + 0x104 + 4 * i);
    }
    for (u32 i = 4; i < 12; ++i) {
        v = Read(reader, state, 5);
        delta[i] = Quarter(0x00783be8u + 4 * v, channel + 0x104 + 4 * i);
    }
    u8 *out = channel + 0x674;
    for (u32 position = 0xd8; position < 0x288; position += 0x6c, out += 0x6c * 4) {
        const u32 past = position - Read(reader, state, 8);
        const u32 gain = Gain(Read(reader, state, 4));
        u32 amp = Loaded(channel + 4 + 4 * Read(reader, state, 6));
        float buffer[5 + 0x6c + 5];   // the excitation between 5 guard samples each side (cleared for the interpolation)
        float *const excitation = buffer + 5;
        if (!state[3]) {
            VoiceCodec_Excitation(reader, nullptr, state, coded, reinterpret_cast<u32 *>(excitation), 1);
        } else {
            const u32 phase = Read(reader, state, 1);
            const u32 zeros = Read(reader, state, 1);
            VoiceCodec_Excitation(reader, nullptr, state, coded, reinterpret_cast<u32 *>(excitation + phase), 2);
            u8 *const other = reinterpret_cast<u8 *>(excitation) + 4 - 4 * phase;   // the samples between the coded ones
            if (zeros) {
                for (u32 i = 0; i < 0x1b0; i += 8) at<u32>(other, i) = 0;
            } else {
                std::memset(buffer, 0, 5 * 4);
                std::memset(excitation + 0x6c, 0, 5 * 4);
                for (u32 i = 0; i < 0x1b0; i += 8) Interpolate(reinterpret_cast<float *>(other + i));
                amp = Half(amp);
            }
        }
        Subframe(gain, amp, excitation, channel + 0x164 + 4 * past, out);
    }
    for (u32 i = 0; i < 0x144; ++i) at<u32>(channel, 0x164 + 4 * i) = at<u32>(channel, 0x824 + 4 * i);
    const float *const step = reinterpret_cast<const float *>(delta);
    Step(channel + 0x104, step, false);
    VoiceCodec_Synthesize(channel, 0, 1);
    Step(channel + 0x104, step, true);
    VoiceCodec_Synthesize(channel, 0xc, 1);
    Step(channel + 0x104, step, false);
    VoiceCodec_Synthesize(channel, 0x18, 1);
    Step(channel + 0x104, step, true);
    VoiceCodec_Synthesize(channel, 0x24, 0x21);
}

void __fastcall VoiceCodec_DecodeFrame(void *reader, void *, u8 *decoder) {
    VoiceCodec_DecodeChannel(reader, nullptr, decoder, decoder + 0x14);
    if (at<u32>(decoder, 0x10)) VoiceCodec_DecodeChannel(reader, nullptr, decoder, decoder + 0xd48);
}

}  // extern "C"
