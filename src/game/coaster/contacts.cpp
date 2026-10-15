#include "contacts.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u32 kOverflow = 0x400000, kLost = 0x200000;

u32 *contacts(u8 *node) { return &at<u32>(node, 0xb4); }
u32 &count(u8 *node) { return at<u32>(node, 0xb0); }

// Takes node out of from's list, closing it up.
void drop(u8 *owner, u8 *from, const u8 *node) {
    u32 *list = contacts(from), *kept = list;
    u32 dropped = 0;
    for (u32 i = 0; i < count(from); ++i) {
        if (list[i] != (u32)(uintptr_t)node)
            *kept++ = list[i];
        else
            ++dropped;  // its slot is overwritten or zeroed below (the original zeroes it here too)
    }
    const u32 n = (u32)((s32)((uintptr_t)kept - (uintptr_t)list) >> 2);
    for (u32 i = n; i < count(from); ++i) list[i] = 0;
    count(from) = n;
    at<u32>(owner, 0x140) -= dropped;
    if (dropped && (at<u32>(from, 8) & kOverflow)) at<u32>(from, 8) |= kLost;
}

}  // namespace

extern "C" {

void __cdecl Contacts_Clear(u8 *owner, u8 *node) {
    if (at<u32>(node, 8) & kOverflow) {
        for (u8 *other = at<u8 *>(owner, 0x54); other; other = at<u8 *>(other, 0x64)) drop(owner, other, node);
        for (int i = 0; i < 8; ++i) contacts(node)[i] = 0;
        at<u32>(owner, 0x140) -= count(node);
        count(node) = 0;
        at<u32>(node, 8) &= ~kOverflow;
        return;
    }
    for (u32 i = 0; i < count(node); ++i) {
        drop(owner, reinterpret_cast<u8 *>(contacts(node)[i]), node);
        contacts(node)[i] = 0;
    }
    at<u32>(owner, 0x140) -= count(node);
    count(node) = 0;
}

void __cdecl Contacts_ClearAll(u8 *owner) {
    for (u8 *node = at<u8 *>(owner, 0x54); node; node = at<u8 *>(node, 0x64)) {
        Contacts_Clear(owner, node);
        at<u32>(node, 8) |= kLost;
    }
}

void __cdecl Contacts_ClearPair(u8 *node, u8 *owner) {
    Contacts_Clear(owner, node);
    if (u8 *partner = at<u8 *>(node, 0x68)) Contacts_Clear(owner, partner);
}

void __cdecl Contacts_MarkLostPair(u8 *node) {
    at<u32>(node, 8) |= kLost;
    if (u8 *partner = at<u8 *>(node, 0x68)) at<u32>(partner, 8) |= kLost;
}

void __cdecl Contacts_UpdateBounds(u8 *owner) {
    const uintptr_t points = at<uintptr_t>(owner, 0x9c);
    for (u8 *node = at<u8 *>(owner, 0x54), *next; (next = at<u8 *>(node, 0x64)) != nullptr; node = next) {
        const u32 first = at<u32>(node, 0xfc);
        const u8 *point = reinterpret_cast<const u8 *>((u32)(points + first * 0x38)) + 4;
        u32 *min = &at<u32>(node, 0x98), *max = &at<u32>(node, 0xa4);
        for (int j = 0; j < 3; ++j) min[j] = max[j] = at<u32>(point, 4 * j);
        u32 rest = at<u32>(next, 0xfc) - first + 1;  // the points through the next node's
        if (rest) --rest;                              // (none past the first when that wraps to 0)
        for (; rest; --rest) {
            point += 0x38;
            for (int j = 0; j < 3; ++j) {
                const float v = at<float>(point, 4 * j);
                if (!(v >= reinterpret_cast<const float *>(min)[j]))
                    min[j] = at<u32>(point, 4 * j);
                else if (v > reinterpret_cast<const float *>(max)[j])
                    max[j] = at<u32>(point, 4 * j);
            }
        }
        const u8 *type = at<const u8 *>(at<const u8 *>(node, 0x100), 0x18);
        if (!type) type = at<const u8 *>(at<const u8 *>(next, 0x100), 0x18);
        const float pad = type ? at<float>(type, 0x58) : 0.0f;
        float *lo = reinterpret_cast<float *>(min), *hi = reinterpret_cast<float *>(max);
        for (int j = 0; j < 3; ++j) lo[j] = (float)(X87Reg(lo[j]) - pad);
        for (int j = 0; j < 3; ++j) hi[j] = (float)(X87Reg(pad) + hi[j]);
    }
}

void __cdecl Contacts_SegmentsInBox(const u8 *owner, const u8 *node, const u8 *next, const float *box, u8 *out,
                                    u32 *count) {
    const u8 *type = at<const u8 *>(at<const u8 *>(node, 0x100), 0x18);
    if (!type) type = at<const u8 *>(at<const u8 *>(next, 0x100), 0x18);
    const float pad = type ? at<float>(type, 0x58) : 0.0f;
    float lo[3], hi[3];
    for (int j = 0; j < 3; ++j) lo[j] = (float)(X87Reg(box[j]) - pad);
    for (int j = 0; j < 3; ++j) hi[j] = (float)(X87Reg(box[3 + j]) + pad);
    const auto inside = [&](const float *p) {
        for (int j = 0; j < 3; ++j)
            if (!(p[j] >= lo[j]) || p[j] > hi[j]) return false;
        return true;
    };
    const u32 first = at<u32>(node, 0xfc), n = at<u32>(next, 0xfc) - first;
    const u8 *point = reinterpret_cast<const u8 *>((u32)(at<u32>(owner, 0x9c) + first * 0x38)) + 4;
    u32 found = 0;
    for (u32 i = 0; i < n; ++i, point += 0x38) {
        const float *a = reinterpret_cast<const float *>(point), *b = reinterpret_cast<const float *>(point + 0x38);
        if (!inside(a) && !inside(b)) continue;
        at<u32>(out, 0) = i + at<u32>(node, 0xfc);
        if (!(a[0] >= b[0])) {
            at<float>(out, 4) = (float)(X87Reg(a[0]) - pad);
            at<float>(out, 0x10) = (float)(X87Reg(pad) + b[0]);
        } else {
            at<float>(out, 4) = (float)(X87Reg(b[0]) - pad);
            at<float>(out, 0x10) = (float)(X87Reg(a[0]) + pad);
        }
        for (int j = 1; j < 3; ++j) {
            // pick the ends by pointer: a value select may become minss, which mixes up -0 and +0
            const bool less = !(a[j] >= b[j]);
            const float *low = less ? a : b, *high = less ? b : a;
            at<float>(out, 4 + 4 * j) = (float)(X87Reg(low[j]) - pad);
            at<float>(out, 0x10 + 4 * j) = (float)(X87Reg(pad) + high[j]);
        }
        out += 0x1c;
        ++found;
    }
    *count = found;
}
}
