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

}  // extern "C"
