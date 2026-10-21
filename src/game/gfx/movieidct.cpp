#include "movieidct.h"

#include <cstdint>
#include <cstring>

#include "../math/x87.h"

namespace {

// The constants: the Q31 multiplier for IMUL, three float multipliers and the rounding constant.
struct IdctConstants {
    s32 k;
    float c1, c2, c3, round;
};

IdctConstants *const kDctConstants = reinterpret_cast<IdctConstants *>((uintptr_t)0x00fbc018);
IdctConstants *const kMovieConstants = reinterpret_cast<IdctConstants *>((uintptr_t)0x00fbb598);
u32 *const kDctScratch = reinterpret_cast<u32 *>((uintptr_t)0x00fbc008);
s32 **const kDctOut = reinterpret_cast<s32 **>((uintptr_t)0x00fbc004);

u32 HighTwice(u32 a, s32 k) { return (u32)(s32)(((long long)(s32)a * k) >> 32) << 1; }  // imul; shl edx, 1

u32 LowDword(double d) {
    u32 parts[2];
    std::memcpy(parts, &d, 8);
    return parts[0];
}

// The listing's order of reads and writes: the scratch (4 dwords) is written before the block's dwords 2, 6, 0 and 4 are read,
// the out pointer read from *outAt after it, and the scratch's last three read back between the stores.
void Transform(const s32 *block, u32 *scratch, const IdctConstants *c, s32 *const *outAt, s32 step, int shift) {
    const u32 s5 = (u32)block[5], s3 = (u32)block[3], s1 = (u32)block[1], s7 = (u32)block[7];
    const u32 sum35 = s3 + s5, sum17 = s7 + s1;
    scratch[0] = s5 - s3;
    scratch[1] = s1 - s7;
    const u32 even = sum35 + sum17;
    const double a = (double)(s32)scratch[0], b = (double)(s32)scratch[1];
    const X87Reg p = X87_MulDouble(b, c->c2);
    const X87Reg q = X87_Add(b, a);
    const X87Reg r = X87_MulDouble(a, c->c1);
    const X87Reg s = X87_MulDouble(q.v, c->c3);
    const u32 h1 = HighTwice(sum17 - sum35, c->k);
    const X87Reg u = (r + s) + X87Reg(c->round);
    const X87Reg v = (p - s) + X87Reg(c->round);
    const u32 s2 = (u32)block[2], s6 = (u32)block[6];
    std::memcpy(scratch + 2, &v.v, 8);
    std::memcpy(scratch, &u.v, 8);
    const u32 lowU = scratch[0], lowV = scratch[2];
    scratch[1] = lowU + h1;
    scratch[2] = h1 + lowV;
    scratch[3] = lowV + even;
    const u32 s0 = (u32)block[0], s4 = (u32)block[4];
    const u32 sum04 = s4 + s0, diff04 = s0 - s4;
    const u32 h2 = HighTwice(s2 - s6, c->k);
    s32 *const out = *outAt;
    const u32 sum26 = s6 + s2 + h2;
    const u32 odd = h2 + diff04, low = diff04 - h2;
    const u32 top = sum26 + sum04, mid = sum04 - sum26;
    const u32 t1 = scratch[1];
    const u32 u0 = scratch[0];
    out[2 * step] = (s32)(low + t1) >> shift;
    const u32 t2 = scratch[2];
    out[5 * step] = (s32)(low - t1) >> shift;
    const u32 t3 = scratch[3];
    out[3 * step] = (s32)(mid + u0) >> shift;
    out[4 * step] = (s32)(mid - u0) >> shift;
    out[0] = (s32)(top + t3) >> shift;
    out[1 * step] = (s32)(t2 + odd) >> shift;
    out[6 * step] = (s32)(odd - t2) >> shift;
    out[7 * step] = (s32)(top - t3) >> shift;
}

// Dwords 1-7 all zero: dword 0 down the column.
bool FilledFlat(const s32 *block, s32 *const *outAt) {
    if ((block[1] | block[5] | block[3] | block[7] | block[2] | block[6] | block[4]) != 0) return false;
    const s32 value = block[0];
    s32 *const out = *outAt;
    for (int k = 0; k < 8; ++k) out[k * 9] = value;
    return true;
}

}  // namespace

extern "C" {

void __cdecl DctIdct_Column(const s32 *block) {
    if (FilledFlat(block, kDctOut)) return;
    Transform(block, kDctScratch, kDctConstants, kDctOut, 9, 0);
}

void __cdecl DctIdct_Row(const s32 *block) { Transform(block, kDctScratch, kDctConstants, kDctOut, 1, 0); }

void __cdecl DctIdct_Block(s32 *out, s32 pitch) {
    u32 *const pitchBytes = reinterpret_cast<u32 *>((uintptr_t)0x00fbc000);
    *pitchBytes = (u32)pitch << 2;
    for (u32 i = 0; i < 8; ++i) {
        *kDctOut = reinterpret_cast<s32 *>((uintptr_t)(0x00fbc02c + i * 4));
        DctIdct_Column(reinterpret_cast<const s32 *>((uintptr_t)(0x00fbc14c + i * 0x20)));
    }
    *kDctOut = out;
    DctIdct_Row(reinterpret_cast<const s32 *>((uintptr_t)0x00fbc02c));
    for (u32 i = 1; i < 8; ++i) {
        *kDctOut = reinterpret_cast<s32 *>((uintptr_t)((u32)(uintptr_t)*kDctOut + *pitchBytes));
        DctIdct_Row(reinterpret_cast<const s32 *>((uintptr_t)(0x00fbc02c + i * 0x24)));
    }
}

void __cdecl MovieIdct_Column(const s32 *block, s32 *out) {
    if (FilledFlat(block, &out)) return;
    u32 scratch[4];
    Transform(block, scratch, kMovieConstants, &out, 9, 0);
}

void __cdecl MovieIdct_Row(const s32 *block, s32 *out) {
    u32 scratch[4];
    Transform(block, scratch, kMovieConstants, &out, 1, 0);
}

void __cdecl MovieIdct_RowShift(const s32 *block, s32 *out) {
    u32 scratch[4];
    Transform(block, scratch, kMovieConstants, &out, 1, 17);
}
}
