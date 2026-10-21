#include "mmxblit.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

uint64_t &qword(u8 *p, u32 offset) { return *reinterpret_cast<uint64_t *>(p + offset); }

uint64_t load(const u8 *p) { return *reinterpret_cast<const uint64_t *>(p); }

// The MMX instructions, lane by lane.
uint64_t paddusb(uint64_t a, uint64_t b) {
    uint64_t out = 0;
    for (int k = 0; k < 64; k += 8) {
        const u32 sum = (u32)(a >> k & 0xff) + (u32)(b >> k & 0xff);
        out |= (uint64_t)(sum > 0xff ? 0xff : sum) << k;
    }
    return out;
}

uint64_t pmaddwd(uint64_t a, uint64_t b) {
    uint64_t out = 0;
    for (int k = 0; k < 64; k += 32) {
        const u32 sum = (u32)((s32)(s16)(a >> k) * (s16)(b >> k)) + (u32)((s32)(s16)(a >> (k + 16)) * (s16)(b >> (k + 16)));  // (wraps)
        out |= (uint64_t)sum << k;
    }
    return out;
}

uint64_t psrld(uint64_t a, uint64_t count) {
    if (count > 31) return 0;
    return (uint64_t)((u32)(a >> 32) >> count) << 32 | (u32)a >> count;
}

uint64_t psrlq(uint64_t a, uint64_t count) { return count > 63 ? 0 : a >> count; }

uint64_t punpckldq(uint64_t a, uint64_t b) { return b << 32 | (u32)a; }

uint64_t saturated(u32 v) {
    const s32 s = (s32)v;
    return (u16)(s > 0x7fff ? 0x7fff : s < -0x8000 ? -0x8000 : s);
}

uint64_t packssdw(uint64_t a, uint64_t b) {
    return saturated((u32)a) | saturated((u32)(a >> 32)) << 16 | saturated((u32)b) << 32 | saturated((u32)(b >> 32)) << 48;
}

// MmxBlit_Convert16's operands: a qword at base + index * scale + offset (32-bit wrapping, as the address modes compute it).
uint64_t &at64(u8 *base, s32 index, u32 scale, s32 offset) {
    return *reinterpret_cast<uint64_t *>((uintptr_t)base + (u32)index * scale + (u32)offset);
}

// ADD ECX,4; JL: the count moved on, and whether the true sum is below 0.
bool next_pass(s32 &count) {
    const int64_t sum = (int64_t)count + 4;
    count = (s32)(u32)sum;
    return sum < 0;
}

uint64_t weights() { return global<uint64_t>(0x00fbe190); }

u8 *moved(u8 *p, s32 by) { return reinterpret_cast<u8 *>((uintptr_t)p + (u32)by); }

}  // namespace

extern "C" {

void __cdecl MmxBlit_Copy16(const u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch) {
    const u32 groups = width >> 2;
    do {
        const u8 *from = source;
        u8 *to = out;
        u32 left = groups;
        do {
            const uint64_t a = *reinterpret_cast<const uint64_t *>(from), b = *reinterpret_cast<const uint64_t *>(from + 8);
            qword(to, 0) = a;
            qword(to, 8) = b;
            from += 0x10;
            to += 0x10;
        } while (--left);
        source += sourcePitch;
        out += outPitch;
    } while (--rows);
}

void __cdecl MmxBlit_Dither16(const u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch) {
    const u32 groups = width >> 2;
    const uint64_t low = global<uint64_t>(0x00fbe198), high = global<uint64_t>(0x00fbe1a0), shift = global<uint64_t>(0x00fbe1a8);
    u32 row = 0;
    do {
        global<uint64_t>(0x00fab4e0) = global<uint64_t>(0x0077e540 + (row & 3) * 16);
        global<uint64_t>(0x00fab4e8) = global<uint64_t>(0x0077e548 + (row & 3) * 16);
        const u8 *from = source;
        u8 *to = out;
        uint64_t a = load(from), b = load(from + 8);
        u32 left = groups;
        do {
            a = paddusb(a, global<uint64_t>(0x00fab4e0));
            b = paddusb(b, global<uint64_t>(0x00fab4e8));
            const uint64_t aHigh = psrld(a & high, shift), bHigh = psrld(b & high, shift);
            a = psrld(pmaddwd(a & low, global<uint64_t>(0x00fbe190)), 6) | aHigh;
            b = psrld(pmaddwd(b & low, global<uint64_t>(0x00fbe190)), 6) | bHigh;
            from += 0x10;
            to += 8;
            const uint64_t pairsA = psrlq(a, 16) | a, pairsB = psrlq(b, 16) | b;
            a = load(from);
            b = load(from + 8);
            *reinterpret_cast<uint64_t *>(to - 8) = punpckldq(pairsA, pairsB);
        } while (--left);
        source += sourcePitch;
        out += outPitch;
    } while (++row != rows);
}

void __cdecl MmxBlit_Convert16(u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch, u8 *depth, u32 depthPitch,
                               u32 clear, u32 fill) {
    global<u32>(0x00fab4d8) = 0u - width;
    global<u32>(0x00fab4fc) = rows;
    global<u32>(0x00fab4b4) = depthPitch;
    const bool clearing = (u8)clear != 0;
    if (clearing) {
        global<u32>(0x00fac51c) = fill;
        global<u32>(0x00fac518) = fill;
    }
    const uint64_t low = global<uint64_t>(0x00fbe198);
    const uint64_t blank = clearing ? global<uint64_t>(0x00fac518) : 0;
    const uint64_t high = global<uint64_t>(0x00fbe1a0);
    const uint64_t far = clearing ? global<uint64_t>(0x0077e580) : 0;
    const u32 back = global<u32>(0x00fab4d8);
    u8 *z = moved(depth, (s32)(0u - back * 2)), *d = moved(out, (s32)(0u - back * 2)), *s = moved(source, (s32)(0u - back * 4));
    const bool packed = global<u32>(0x00fbe058) == 5;
    u32 row = 0;
    do {
        const uint64_t d0 = global<uint64_t>(0x0077e540 + (row & 3) * 16), d1 = global<uint64_t>(0x0077e548 + (row & 3) * 16);
        s32 c = (s32)global<u32>(0x00fab4d8);
        uint64_t m0, m1, m2, m3;
        if (!clearing && packed) {
            m0 = paddusb(at64(s, c, 4, 0), d0);
            m2 = at64(s, c, 4, 8);
            c = (s32)((u32)c + 4);
            m2 = paddusb(m2, d1);
            m1 = m0 & high;
            m3 = m2;
            m0 = pmaddwd(m0 & low, weights());
            m2 = pmaddwd(m2 & low, weights());
            do {
                m3 &= high;
                m1 = psrld(m1 | m0, 6);
                m3 |= m2;
                m0 = at64(s, c, 4, 0);
                m3 = psrld(m3, 6);
                m2 = at64(s, c, 4, 8);
                m1 = packssdw(m1, m3);
                m0 = paddusb(m0, d0);
                m2 = paddusb(m2, d1);
                at64(d, c, 2, -8) = m1;
                m1 = m0 & high;
                m3 = m2;
                m0 = pmaddwd(m0 & low, weights());
                m2 = pmaddwd(m2 & low, weights());
            } while (next_pass(c));
            m3 &= high;
            m1 = psrld(m1 | m0, 6);
            m3 = psrld(m3 | m2, 6);
            m1 = packssdw(m1, m3);
            s = moved(s, sourcePitch);
            at64(d, 0, 0, -8) = m1;
            d = moved(d, outPitch);
        } else if (!clearing) {
            m0 = paddusb(at64(s, c, 4, 0), d0);
            m2 = paddusb(at64(s, c, 4, 8), d1);
            m1 = m0;
            do {
                m3 = m2;
                m0 = pmaddwd(m0 & low, weights());
                m2 = pmaddwd(m2 & low, weights());
                m1 = psrld(m1 & high, 5);
                m3 = psrld(m3 & high, 5);
                m0 = psrld(m0, 6) | m1;
                m2 = psrld(m2, 6) | m3;
                m1 = psrlq(m0, 16) | m0;
                m0 = at64(s, c, 4, 0x10);
                m3 = psrlq(m2, 16) | m2;
                m2 = at64(s, c, 4, 0x18);
                m1 = punpckldq(m1, m3);
                m0 = paddusb(m0, d0);
                m2 = paddusb(m2, d1);
                at64(d, c, 2, 0) = m1;
                m1 = m0;
            } while (next_pass(c));
            s = moved(s, sourcePitch);
            d = moved(d, outPitch);
        } else if (packed) {
            global<uint64_t>(0x00fab4e0) = d0;
            global<uint64_t>(0x00fab4e8) = d1;
            m0 = at64(s, c, 4, 0);
            m2 = at64(s, c, 4, 8);
            m0 = paddusb(m0, global<uint64_t>(0x00fab4e0));
            m2 = paddusb(m2, d1);
            m1 = m0;
            m0 &= low;
            at64(z, c, 2, 0) = far;
            c = (s32)((u32)c + 4);
            m3 = m2;
            m0 = pmaddwd(m0, weights());
            m2 = pmaddwd(m2 & low, weights());
            m1 &= high;
            do {
                at64(s, c, 4, -0x10) = blank;
                m3 &= high;
                at64(s, c, 4, -8) = blank;
                m1 = psrld(m1 | m0, 6);
                m3 |= m2;
                m0 = at64(s, c, 4, 0);
                m3 = psrld(m3, 6);
                m2 = at64(s, c, 4, 8);
                m1 = packssdw(m1, m3);
                m0 = paddusb(m0, global<uint64_t>(0x00fab4e0));
                at64(d, c, 2, -8) = m1;
                m1 = m0;
                m2 = paddusb(m2, global<uint64_t>(0x00fab4e8));
                m0 &= low;
                at64(z, c, 2, 0) = far;
                m3 = m2;
                m0 = pmaddwd(m0, weights());
                m2 = pmaddwd(m2 & low, weights());
                m1 &= high;
            } while (next_pass(c));
            at64(s, 0, 0, -0x10) = blank;
            m3 &= high;
            at64(s, 0, 0, -8) = blank;
            m1 = psrld(m1 | m0, 6);
            m3 = psrld(m3 | m2, 6);
            const u32 depthStep = global<u32>(0x00fab4b4);
            m1 = packssdw(m1, m3);
            s = moved(s, sourcePitch);
            z = moved(z, (s32)depthStep);
            at64(d, 0, 0, -8) = m1;
            d = moved(d, outPitch);
        } else {
            global<uint64_t>(0x00fab4e0) = d0;
            global<uint64_t>(0x00fab4e8) = d1;
            m0 = at64(s, c, 4, 0);
            m2 = at64(s, c, 4, 8);
            m0 = paddusb(m0, global<uint64_t>(0x00fab4e0));
            m1 = m0;
            do {
                m2 = paddusb(m2, global<uint64_t>(0x00fab4e8));
                m0 &= low;
                at64(z, c, 2, 0) = far;
                m3 = m2;
                m0 = pmaddwd(m0, weights());
                m2 = pmaddwd(m2 & low, weights());
                m1 = psrld(m1 & high, 5);
                m3 = psrld(m3 & high, 5);
                m0 = psrld(m0, 6) | m1;
                m2 = psrld(m2, 6) | m3;
                m1 = psrlq(m0, 16);
                at64(s, c, 4, 0) = blank;
                m1 |= m0;
                m0 = at64(s, c, 4, 0x10);
                m3 = psrlq(m2, 16);
                at64(s, c, 4, 8) = blank;
                m3 |= m2;
                m2 = at64(s, c, 4, 0x18);
                m1 = punpckldq(m1, m3);
                m0 = paddusb(m0, global<uint64_t>(0x00fab4e0));
                at64(d, c, 2, 0) = m1;
                m1 = m0;
            } while (next_pass(c));
            const u32 depthStep = global<u32>(0x00fab4b4);
            s = moved(s, sourcePitch);
            d = moved(d, outPitch);
            z = moved(z, (s32)depthStep);
        }
    } while (global<u32>(0x00fab4fc) != ++row);
}

void __cdecl MmxBlit_Copy32(u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch, u8 *depth, u32 depthPitch,
                            u32 clear, u32 fillLow, u32 fillHigh) {
    global<u32>(0x00fab484) = width;
    global<u32>(0x00fab4a0) = rows;
    global<u32>(0x00fab4b0) = depthPitch;
    if ((u8)clear == 0) {
        global<u32>(0x00fac51c) = fillLow;
        global<u32>(0x00fac518) = fillLow;
        u32 left = global<u32>(0x00fab4a0);
        do {
            s32 count = (s32)global<u32>(0x00fab484);
            u8 *from = source, *to = out;
            do {
                const uint64_t q0 = qword(from, 0), q1 = qword(from, 8), q2 = qword(from, 0x10), q3 = qword(from, 0x18);
                qword(to, 0) = q0;
                qword(to, 8) = q1;
                qword(to, 0x10) = q2;
                qword(to, 0x18) = q3;
                from += 0x20;
                to += 0x20;
                count -= 8;
            } while (count > 0);
            source += sourcePitch;
            out += outPitch;
        } while (--left);
        return;
    }
    const uint64_t depthValue = global<uint64_t>(0x0077e580), fill = (uint64_t)fillHigh << 32 | fillLow;
    u32 left = global<u32>(0x00fab4a0);
    do {
        s32 count = (s32)global<u32>(0x00fab484);
        u8 *from = source, *to = out, *z = depth;
        do {
            const uint64_t q0 = qword(from, 0), q1 = qword(from, 8), q2 = qword(from, 0x10), q3 = qword(from, 0x18);
            qword(from, 0) = fill;
            qword(from, 8) = fill;
            qword(from, 0x10) = fill;
            qword(from, 0x18) = fill;
            qword(to, 0) = q0;
            qword(to, 8) = q1;
            qword(to, 0x10) = q2;
            qword(to, 0x18) = q3;
            qword(z, 0) = depthValue;
            qword(z, 8) = depthValue;
            from += 0x20;
            to += 0x20;
            z += 0x10;
            count -= 8;
        } while (count > 0);
        source += sourcePitch;
        out += outPitch;
        depth += global<u32>(0x00fab4b0);
    } while (--left);
}
}
