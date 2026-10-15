#include "xsection.h"

#include <vector>

#include "../core/globalmem.h"
#include "../math/sintable.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 off) {
    return *reinterpret_cast<T *>(reinterpret_cast<u8 *>(const_cast<void *>(p)) + off);
}

template <typename T = u32> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

const u32 kBuckets = 20;

}  // namespace

extern "C" {

u32 __cdecl CrossSection_Setup(u8 *coaster, const u8 *source) {
    const u32 n = at<u32>(source, 0x3530);
    if (!n) return 0;
    u8 *const sections = static_cast<u8 *>(GlobalMem_Alloc((n + 1) * 0x18, 1));
    at<u8 *>(coaster, 0x88) = sections;
    at<u32>(coaster, 0x8c) = n;
    u32 shared;
    if (!CrossSection_BuildPoints(coaster, sections, source, &shared)) return 0;
    if (!CrossSection_BuildIndices(coaster, sections, source)) return 0;
    const u32 number = at<u32>(source, 0x3b00);
    const u8 *const from = sections + (number ? number - 1 : 0) * 0x18;
    const u32 count = at<u32>(from, 4);
    u8 *const extra = sections + n * 0x18;
    for (u32 k = 0; k < 0x18; k += 4) at<u32>(extra, k) = at<u32>(from, k);
    u8 *const entries = static_cast<u8 *>(GlobalMem_Alloc(count * 12, 1));
    at<u8 *>(extra, 0xc) = entries;
    if (entries && at<u8 *>(from, 0xc) && count) {
        const u8 *old = at<u8 *>(from, 0xc);
        for (u32 k = 0; k < count * 12; ++k) entries[k] = old[k];
    }
    if (!CrossSection_BuildPieces(coaster, sections, n, source, shared)) return 0;
    CrossSection_DropUnusedPoints(coaster, sections, n);
    return 1;
}

u32 __cdecl CrossSection_BuildPieces(u8 *coaster, u8 *sections, u32 n, const u8 *source, u32 flag) {
    u8 *link = nullptr;
    if (at<u32>(source, 0x3b04)) {
        link = sections + at<u32>(source, 0x3b04) * 0x18 - 0x18;
        at<u32>(coaster, 0) |= 0x40;
    }
    const u32 count = at<u32>(source, 0x38a4);
    if (!count) return 0;
    u8 *const pieces = static_cast<u8 *>(GlobalMem_Alloc(count * 0x2c, 1));
    if (flag) at<u32>(coaster, 0x74) |= 1;
    at<u32>(coaster, 0x7c) = count;
    at<u8 *>(coaster, 0x78) = pieces;
    at<u8 *>(coaster, 0x84) = link;
    at<u8 *>(coaster, 0x80) = sections + n * 0x18;
    u8 *piece = pieces;
    for (u32 i = count; i; --i, piece += 0x2c) {
        for (u32 k = 0; k < 0x20; k += 8) {
            at<u32>(piece, k) = 0xff7fffff;  // -FLT_MAX
            at<u32>(piece, k + 4) = 0x7f7fffff;
        }
        at<u32>(piece, 0x20) = 0;
    }

    const float kRadians = 0.0174532924f;  // pi / 180 (0x6fde78: 0x3c8efa35)
    const auto zero = [](float x) { return !(x < 0.0f) && !(x > 0.0f); };  // fcom: NaN is equal
    const u8 *from = source + 0x3534;
    piece = pieces;
    for (u32 i = 0; i < count; ++i, from += 0x2c, piece += 0x2c) {
        for (u32 k = 0; k < 0x20; k += 8) {
            const float lo = at<float>(from, k), hi = at<float>(from, k + 4);
            if (zero(lo) && zero(hi)) continue;
            at<float>(piece, k) = (float)(X87Reg(lo) * kRadians);
            at<float>(piece, k + 4) = (float)(X87Reg(hi) * kRadians);
        }
        for (u32 k = 0; k < 3; ++k) {
            const u32 number = at<u32>(from, 0x20 + 4 * k);
            if (!number) {
                if (k == 0) return 0;
                continue;
            }
            if (number > n) return 0;
            at<u8 *>(piece, 0x20 + 4 * k) = sections + number * 0x18 - 0x18;
        }
    }
    return 1;
}

void __cdecl CrossSection_PackPoints(s32 *used, u8 *section, u8 *to) {
    const u8 *from = at<u8 *>(section, 8);
    s32 next = 0;
    for (u32 i = 0; i < at<u32>(section, 0); ++i, from += 20) {
        if (!used[i]) continue;
        if (to != from)
            for (u32 k = 0; k < 20; k += 4) at<u32>(to, k) = at<u32>(from, k);
        to += 20;
        used[i] = next++;
    }
    u8 *entry = at<u8 *>(section, 0xc);
    if (at<u32>(entry, 0) & 1) return;
    for (u32 i = 0; i < at<u32>(section, 4); ++i, entry += 12) {
        at<u16>(entry, 6) = (u16)used[at<u16>(entry, 6)];
        at<u16>(entry, 8) = (u16)used[at<u16>(entry, 8)];
        at<u32>(entry, 0) |= 1;
    }
}

void __cdecl CrossSection_DropUnusedPoints(u8 *coaster, u8 *sections, u32 n) {
    std::vector<s32> used(0x10000);  // the original's 20 entries, widened: see xsection.h
    u32 total = 0;
    u8 *block = nullptr;
    for (u32 pass = 0;; ++pass) {
        if (pass == 1) {
            block = static_cast<u8 *>(GlobalMem_Alloc(total * 20, 1));
            at<u8 *>(coaster, 0x98) = block;
        }
        u8 *section = sections;
        for (u32 s = 0; s < n; ++s, section += 0x18) {
            const u32 points = at<u32>(section, 0);
            for (u32 i = 0; i < points && i < used.size(); ++i) used[i] = 0;
            const u8 *entry = at<u8 *>(section, 0xc);
            for (u32 i = at<u32>(section, 4); i; --i, entry += 12) {
                used[at<u16>(entry, 6)] = 1;
                used[at<u16>(entry, 8)] = 1;
            }
            u32 count = 0;
            for (u32 i = 0; i < points && i < used.size(); ++i) count += (u32)used[i];
            if (count >= points || at<u32>(section, 0x10)) continue;
            bool shared = false;
            const u8 *other = sections;
            for (u32 j = 0; j < n; ++j, other += 0x18) {
                if (j != s && at<u32>(section, 8) == at<u32>(other, 8)) {
                    shared = true;
                    break;
                }
            }
            if (!shared) {
                CrossSection_PackPoints(used.data(), section, at<u8 *>(section, 8));
            } else if (pass == 0) {
                total += count;
            } else {
                CrossSection_PackPoints(used.data(), section, block);
                at<u8 *>(section, 8) = block;
                at<u32>(section, 0) = count;
                block += count * 20;
            }
        }
        if (!total) {
            at<u8 *>(coaster, 0x98) = nullptr;
            return;
        }
        if (pass == 1) return;
    }
}

u32 __cdecl CrossSection_BuildPoints(u8 *coaster, u8 *sections, const u8 *source, u32 *shared) {
    *shared = 0;
    const auto count = [source](u32 b) { return at<s32>(source, 0x290 + 0x194 * b); };
    u32 total = 0;
    for (u32 b = 0; b < kBuckets; ++b) total += (u32)count(b);
    u32 used = kBuckets;  // the last non-empty bucket, from 1
    while (used && !count(used - 1)) --used;
    if (!used || !total) return 0;

    u8 *point = static_cast<u8 *>(GlobalMem_Alloc(total * 20, 1));
    u8 **const pointers = static_cast<u8 **>(GlobalMem_Alloc(used << 2, 1));
    s32 *const counts = static_cast<s32 *>(GlobalMem_Alloc(used << 2, 1));
    at<u8 *>(coaster, 0x90) = point;
    u8 **pointer = pointers;
    s32 *n = counts;
    for (u32 b = 0; b < kBuckets; ++b) {
        if (count(b)) {
            *pointer++ = point;
            *n++ = count(b);
        }
        const u8 *from = source + 0x100 + 0x194 * b;
        for (s32 i = 0; i < count(b); ++i, from += 20, point += 20) {
            for (u32 k = 0; k < 20; k += 4) at<u32>(point, k) = at<u32>(from, k);
            at<float>(point, 4) = (float)(X87Reg(at<float>(coaster, 0x40)) + at<float>(point, 4));
        }
    }

    const u8 *record = source + 0x33a0;
    u8 *section = sections;
    for (s32 k = 0; k < at<s32>(source, 0x3530); ++k, record += 0x14, section += 0x18) {
        const s32 bucket = at<s32>(record, 0);
        if (!bucket) return 0;  // sic: the lists leak
        at<u8 *>(section, 8) = pointers[bucket - 1];
        at<s32>(section, 0) = counts[bucket - 1];
        const s32 second = at<s32>(record, 0xc);
        if (second) {
            *shared = 1;
            if (counts[second - 1] != at<s32>(section, 0)) return 0;  // sic: the lists leak
            at<u8 *>(section, 0x10) = pointers[second - 1];
            at<u32>(section, 0x14) = at<u32>(record, 0x10);
        }
    }
    GlobalMem_Free(counts);
    GlobalMem_Free(pointers);
    return 1;
}

u32 __cdecl CrossSection_BuildIndices(u8 *coaster, u8 *sections, const u8 *source) {
    const auto count = [source](u32 b) { return at<s32>(source, 0x2180 + 0xf4 * b); };
    u32 total = 0;
    for (u32 b = 0; b < kBuckets; ++b) total += (u32)count(b);
    u32 used = kBuckets;  // the last non-empty bucket, from 1
    while (used && !count(used - 1)) --used;
    if (!used || !total) return 0;

    u8 *entry = static_cast<u8 *>(GlobalMem_Alloc(total * 12, 1));
    u8 **const pointers = static_cast<u8 **>(GlobalMem_Alloc(used << 2, 1));
    s32 *const counts = static_cast<s32 *>(GlobalMem_Alloc(used << 2, 1));
    at<u8 *>(coaster, 0x94) = entry;
    u8 **pointer = pointers;
    s32 *n = counts;
    for (u32 b = 0; b < kBuckets; ++b) {
        if (count(b)) {
            *pointer++ = entry;
            *n++ = count(b);
        }
        const u8 *from = source + 0x2090 + 0xf4 * b;
        for (s32 i = 0; i < count(b); ++i, from += 12, entry += 12) {
            at<u16>(entry, 4) = at<u16>(from, 0);
            at<u16>(entry, 6) = at<u16>(from, 4);
            at<u16>(entry, 8) = at<u16>(from, 8);
        }
    }

    const u8 *record = source + 0x33a4;  // the records' indices bucket
    u8 *section = sections;
    for (s32 k = 0; k < at<s32>(source, 0x3530); ++k, record += 0x14, section += 0x18) {
        const s32 bucket = at<s32>(record, 0);
        if (!bucket) return 0;  // sic: the lists leak
        at<u8 *>(section, 0xc) = pointers[bucket - 1];
        at<s32>(section, 4) = counts[bucket - 1];
    }
    GlobalMem_Free(counts);
    GlobalMem_Free(pointers);
    return 1;
}

void __cdecl CrossSection_Emit(float blend, float yaw, float pitch, float roll, const float *offset, float *min,
                               float *max, u32 separate, const u8 *section, const float *key) {
    const float kQuarter = -1.57079637f;  // -pi / 2 (0x6fe138): sin(a - kQuarter) = cos(a)
    const s32 n = at<s32>(section, 0);
    const u8 *points = at<u8 *>(section, 8);
    u32 vertex = global(0x0079c600);
    std::vector<float> blended;
    if (const u8 *const second = at<u8 *>(section, 0x10)) {
        X87Reg f = (X87Reg(blend) - key[-3]) / (X87Reg(key[1]) - key[-3]) * key[2];
        if (at<float>(section, 0x14) > 0.0f) f = f + (X87Reg(1.0f) - key[2]);
        if (!(f.v > 0.0)) f = 0.0;  // NaN too
        if (!(f.v < 1.0)) f = 1.0;
        const float rest = (float)(X87Reg(1.0f) - f);
        if (n > 0) {  // see the header
            blended.resize(n * 5);
            for (s32 i = 0; i < n * 5; ++i)
                blended[i] = (float)(X87Reg(at<float>(points, 4 * i)) * rest + X87Reg(at<float>(second, 4 * i)) * f);
            points = reinterpret_cast<const u8 *>(blended.data());
        }
    }
    if (n <= 0) return;

    const auto sine = [](float angle) {
        const SinTable table = SinTable_Game();
        return SinTable_At(table, (float)(X87Reg(table.scale) * angle));
    };
    const float rollCos = (float)(X87Reg(roll) - kQuarter), pitchCos = (float)(X87Reg(pitch) - kQuarter),
                yawCos = (float)(X87Reg(yaw) - kQuarter);
    for (s32 i = 0; i < n; ++i, points += 20, ++vertex) {
        const float px = at<float>(points, 0), py = at<float>(points, 4);
        const float qx = at<float>(points, 0xc), qy = at<float>(points, 0x10);
        // The position: rolled, pitched, yawed.
        const float s3 = sine(roll), c3 = sine(rollCos);
        const float x1 = (float)(X87Reg(c3) * px - X87Reg(s3) * py);
        const float s2 = sine(pitch), c2 = sine(pitchCos);
        const X87Reg y1 = X87Reg(c3) * py + X87Reg(px) * s3;
        const float y1f = (float)y1, z1 = (float)(y1 * s2);
        const float s1 = sine(yaw), c1 = sine(yawCos);
        const float x2 = (float)(X87Reg(s1) * z1 + X87Reg(c1) * x1);
        // The normal, the same way.
        const float s3n = sine(roll), c3n = sine(rollCos);
        const float nx1 = (float)(X87Reg(c3n) * qx - X87Reg(s3n) * qy);
        const float s2n = sine(pitch), c2n = sine(pitchCos);
        const X87Reg ny1 = X87Reg(c3n) * qy + X87Reg(qx) * s3n;
        const float ny1f = (float)ny1, ny2 = (float)(ny1 * c2n), nz1 = (float)(X87Reg(ny1f) * s2n);
        const float s1n = sine(yaw), c1n = sine(yawCos);
        const float nx2 = (float)(X87Reg(c1n) * nx1 + X87Reg(s1n) * nz1);
        const float nz2 = (float)(X87Reg(c1n) * nz1 - X87Reg(nx1) * s1n);
        const float length = (float)X87_Sqrt(X87Reg(nx2) * nx2 + X87Reg(ny2) * ny2 + X87Reg(nz2) * nz2);
        const X87Reg nx = X87Reg(nx2) / length;
        const float ny = (float)(X87Reg(ny2) / length), nz = (float)(X87Reg(nz2) / length);

        const X87Reg x = X87Reg(offset[0]) + x2;
        const X87Reg y = X87Reg(y1f) * c2 + offset[1];
        const float yf = (float)y;
        const float z = (float)(X87Reg(c1) * z1 - X87Reg(x1) * s1 + offset[2]);
        // fcom: "less" includes NaN, "greater" does not.
        if (separate) {
            if (!(x.v >= min[0])) min[0] = (float)x;
            if (x.v > max[0]) max[0] = (float)x;
            if (!(y.v >= min[1])) min[1] = yf;
            if (y.v > max[1]) max[1] = yf;
            if (!(z >= min[2])) min[2] = z;
            if (z > max[2]) max[2] = z;
        } else {
            if (!(x.v >= min[0])) min[0] = (float)x;
            else if (x.v > max[0]) max[0] = (float)x;
            if (!(y.v >= min[1])) min[1] = yf;
            else if (y.v > max[1]) max[1] = yf;
            if (!(z >= min[2])) min[2] = z;
            else if (z > max[2]) max[2] = z;
        }

        u8 *const mesh = at<u8 *>(global<u8 *>(0x00792218), 0x38);
        float *const position = at<float *>(mesh, 0x10) + (vertex >> 2) * 12 + (vertex & 3);
        position[0] = (float)x;
        position[4] = yf;
        position[8] = z;
        float *const normal = at<float *>(mesh, 0x14) + vertex * 3;
        normal[0] = (float)nx;
        normal[1] = ny;
        normal[2] = nz;
        float *const uv = at<float *>(mesh, 0x18) + (vertex >> 2) * 8 + (vertex & 3);
        uv[0] = X87_Through(at<float>(points, 8));
        uv[4] = X87_Through(at<float>(global<u8 *>(0x00792218), 0x74));
    }
}
}
