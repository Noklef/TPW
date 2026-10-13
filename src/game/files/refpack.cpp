// EA RefPack: reimplementation of the decoder (0x641f10, 0x642110) and encoder (0x641a50).
#include "refpack.h"

#include <vector>

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

extern "C" u32 __stdcall RefPack_DecodedSize(const u8 *src) {
    return be24(src + (((u32)src[0] << 8 | src[1]) == 0x10fb ? 2 : 5));
}

// The encoder: greedy LZ77 over a 128 KB window, candidates found through 3-byte hash
// chains (head table of 64K entries, links by position & 0x1ffff: the 0xc0000-byte
// workspace that vtable slot 3 reports).
namespace {

// The CRC-16 (reflected poly 0xA001) table at 0x779c60, used as the 3-byte hash.
struct Crc16Table {
    u32 t[256];
    Crc16Table() {
        for (u32 i = 0; i < 256; ++i) {
            u32 c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? (c >> 1) ^ 0xa001 : c >> 1;
            t[i] = c;
        }
    }
};

// Bytes past the end read as 0. The original reads whatever follows the input, but those
// hashes are only taken with fewer than 3 bytes left, when no match can be chosen.
inline u32 hash3(const u8 *src, u32 pos, u32 len) {
    static const Crc16Table crc;
    u32 b1 = pos + 1 < len ? src[pos + 1] : 0, b2 = pos + 2 < len ? src[pos + 2] : 0;
    u32 h = crc.t[src[pos]];
    h = crc.t[(h & 0xff) ^ b1] ^ (h >> 8);
    return crc.t[(h & 0xff) ^ b2] ^ (h >> 8);
}

// Literal runs of 4-112 bytes (0xe0-0xfb) until at most 3 are left for the next command.
u8 *flush_literals(u8 *out, const u8 *&lit, u32 &count) {
    while (count > 3) {
        u32 n = count & ~3u;
        if (n > 0x70) n = 0x70;
        *out++ = (u8)(0xe0 + (n >> 2) - 1);
        copy(out, lit, n);
        lit += n;
        count -= n;
    }
    return out;
}

}  // namespace

u32 RefPack_Encode(u8 *dst, const u8 *src, u32 len) {
    std::vector<s32> head(0x10000, -1), link(0x20000);
    u8 *out = dst;
    *out++ = 0x10;
    *out++ = 0xfb;
    *out++ = (u8)(len >> 16);
    *out++ = (u8)(len >> 8);
    *out++ = (u8)len;

    auto insert = [&](u32 pos) {
        u32 h = hash3(src, pos, len);
        link[pos & 0x1ffff] = head[h];
        head[h] = (s32)pos;
    };

    const u8 *lit = src;  // first pending literal
    u32 lits = 0, pos = 0;
    while (pos < len) {
        u32 remaining = len - pos, max = remaining < 0x404 ? remaining : 0x404;
        u32 best_len = 2, best_cost = 2, best_off = 0;
        s32 lowest = pos > 0x1ffff ? (s32)(pos - 0x1ffff) : 0;
        const u8 *cur = src + pos;
        for (s32 cand = head[hash3(src, pos, len)]; cand >= lowest; cand = link[cand & 0x1ffff]) {
            const u8 *m = src + cand;
            if (best_len >= max || cur[best_len] != m[best_len]) continue;
            u32 n = 0;
            while (n < max && cur[n] == m[n]) ++n;
            if (n <= best_len) continue;
            u32 off = (u32)(cur - m) - 1, cost;
            if (off < 0x400 && n <= 10) cost = 2;
            else if (off < 0x4000 && n <= 0x43) cost = 3;
            else cost = 4;
            if (n - cost + 4 > best_len - best_cost + 4) {  // more bytes saved
                best_len = n, best_cost = cost, best_off = off;
                if (n >= 0x404) break;
            }
        }

        if (best_cost >= best_len) {  // no match worth a command: one more literal
            insert(pos);
            ++lits, ++pos;
            continue;
        }
        out = flush_literals(out, lit, lits);
        switch (best_cost) {
        case 2:
            *out++ = (u8)(((best_len - 3) << 2) + ((best_off >> 3) & 0x60) + lits);
            break;
        case 3:
            *out++ = (u8)(0x80 + best_len - 4);
            *out++ = (u8)((best_off >> 8) + (lits << 6));
            break;
        default:
            *out++ = (u8)(0xc0 + (((best_len - 5) >> 8) << 2) + ((best_off >> 12) & 0x10) + lits);
            *out++ = (u8)(best_off >> 8);
            *out++ = (u8)best_off;
            best_off = best_len - 5;  // the last byte below is the length's low byte
            break;
        }
        *out++ = (u8)best_off;
        copy(out, lit, lits);
        lits = 0;
        for (u32 i = 0; i < best_len; ++i) insert(pos + i);
        pos += best_len;
        lit = src + pos;
    }
    out = flush_literals(out, lit, lits);
    *out++ = (u8)(0xfc + lits);
    copy(out, lit, lits);
    return (u32)(out - dst);
}
