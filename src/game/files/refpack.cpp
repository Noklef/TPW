// EA RefPack: reimplementation of the decoder (0x641f10, 0x642110, the video library's 0x67850c) and encoder (0x641a50, 0x6419e0,
// 0x641a20).
#include "refpack.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"

namespace {

inline u32 be24(const u8 *p) { return (u32)p[0] << 16 | (u32)p[1] << 8 | p[2]; }

// Byte by byte: a match may overlap the bytes it is producing.
inline void copy(u8 *&dst, const u8 *src, u32 n) {
    while (n--) *dst++ = *src++;
}

}  // namespace

extern "C" s32 __stdcall RefPack_Decode(u8 *dst, const u8 *src, s32 /*unused*/) {
    if (!src) return 0;
    const u8 *in = src + ((src[0] & 0x01) ? 5 : 2);
    s32 size = (s32)be24(in);
    in += 3;
    for (;;) {
        u8 b0 = in[0];
        if (!(b0 & 0x80)) {  // 2 bytes: 0-3 literals, then 3-10 bytes from up to 1 KB back
            u8 b1 = in[1];
            in += 2;
            copy(dst, in, b0 & 3);
            in += b0 & 3;
            copy(dst, dst - (((b0 & 0x60) << 3) + b1 + 1), ((b0 >> 2) & 7) + 3);
        } else if (!(b0 & 0x40)) {  // 3 bytes: 0-3 literals, then 4-67 bytes from up to 16 KB back
            u8 b1 = in[1], b2 = in[2];
            in += 3;
            copy(dst, in, b1 >> 6);
            in += b1 >> 6;
            copy(dst, dst - (((b1 & 0x3f) << 8) + b2 + 1), (b0 & 0x3f) + 4);
        } else if (!(b0 & 0x20)) {  // 4 bytes: 0-3 literals, then 5-1028 bytes from up to 128 KB back
            u8 b1 = in[1], b2 = in[2], b3 = in[3];
            in += 4;
            copy(dst, in, b0 & 3);
            in += b0 & 3;
            copy(dst, dst - (((b0 & 0x10) << 12) + (b1 << 8) + b2 + 1), ((b0 & 0x0c) << 6) + b3 + 5);
        } else {
            u32 n = (b0 & 0x1f) * 4 + 4;
            ++in;
            if (n > 0x70) {  // 0xfc-0xff: 0-3 last literals, end
                copy(dst, in, b0 & 3);
                return size;
            }
            copy(dst, in, n);  // 4-112 literals
            in += n;
        }
    }
}

extern "C" void __cdecl RefPack_DecodeIf(const u8 *src, u8 *dst, s32 decode) {
    if (src && decode) RefPack_Decode(dst, src, 0);
}

extern "C" u32 __cdecl ReadBigEndian(const u8 *p, u32 n) {
    u32 value = 0;
    while (n--) value = value << 8 | *p++;
    return value;
}

extern "C" u32 __stdcall RefPack_DecodedSize(const u8 *src) {
    return be24(src + (((u32)src[0] << 8 | src[1]) == 0x10fb ? 2 : 5));
}

extern "C" void __cdecl WriteBigEndian(u8 *dst, u32 value, u32 n) {
    u32 v = value << ((32 - 8 * n) & 31);  // (the CPU's shift count: n above 4 wraps, sic)
    for (; n; --n) {
        *dst++ = (u8)(v >> 24);
        v <<= 8;
    }
}

extern "C" u32 __cdecl RefPack_MatchLength(const u8 *a, const u8 *b, u32 max) {
    u32 n = 0;
    if (max)
        do {
            if (a[n] != b[n]) break;
            ++n;
        } while (n < max);
    return n;
}

namespace {

// REP MOVSD then REP MOVSB, as the original copies (forward, a dword at a time, then the rest).
void movs(u8 *&out, const u8 *in, u32 n) {
    for (u32 i = n >> 2; i; --i, in += 4, out += 4) *reinterpret_cast<u32 *>(out) = *reinterpret_cast<const u32 *>(in);
    for (u32 i = n & 3; i; --i) *out++ = *in++;
}

// The CRC-16 table at 0x779c60 (writable .data: read live) as the hash of the three bytes at p.
u32 hash3(const u8 *p) {
    const u32 *const crc = gCrc16Table;
    u32 h = crc[p[0]];
    h = crc[(h & 0xff) ^ p[1]] ^ (h >> 8);
    return crc[(h & 0xff) ^ p[2]] ^ (h >> 8);
}

// Literal runs of 4-112 bytes (0xe0-0xfb) until at most 3 are left.
void flush_literals(u8 *&out, const u8 *&lit, u32 &lits) {
    while (lits > 3) {
        u32 n = lits & ~3u;
        if (n > 0x70) n = 0x70;
        lits -= n;
        *out++ = (u8)((n >> 2) - 0x21);
        movs(out, lit, n);
        lit += n;
    }
}

}  // namespace

extern "C" u32 __fastcall RefPack_Encode(u8 *codec, void *, u8 *dst, const u8 *src, u32 len, s32 *work) {
    WriteBigEndian(dst, 0x10fb, 2);
    WriteBigEndian(dst + 2, len, 3);
    u8 *out = dst + 5;
    void *allocated = nullptr;
    if (!work) {
        allocated = GameHeap_Alloc(vcall<u32>(codec, 0xc));
        if (!allocated) return 0;
        work = static_cast<s32 *>(allocated);
    }
    s32 *const link = work + 0x10000;
    for (u32 i = 0; i < 0x10000; ++i) work[i] = -1;

    const u8 *cur = src, *lit = src;
    u32 lits = 0;
    for (u32 left = len; left;) {
        u32 best_len = 2, best_cost = 2, best_off = 0;
        const u32 max = left < 0x404 ? left : 0x404;
        const u32 h = hash3(cur);  // (two bytes past the input near its end, sic)
        const s32 pos = (s32)(cur - src);
        const s32 lowest = (s32)((u32)pos - 0x1ffff) > 0 ? (s32)((u32)pos - 0x1ffff) : 0;
        for (s32 cand = work[h]; cand >= lowest; cand = link[cand & 0x1ffff]) {
            const u8 *const m = src + cand;
            if (cur[best_len] != m[best_len]) continue;  // (also at best_len == max: a byte past the match, sic)
            const u32 n = RefPack_MatchLength(cur, m, max);
            if (n <= best_len) continue;
            const u32 off = (u32)(cur - m) - 1;
            u32 cost;
            if (off < 0x400 && n <= 10) cost = 2;
            else if (off < 0x4000 && n <= 0x43) cost = 3;
            else cost = 4;
            if (n - cost + 4 <= best_len - best_cost + 4) continue;
            best_len = n, best_cost = cost, best_off = off;
            if (n >= 0x404) break;
        }

        if (best_cost >= best_len) {  // no match worth a command: one more literal
            link[pos & 0x1ffff] = work[h];
            work[h] = pos;
            ++lits, ++cur, --left;
            continue;
        }
        flush_literals(out, lit, lits);
        u8 last = (u8)best_off;
        if (best_cost == 2) {
            *out++ = (u8)(((best_len + 0x3d) << 2) + ((best_off >> 3) & 0xe0) + lits);
        } else if (best_cost == 3) {
            *out++ = (u8)(best_len + 0x7c);
            *out++ = (u8)((best_off >> 8) + (lits << 6));
        } else {
            *out++ = (u8)((((best_len - 5) >> 6) & 0xfc) + ((best_off >> 12) & 0xf0) + lits - 0x40);
            *out++ = (u8)(best_off >> 8);
            *out++ = (u8)best_off;
            last = (u8)(best_len - 5);
        }
        *out++ = last;
        if (lits) {
            movs(out, lit, lits);
            lits = 0;
        }
        if ((s32)best_len > 0)
            for (u32 i = best_len; i; --i, ++cur) {
                const u32 hh = hash3(cur);
                const s32 p = (s32)(cur - src);
                link[p & 0x1ffff] = work[hh];
                work[hh] = p;
            }
        left -= best_len;
        lit = cur;
    }
    flush_literals(out, lit, lits);
    *out++ = (u8)(lits - 4);
    if (lits) movs(out, lit, lits);
    if (allocated) GameHeap_Free(allocated);
    return (u32)(out - dst) + 5;
}
