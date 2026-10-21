#include "mp2synth.h"

#include <cstdint>
#include <cstring>
#include <emmintrin.h>

#include "../math/x87.h"

namespace {

// The cosine factors at 0x711684 (1 / (2 cos((2k + 1) pi / 64)) and so on, as floats).
constexpr float kC[31] = {
    0x1.004f0ap-1f, 0x1.02cd16p-1f, 0x1.07e8b6p-1f, 0x1.0fe4dp-1f,
    0x1.1b307p-1f, 0x1.2a7674p-1f, 0x1.3eb8dcp-1f, 0x1.59807ap-1f,
    0x1.7d33dcp-1f, 0x1.adbf3cp-1f, 0x1.f1f476p-1f, 0x1.2b606ap+0f,
    0x1.7bf236p+0f, 0x1.07655ep+1f, 0x1.b42c84p+1f, 0x1.46148cp+3f,
    0x1.013d1ap-1f, 0x1.0b84fp-1f, 0x1.22467ep-1f, 0x1.4b2c3ap-1f,
    0x1.9389p-1f, 0x1.0f8892p+0f, 0x1.b8f24cp+0f, 0x1.467938p+2f,
    0x1.0503eep-1f, 0x1.33e37ap-1f, 0x1.ccc9aep-1f, 0x1.480d9ep+1f,
    0x1.1517a8p-1f, 0x1.4e7aeap+0f, 0x1.6a09e6p-1f,
};

// A float moved through an integer register: its bits, a signalling NaN included.
void CopyBits(float *to, const float &from) { std::memcpy(to, &from, 4); }

template <typename T> T Get(u32 address) {
    T v;
    std::memcpy(&v, reinterpret_cast<const void *>(static_cast<uintptr_t>(address)), sizeof v);
    return v;
}
template <typename T> void Put(u32 address, T v) { std::memcpy(reinterpret_cast<void *>(static_cast<uintptr_t>(address)), &v, sizeof v); }
u32 Addr(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

constexpr u32 kWindow = 0x70bd48;   // float[]: the synthesis window
constexpr u32 kHalfRate = 0xfb2708;  // (a dword: nonzero, half the samples)

// A window tap times a buffer value (fld window; fmul buffer).
X87Reg Tap(u32 window, u32 buffer) { return X87Reg(Get<float>(window)) * Get<float>(buffer); }

// fistp (round to nearest; 0x80000000 out of range or NaN) and the clip to a halfword.
s16 Pcm(X87Reg sum) {
    s32 n = _mm_cvtsd_si32(_mm_set_sd(sum.v));
    if (n > 0x7fff) n = 0x7fff;
    if (n < -0x8000) n = -0x8000;
    return static_cast<s16>(n);
}

// The 16 taps window[k] * buffer[k], the odd ones subtracted: four partial sums (0 8 -1 -9, 2 12 -5 -13, 4 10 -3 -11, 6 14 -7 -15).
X87Reg Alternating(u32 w, u32 b) {
    X87Reg a = Tap(w, b) + Tap(w + 0x20, b + 0x20), c = Tap(w + 8, b + 8) + Tap(w + 0x30, b + 0x30);
    X87Reg e = Tap(w + 0x10, b + 0x10) + Tap(w + 0x28, b + 0x28), g = Tap(w + 0x18, b + 0x18) + Tap(w + 0x38, b + 0x38);
    a = a - Tap(w + 4, b + 4), c = c - Tap(w + 0x14, b + 0x14), e = e - Tap(w + 0xc, b + 0xc), g = g - Tap(w + 0x1c, b + 0x1c);
    a = a - Tap(w + 0x24, b + 0x24), c = c - Tap(w + 0x34, b + 0x34), e = e - Tap(w + 0x2c, b + 0x2c), g = g - Tap(w + 0x3c, b + 0x3c);
    return (a + c) + (g + e);
}

// The even taps only.
X87Reg Even(u32 w, u32 b) {
    const X87Reg a = Tap(w, b) + Tap(w + 0x20, b + 0x20), c = Tap(w + 8, b + 8) + Tap(w + 0x30, b + 0x30);
    const X87Reg e = Tap(w + 0x10, b + 0x10) + Tap(w + 0x28, b + 0x28), g = Tap(w + 0x18, b + 0x18) + Tap(w + 0x38, b + 0x38);
    return (a + c) + (g + e);
}

// The mirrored taps window[-1 - k] * buffer[k] (window[0] * buffer[15] last), all added: partial sums (0 4 8 12, 1 6 10 14, 2 5 9 13,
// 3 7 11 15), negated.
X87Reg Mirrored(u32 w, u32 b) {
    auto q = [&](u32 k) { return Tap(k == 15 ? w : w - 4 * (k + 1), b + 4 * k); };
    X87Reg a = q(0) + q(4), c = q(1) + q(6), e = q(2) + q(5), g = q(3) + q(7);
    a = a + q(8), c = c + q(10), e = e + q(9), g = g + q(11);
    a = a + q(12), c = c + q(14), e = e + q(13), g = g + q(15);
    const X87Reg sum = (a + c) + (g + e);
    return X87Reg(-sum.v);
}

// The fixed-point form's (0x6ca580): the window of 16-bit taps (0x70c9e8 for the first half and the even taps, 0x70c5e8 for the mirrored
// ones, both less twice the ring position; already signed, so every tap is added), its sums kept in 32 bits as the MMX code does them.
constexpr u32 kWindowFixed = 0x70c9e8, kWindowFixedMirror = 0x70c5e8;
const Mp2SynthEnv kGameEnv = {&LbMpeg_MmxTransform};
const Mp2SynthEnv *g_env = &kGameEnv;

// The fixed-point transform's two tables of 16 rows of 16 Q14 halfwords (section lbmpeg_d): the sums' rows at 0xfc0200, the differences'
// at 0xfc0000.
constexpr u32 kTransformSums = 0xfc0200, kTransformDiffs = 0xfc0000;

// A row times 16 halfwords (pmaddwd, paddd: wrapping in 32 bits), shifted right 14 (psrad), its low halfword.
s16 TransformRow(u32 row, const s16 *x) {
    u32 sum = 0;
    for (u32 j = 0; j < 16; ++j) sum += static_cast<u32>(static_cast<s32>(Get<s16>(row + 2 * j))) * static_cast<u32>(static_cast<s32>(x[j]));
    return static_cast<s16>(static_cast<s32>(sum) >> 14);
}

// The 16 taps window[k] * buffer[k] (pmaddwd and paddd: wrapping in 32 bits).
u32 TapsFixed(u32 w, u32 b) {
    u32 sum = 0;
    for (u32 k = 0; k < 0x20; k += 2) sum += static_cast<u32>(static_cast<s32>(Get<s16>(w + k)) * Get<s16>(b + k));
    return sum;
}

// The even taps' products without their low halfwords (pmulhw, then pslld 16).
u32 EvenFixed(u32 w, u32 b) {
    u32 sum = 0;
    for (u32 k = 0; k < 0x20; k += 4) sum += static_cast<u32>(static_cast<s32>(Get<s16>(w + k)) * Get<s16>(b + k)) & 0xffff0000u;
    return sum;
}

// psrad 13 and packssdw.
s16 PcmFixed(u32 sum) {
    const s32 n = static_cast<s32>(sum) >> 13;
    return static_cast<s16>(n > 0x7fff ? 0x7fff : n < -0x8000 ? -0x8000 : n);
}

}  // namespace

void Mp2Synth_SetEnv(const Mp2SynthEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Mp2_Dct64(float *out0, float *out1, const float *in) {
    const float x1 = (float)((X87Reg(in[0]) - in[31]) * kC[0]);
    const float x2 = (float)(X87Reg(in[1]) + in[30]);
    const float x3 = (float)((X87Reg(in[1]) - in[30]) * kC[1]);
    const float x4 = (float)(X87Reg(in[2]) + in[29]);
    const float x5 = (float)((X87Reg(in[2]) - in[29]) * kC[2]);
    const float x6 = (float)(X87Reg(in[3]) + in[28]);
    const float x7 = (float)((X87Reg(in[3]) - in[28]) * kC[3]);
    const float x8 = (float)(X87Reg(in[4]) + in[27]);
    const float x9 = (float)((X87Reg(in[4]) - in[27]) * kC[4]);
    const float x10 = (float)(X87Reg(in[5]) + in[26]);
    const float x11 = (float)((X87Reg(in[5]) - in[26]) * kC[5]);
    const float x12 = (float)(X87Reg(in[6]) + in[25]);
    const float x13 = (float)((X87Reg(in[6]) - in[25]) * kC[6]);
    const float x14 = (float)(X87Reg(in[7]) + in[24]);
    const float x15 = (float)((X87Reg(in[7]) - in[24]) * kC[7]);
    const float x16 = (float)(X87Reg(in[8]) + in[23]);
    const float x17 = (float)((X87Reg(in[8]) - in[23]) * kC[8]);
    const float x18 = (float)(X87Reg(in[9]) + in[22]);
    const float x19 = (float)((X87Reg(in[9]) - in[22]) * kC[9]);
    const float x20 = (float)(X87Reg(in[10]) + in[21]);
    const float x21 = (float)((X87Reg(in[10]) - in[21]) * kC[10]);
    const float x22 = (float)(X87Reg(in[11]) + in[20]);
    const float x23 = (float)((X87Reg(in[11]) - in[20]) * kC[11]);
    const float x24 = (float)(X87Reg(in[12]) + in[19]);
    const float x25 = (float)((X87Reg(in[12]) - in[19]) * kC[12]);
    const float x26 = (float)(X87Reg(in[13]) + in[18]);
    const X87Reg r1 = X87Reg(in[15]) + in[16];
    const float x27 = (float)r1;
    const float x28 = (float)((X87Reg(in[13]) - in[18]) * kC[13]);
    const float x29 = (float)(X87Reg(in[14]) + in[17]);
    const float x30 = (float)((X87Reg(in[14]) - in[17]) * kC[14]);
    const float x31 = (float)((X87Reg(in[15]) - in[16]) * kC[15]);
    const X87Reg r2 = X87Reg(in[0]) + in[31];
    const float x32 = (float)(r1 + r2);
    const float x33 = (float)((r2 - x27) * kC[16]);
    const float x34 = (float)(X87Reg(x29) + x2);
    const float x35 = (float)((X87Reg(x2) - x29) * kC[17]);
    const float x36 = (float)(X87Reg(x26) + x4);
    const float x37 = (float)((X87Reg(x4) - x26) * kC[18]);
    const float x38 = (float)(X87Reg(x24) + x6);
    const float x39 = (float)((X87Reg(x6) - x24) * kC[19]);
    const float x40 = (float)(X87Reg(x22) + x8);
    const float x41 = (float)((X87Reg(x8) - x22) * kC[20]);
    const float x42 = (float)(X87Reg(x20) + x10);
    const float x43 = (float)((X87Reg(x10) - x20) * kC[21]);
    const float x44 = (float)(X87Reg(x18) + x12);
    const float x45 = (float)((X87Reg(x12) - x18) * kC[22]);
    const float x46 = (float)(X87Reg(x16) + x14);
    const float x47 = (float)((X87Reg(x14) - x16) * kC[23]);
    const float x48 = (float)(X87Reg(x1) + x31);
    const float x49 = (float)((X87Reg(x1) - x31) * kC[16]);
    const float x50 = (float)(X87Reg(x3) + x30);
    const float x51 = (float)((X87Reg(x3) - x30) * kC[17]);
    const float x52 = (float)(X87Reg(x5) + x28);
    const float x53 = (float)((X87Reg(x5) - x28) * kC[18]);
    const float x54 = (float)(X87Reg(x7) + x25);
    const float x55 = (float)((X87Reg(x7) - x25) * kC[19]);
    const float x56 = (float)(X87Reg(x9) + x23);
    const float x57 = (float)((X87Reg(x9) - x23) * kC[20]);
    const float x58 = (float)(X87Reg(x11) + x21);
    const float x59 = (float)((X87Reg(x11) - x21) * kC[21]);
    const float x60 = (float)(X87Reg(x13) + x19);
    const float x61 = (float)((X87Reg(x13) - x19) * kC[22]);
    const float x62 = (float)(X87Reg(x15) + x17);
    const float x63 = (float)((X87Reg(x15) - x17) * kC[23]);
    const float x64 = (float)((X87Reg(x32) - x46) * kC[24]);
    const float x65 = (float)(X87Reg(x34) + x44);
    const float x66 = (float)((X87Reg(x34) - x44) * kC[25]);
    const float x67 = (float)(X87Reg(x36) + x42);
    const float x68 = (float)((X87Reg(x36) - x42) * kC[26]);
    const float x69 = (float)(X87Reg(x38) + x40);
    const float x70 = (float)((X87Reg(x38) - x40) * kC[27]);
    const float x71 = (float)(X87Reg(x47) + x33);
    const float x72 = (float)((X87Reg(x33) - x47) * kC[24]);
    const float x73 = (float)(X87Reg(x45) + x35);
    const float x74 = (float)((X87Reg(x35) - x45) * kC[25]);
    const float x75 = (float)(X87Reg(x43) + x37);
    const float x76 = (float)((X87Reg(x37) - x43) * kC[26]);
    const float x77 = (float)(X87Reg(x41) + x39);
    const float x78 = (float)((X87Reg(x39) - x41) * kC[27]);
    const float x79 = (float)(X87Reg(x48) + x62);
    const float x80 = (float)((X87Reg(x48) - x62) * kC[24]);
    const float x81 = (float)(X87Reg(x50) + x60);
    const float x82 = (float)((X87Reg(x50) - x60) * kC[25]);
    const float x83 = (float)(X87Reg(x52) + x58);
    const float x84 = (float)((X87Reg(x52) - x58) * kC[26]);
    const float x85 = (float)(X87Reg(x54) + x56);
    const float x86 = (float)((X87Reg(x54) - x56) * kC[27]);
    const float x87 = (float)(X87Reg(x63) + x49);
    const float x88 = (float)((X87Reg(x49) - x63) * kC[24]);
    const float x89 = (float)(X87Reg(x61) + x51);
    const float x90 = (float)((X87Reg(x51) - x61) * kC[25]);
    const float x91 = (float)(X87Reg(x59) + x53);
    const float x92 = (float)((X87Reg(x53) - x59) * kC[26]);
    const float x93 = (float)(X87Reg(x57) + x55);
    const float x94 = (float)((X87Reg(x55) - x57) * kC[27]);
    const X87Reg r3 = X87Reg(x46) + x32;
    const float x95 = (float)(X87Reg(x69) + r3);
    const float x96 = (float)((r3 - x69) * kC[28]);
    const float x97 = (float)(X87Reg(x67) + x65);
    const float x98 = (float)((X87Reg(x65) - x67) * kC[29]);
    const float x99 = (float)(X87Reg(x64) + x70);
    const float x100 = (float)((X87Reg(x64) - x70) * kC[28]);
    const float x101 = (float)(X87Reg(x66) + x68);
    const float x102 = (float)((X87Reg(x66) - x68) * kC[29]);
    const float x103 = (float)(X87Reg(x77) + x71);
    const float x104 = (float)((X87Reg(x71) - x77) * kC[28]);
    const float x105 = (float)(X87Reg(x75) + x73);
    const float x106 = (float)((X87Reg(x73) - x75) * kC[29]);
    const float x107 = (float)(X87Reg(x72) + x78);
    const float x108 = (float)((X87Reg(x72) - x78) * kC[28]);
    const float x109 = (float)(X87Reg(x74) + x76);
    const float x110 = (float)((X87Reg(x74) - x76) * kC[29]);
    const float x111 = (float)(X87Reg(x85) + x79);
    const float x112 = (float)((X87Reg(x79) - x85) * kC[28]);
    const float x113 = (float)(X87Reg(x83) + x81);
    const float x114 = (float)((X87Reg(x81) - x83) * kC[29]);
    const float x115 = (float)(X87Reg(x80) + x86);
    const float x116 = (float)((X87Reg(x80) - x86) * kC[28]);
    const float x117 = (float)(X87Reg(x82) + x84);
    const float x118 = (float)((X87Reg(x82) - x84) * kC[29]);
    const float x119 = (float)(X87Reg(x93) + x87);
    const float x120 = (float)((X87Reg(x87) - x93) * kC[28]);
    const float x121 = (float)(X87Reg(x91) + x89);
    const float x122 = (float)((X87Reg(x89) - x91) * kC[29]);
    const float x123 = (float)(X87Reg(x88) + x94);
    const float x124 = (float)((X87Reg(x88) - x94) * kC[28]);
    const float x125 = (float)(X87Reg(x90) + x92);
    const float x126 = (float)((X87Reg(x90) - x92) * kC[29]);
    const float x127 = (float)(X87Reg(x97) + x95);
    const float x128 = (float)((X87Reg(x95) - x97) * kC[30]);
    const X87Reg r4 = (X87Reg(x96) - x98) * kC[30];
    const float x129 = (float)((X87Reg(x98) + x96) + r4);
    const float x130 = (float)(X87Reg(x99) + x101);
    const X87Reg r5 = X87Reg(x102) + x100;
    const float x131 = (float)r5;
    const float x132 = (float)((X87Reg(x99) - x101) * kC[30]);
    const float x133 = (float)((X87Reg(x100) - x102) * kC[30]);
    const float x134 = (float)(r5 + x133);
    const float x135 = (float)(X87Reg(x134) + x130);
    const float x136 = (float)(X87Reg(x134) + x132);
    const float x137 = (float)(X87Reg(x133) + x132);
    const float x138 = (float)(X87Reg(x103) + x105);
    const float x139 = (float)((X87Reg(x103) - x105) * kC[30]);
    const X87Reg r6 = X87Reg(x106) + x104;
    const float x140 = (float)r6;
    const float x141 = (float)((X87Reg(x104) - x106) * kC[30]);
    const float x142 = (float)(r6 + x141);
    const float x143 = (float)(X87Reg(x107) + x109);
    const X87Reg r7 = X87Reg(x110) + x108;
    const float x144 = (float)r7;
    const float x145 = (float)((X87Reg(x107) - x109) * kC[30]);
    const float x146 = (float)((X87Reg(x108) - x110) * kC[30]);
    const X87Reg r8 = r7 + x146;
    const float x147 = (float)r8;
    const float x148 = (float)(r8 + x143);
    const float x149 = (float)(X87Reg(x147) + x145);
    const float x150 = (float)(X87Reg(x146) + x145);
    const float x151 = (float)(X87Reg(x111) + x113);
    const X87Reg r9 = X87Reg(x114) + x112;
    const float x152 = (float)r9;
    const float x153 = (float)((X87Reg(x112) - x114) * kC[30]);
    const float x154 = (float)(r9 + x153);
    const float x155 = (float)((X87Reg(x111) - x113) * kC[30]);
    const float x156 = (float)(X87Reg(x115) + x117);
    const float x157 = (float)((X87Reg(x115) - x117) * kC[30]);
    const X87Reg r10 = X87Reg(x118) + x116;
    const float x158 = (float)r10;
    const float x159 = (float)((X87Reg(x116) - x118) * kC[30]);
    const X87Reg r11 = r10 + x159;
    const float x160 = (float)r11;
    const float x161 = (float)(r11 + x156);
    const float x162 = (float)(X87Reg(x160) + x157);
    const float x163 = (float)(X87Reg(x159) + x157);
    const float x164 = (float)(X87Reg(x119) + x121);
    const float x165 = (float)((X87Reg(x119) - x121) * kC[30]);
    const X87Reg r12 = X87Reg(x122) + x120;
    const float x166 = (float)r12;
    const float x167 = (float)((X87Reg(x120) - x122) * kC[30]);
    const float x168 = (float)(r12 + x167);
    const float x169 = (float)(X87Reg(x123) + x125);
    const X87Reg r13 = X87Reg(x126) + x124;
    const float x170 = (float)r13;
    const float x171 = (float)((X87Reg(x123) - x125) * kC[30]);
    const float x172 = (float)((X87Reg(x124) - x126) * kC[30]);
    const X87Reg r14 = r13 + x172;
    const float x173 = (float)r14;
    const float x174 = (float)(r14 + x169);
    const float x175 = (float)(X87Reg(x173) + x171);
    const float x176 = (float)(X87Reg(x172) + x171);
    out0[256] = X87_Through(x127);
    out0[192] = X87_Through(x135);
    out0[128] = X87_Through(x129);
    out0[64] = X87_Through(x136);
    out0[0] = X87_Through(x128);
    out1[0] = X87_Through(x128);
    CopyBits(&out1[192], x133);
    out1[64] = X87_Through(x137);
    out1[128] = (float)r4;
    const float x177 = (float)(X87Reg(x138) + x148);
    CopyBits(&out0[224], x177);
    const float x178 = (float)(X87Reg(x142) + x148);
    CopyBits(&out0[160], x178);
    const float x179 = (float)(X87Reg(x142) + x149);
    CopyBits(&out0[96], x179);
    const float x180 = (float)(X87Reg(x139) + x149);
    CopyBits(&out0[32], x180);
    const float x181 = (float)(X87Reg(x139) + x150);
    CopyBits(&out1[32], x181);
    const float x182 = (float)(X87Reg(x141) + x150);
    CopyBits(&out1[96], x182);
    const float x183 = (float)(X87Reg(x141) + x146);
    const X87Reg r15 = X87Reg(x164) + x174;
    const float x184 = (float)r15;
    CopyBits(&out1[160], x183);
    CopyBits(&out1[224], x146);
    out0[240] = (float)(r15 + x151);
    const X87Reg r16 = X87Reg(x168) + x174;
    const float x185 = (float)r16;
    out0[208] = (float)(X87Reg(x184) + x161);
    const X87Reg r17 = X87Reg(x168) + x175;
    const float x186 = (float)r17;
    out0[176] = (float)(r16 + x161);
    const X87Reg r18 = X87Reg(x165) + x175;
    const float x187 = (float)r18;
    out0[144] = (float)(X87Reg(x185) + x154);
    const X87Reg r19 = X87Reg(x165) + x176;
    const float x188 = (float)r19;
    out0[112] = (float)(r17 + x154);
    out0[80] = (float)(X87Reg(x186) + x162);
    out0[48] = (float)(r18 + x162);
    out0[16] = (float)(X87Reg(x187) + x155);
    out1[16] = (float)(r19 + x155);
    const X87Reg r20 = X87Reg(x167) + x172;
    const float x189 = (float)r20;
    out1[48] = (float)(X87Reg(x188) + x163);
    const X87Reg r21 = X87Reg(x167) + x176;
    out1[80] = (float)(r21 + x163);
    out1[112] = (float)(r21 + x153);
    out1[144] = (float)(r20 + x153);
    out1[176] = (float)(X87Reg(x189) + x159);
    out1[208] = (float)(X87Reg(x172) + x159);
    out1[240] = X87_Through(x172);
}

void __fastcall Mp2Decoder_Synth(u8 *decoder_, void *, s16 *out, const float *fraction) {
    const u32 decoder = Addr(decoder_);
    Put<u8>(decoder + 0x243d, static_cast<u8>((Get<u8>(decoder + 0x243d) - 1) & 0xf));
    const s32 channels = Get<s32>(decoder + 0x244c);
    u32 samples = Addr(fraction), pcm = Addr(out);
    for (s32 c = 0; c < channels; ++c, samples += 0x80, pcm += 2) {
        const u32 bo = Get<u8>(decoder + 0x243d), odd = bo & 1;
        const u32 buffers = decoder + 4 + 0x900 * (~static_cast<u32>(c) & 1);
        const u32 bo1 = bo + (odd ^ 1);
        const u32 b0 = buffers + 0x480 * (odd ^ 1);
        Mp2_Dct64(reinterpret_cast<float *>(static_cast<uintptr_t>(buffers + 4 * (((bo + odd) & 0xf) + 0x120 * odd))),
                  reinterpret_cast<float *>(static_cast<uintptr_t>(b0 + 4 * bo1)), reinterpret_cast<const float *>(static_cast<uintptr_t>(samples)));
        if (!out) continue;
        const bool half = Get<u32>(kHalfRate) != 0;
        const u32 count = half ? 8 : 16, wstep = half ? 0x100 : 0x80, bstep = half ? 0x80 : 0x40;
        const u32 stride = 2 * static_cast<u32>(channels);
        u32 w = kWindow - 4 * bo1, b = b0, o = pcm;
        for (u32 n = count; n; --n, o += stride, w += wstep, b += bstep) Put<s16>(o, Pcm(Alternating(w, b)));
        Put<s16>(o, Pcm(Even(w, b)));
        o += stride;
        w = w - wstep + 8 * bo1;
        b -= bstep;
        for (u32 n = count - 1; n; --n, o += stride, w -= wstep, b -= bstep) Put<s16>(o, Pcm(Mirrored(w, b)));
    }
}

void __cdecl LbMpeg_MmxTransform(s16 *out0, s16 *out1, const s16 *in) {
    s16 sum[16], diff[16];
    for (u32 j = 0; j < 16; ++j) {
        const u16 a = static_cast<u16>(in[j]), b = static_cast<u16>(in[31 - j]);
        sum[j] = static_cast<s16>(static_cast<u16>(a + b));  // paddw / psubw: wrapping
        diff[j] = static_cast<s16>(static_cast<u16>(a - b));
    }
    const u32 o0 = Addr(out0), o1 = Addr(out1);
    for (u32 k = 0; k < 16; ++k) {
        const s16 v = TransformRow(kTransformSums + 0x20 * k, sum);
        if (k <= 8) Put<s16>(o0 + 0x200 - 0x40 * k, v);
        if (k >= 8) Put<s16>(o1 + 0x40 * (k - 8), v);
    }
    for (u32 k = 0; k < 16; ++k) {
        const s16 v = TransformRow(kTransformDiffs + 0x20 * k, diff);
        Put<s16>(k < 8 ? o0 + 0x1e0 - 0x40 * k : o1 + 0x20 + 0x40 * (k - 8), v);
    }
}

void __fastcall Mp2Decoder_SynthFixed(u8 *decoder_, void *, s16 *out, const s16 *fraction) {
    const u32 decoder = Addr(decoder_);
    const u32 bo = (Get<u8>(decoder + 0x243d) - 1u) & 0xf;
    Put<u8>(decoder + 0x243d, static_cast<u8>(bo));
    const u32 odd = bo & 1, bo1 = bo + (odd ^ 1);
    for (s32 c = 0; c < Get<s32>(decoder + 0x244c); ++c) {
        const u32 buffers = decoder + 4 + 0x480 * static_cast<u32>(c), b = buffers + 0x240 * (odd ^ 1);
        g_env->transform(reinterpret_cast<s16 *>(static_cast<uintptr_t>(buffers + 2 * (((bo + odd) & 0xf) + 0x120 * odd))),
                         reinterpret_cast<s16 *>(static_cast<uintptr_t>(b + 2 * bo1)), fraction + 0x20 * c);
        if (!out) continue;
        const bool half = Get<u32>(kHalfRate) != 0;
        const u32 count = half ? 8 : 16, wstep = half ? 0x80 : 0x40, bstep = half ? 0x40 : 0x20;
        const u32 stride = Get<s32>(decoder + 0x244c) == 2 ? 4 : 2;
        const u32 w = kWindowFixed - 2 * bo1, wm = kWindowFixedMirror - 2 * bo1;
        u32 o = Addr(out) + 2 * static_cast<u32>(c);
        for (u32 n = 0; n < count; ++n, o += stride) Put<s16>(o, PcmFixed(TapsFixed(w + wstep * n, b + bstep * n)));
        Put<s16>(o, PcmFixed(EvenFixed(w + 0x400, b + 0x200)));
        o += stride;
        for (u32 n = 1; n < count; ++n, o += stride) Put<s16>(o, PcmFixed(TapsFixed(wm + wstep * (n - 1), b + 0x200 - bstep * n)));
    }
}

}  // extern "C"
