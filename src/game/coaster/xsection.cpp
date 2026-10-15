#include "xsection.h"

#include "../core/globalmem.h"

namespace {

template <typename T> T &at(const void *p, u32 off) {
    return *reinterpret_cast<T *>(reinterpret_cast<u8 *>(const_cast<void *>(p)) + off);
}

const u32 kBuckets = 20;

}  // namespace

extern "C" {

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

    const u8 *record = source + 0x33a4;
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
}
