#include "clone.h"

#include <cstring>

#include "../core/globalmem.h"
#include "parts.h"

namespace {

// Addresses as the 32-bit game has them, so that the sums wrap as they do there.
template <typename T> T &at(u32 p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }
u32 align8(u32 v) { return (v + 7) & ~7u; }
void copy(u32 to, u32 from, u32 bytes) {  // rep movsd / movsb
    std::memcpy(reinterpret_cast<void *>(to), reinterpret_cast<const void *>(from), bytes);
}

// The +0x50 array (and flag 0x40): asked for, or the model's 0x1000, or its 8 while the
// procedural terrain textures could load.
bool wants_points(u32 flags, u32 model) {
    const u32 own = at<u32>(model, 0x30);
    return (flags & 0x200) || (own & 0x1000) || ((own & 8) && (at<u32>(0x007a1a8c, 0) & 0x2000));
}

}  // namespace

extern "C" {

u32 __cdecl Holder_Clone(const u8 *holder_, u32, u32 flags, u8 **out, u32 mask) {
    const u32 holder = (u32)(uintptr_t)holder_;
    const u32 model = at<u32>(holder, 4);
    if (wants_points(flags, model)) flags |= 0x40;
    if (flags & 4) {
        if (!(flags & 1)) flags |= 3;
        if (flags & 4) flags |= 0x100;
    }
    if (flags & 1) flags |= 2;  // (unused)

    // The size: each section rounded up to 8, the padding counted as well (sic: on top).
    u32 size = (flags & 1) ? at<u16>(model, 0x44) * 0x48u + at<u16>(model, 0x42) * 0x58u + 0xf8
                           : (at<u32>(at<u32>(model, 0x78), 0) & 0x200) ? 0x150 : 0x198;
    u32 pad = 0;
    const auto section = [&](u32 bytes) {
        const u32 end = size + bytes;
        size = align8(end);
        pad += size - end;
    };
    section(0);
    if (flags & 4) section(at<u16>(model, 0x38) * 0x30u);
    if (flags & 8) section(at<u16>(model, 0x4a) * 12u);
    if (flags & 0x20) section(at<u16>(model, 0x3e) * 8u);
    if (flags & 0x40) section(at<u16>(model, 0x3c) * 16u);
    u32 runs = 0;
    if (flags & 0x90) {
        if (flags & 0x80) {
            runs = at<u16>(model, 0x3a);
        } else {
            u32 node = at<u32>(model, 0x70);
            for (u32 n = at<u16>(model, 0x44); n; --n, node += 0xa0)
                if (at<u8>(node, 0) & 0x80) runs += (at<u16>(node, 0x5e) >> 2) + 1u;
        }
    }
    section(runs * 32);
    if (wants_points(flags, model)) size += at<u16>(model, 0x36) * 8u;
    if (at<u32>(model, 0x30) & 0x18000) {
        const u32 n = at<u16>(model, 0x36);
        u32 records = 2;
        if (at<u32>(model, 0x30) & 0x10000) {
            records = 0;
            u32 entry = at<u32>(model, 0x54);
            for (u32 k = n; k; --k, entry += 0x10)
                if (at<u8>(entry, 0) & 2) ++records;
        }
        size += n * 16 + records * 22;
    }
    if (at<u16>(model, 0x48))
        size += at<u32>(at<u32>(holder, 0x28), 0xc) * 64 + (at<u16>(model, 0x48) + 1u) * 20;

    const u32 raw = (u32)(uintptr_t)GlobalMem_Alloc(size + pad + 4, 0);
    const u32 block = align8(raw), c = block + 0x40;  // c: the model's copy
    u32 cursor = block + 0xf8;
    if (block && holder) copy(block, holder, 0x40);
    if (c && model) copy(c, model, 0xb8);
    at<u32>(block, 0x24) = raw;
    at<u32>(block, 8) = at<u32>(holder, 8) ? at<u32>(holder, 8) : holder;
    at<u32>(block, 4) = c;
    at<u32>(block, 0) &= ~1u;
    at<u32>(block, 0x1c) = 0;

    if (flags & 1) {
        if (at<u32>(model, 0x70)) {
            const u32 primary = at<u16>(c, 0x44);
            if (primary) {
                at<u32>(c, 0x70) = cursor;
                if (cursor && at<u32>(model, 0x70)) {
                    copy(cursor, at<u32>(model, 0x70), primary * 0xa0);
                    cursor += primary * 0xa0;
                }
            }
            const u32 secondary = at<u16>(c, 0x42) - primary;
            if (secondary) {
                at<u32>(c, 0x74) = cursor;
                if (cursor && at<u32>(model, 0x74)) {
                    copy(cursor, at<u32>(model, 0x74), secondary * 0x58);
                    cursor += secondary * 0x58;
                }
            }
            // The nodes' pointers (+4, +8, +0xc: into the node arrays) by how far they moved.
            const u32 to = at<u32>(c, 0x70), from = at<u32>(model, 0x70);
            const bool back = !(to > from);
            const u32 moved = back ? from - to : to - from;
            const auto move = [&](u32 &p) {
                if (p) p = back ? p - moved : p + moved;
            };
            for (u32 i = 0; i < at<u16>(c, 0x42); ++i) {
                const u32 first = at<u16>(c, 0x44);
                const u32 node = i < first ? at<u32>(c, 0x70) + i * 0xa0 : at<u32>(c, 0x74) + (i - first) * 0x58;
                move(at<u32>(node, 4));
                move(at<u32>(node, 8));
                move(at<u32>(node, 0xc));
                if (at<u32>(node, 0) & 0x4000) at<u32>(block, 0x2c) = node;
            }
            move(at<u32>(c, 0x78));
        }
    } else if (at<u32>(c, 0x78)) {  // only the root node
        at<u32>(c, 0x78) = cursor;
        const u32 root = at<u32>(model, 0x78);
        const u32 bytes = (at<u32>(root, 0) & 0x200) ? 0x58 : 0xa0;
        if (cursor && root) {
            copy(cursor, root, bytes);
            cursor += bytes;
        }
    }

    u32 p = align8(cursor);
    if (flags & 8) {
        at<u32>(c, 0x5c) = p;
        if (p && at<u32>(model, 0x5c) && at<u16>(c, 0x4a)) {
            copy(p, at<u32>(model, 0x5c), at<u16>(c, 0x4a) * 12u);
            p += at<u16>(c, 0x4a) * 12u;
        }
        p = align8(p);
    }
    if ((flags & 4) && (flags & 0x100) && at<u32>(model, 0x58)) {
        at<u32>(c, 0x58) = p;
        if (p && at<u32>(model, 0x58) && at<u16>(c, 0x38)) {
            copy(p, at<u32>(model, 0x58), at<u16>(c, 0x38) * 0x30u);
            p += at<u16>(c, 0x38) * 0x30u;
        }
        u32 node = at<u32>(c, 0x70), src = at<u32>(model, 0x70);
        p = align8(p);
        for (u32 n = at<u16>(c, 0x44); n; --n, node += 0xa0, src += 0xa0) {
            if (at<u32>(node, 0x60))
                at<u32>(node, 0x60) = (u32)((s32)(at<u32>(src, 0x60) - at<u32>(model, 0x58)) / 0x30) * 0x30 + at<u32>(c, 0x58);
            if (at<u32>(node, 0x64))
                at<u32>(node, 0x64) = at<u32>(c, 0x5c) + (u32)((s32)(at<u32>(src, 0x64) - at<u32>(model, 0x5c)) / 12) * 12;
        }
    }
    if (flags & 0x20) {
        at<u32>(c, 0x68) = p;
        if (p && at<u32>(model, 0x68) && at<u16>(c, 0x3e)) {
            copy(p, at<u32>(model, 0x68), at<u16>(c, 0x3e) * 8u);
            p += at<u16>(c, 0x3e) * 8u;
        }
        const u32 node = at<u32>(c, 0x70);
        p = align8(p);
        for (u32 n = at<u16>(model, 0x44); n; --n) at<u32>(node, 0x70) = at<u32>(c, 0x68);  // sic: the first node each time
    }
    if (flags & 0x40) {
        at<u32>(c, 0x64) = p;
        if (p && at<u32>(model, 0x64) && at<u16>(c, 0x3c)) {
            copy(p, at<u32>(model, 0x64), at<u16>(c, 0x3c) * 16u);
            p += at<u16>(c, 0x3c) * 16u;
        }
        u32 src = at<u32>(model, 0x70), node = at<u32>(c, 0x70);
        p = align8(p);
        for (u32 n = at<u16>(model, 0x44); n; --n, src += 0xa0, node += 0xa0)  // sar 4, shl 4
            at<u32>(node, 0x6c) = ((at<u32>(src, 0x6c) - at<u32>(model, 0x64)) & ~0xfu) + at<u32>(c, 0x64);
    }
    if (flags & 0x90) {
        if (flags & 0x80) {
            at<u32>(c, 0x60) = p;
            if (p && at<u32>(model, 0x60) && at<u16>(c, 0x3a)) {
                copy(p, at<u32>(model, 0x60), at<u16>(c, 0x3a) * 32u);
                p += at<u16>(c, 0x3a) * 32u;
            }
            u32 node = at<u32>(c, 0x70);
            p = align8(p);
            for (u32 n = at<u16>(model, 0x44); n; --n, node += 0xa0) at<u32>(node, 0x68) = at<u32>(c, 0x60);
        } else {  // each flagged node's own run
            u32 src = at<u32>(model, 0x70), node = at<u32>(c, 0x70);
            for (u32 n = at<u16>(model, 0x44); n; --n, src += 0xa0, node += 0xa0) {
                if (!(at<u8>(src, 0) & 0x80)) continue;
                const u32 bytes = ((at<u16>(src, 0x5e) >> 2) + 1u) * 32;
                at<u32>(node, 0x68) = p;  // before the source is read: a shared node array reads it back
                if (p && at<u32>(src, 0x68)) {
                    copy(p, at<u32>(src, 0x68), bytes);
                    p += bytes;
                }
            }
        }
    }
    if (wants_points(flags, model)) {
        at<u32>(c, 0x50) = p;
        if (p && at<u32>(model, 0x50) && at<u16>(model, 0x36)) {
            copy(p, at<u32>(model, 0x50), at<u16>(model, 0x36) * 8u);
            p += at<u16>(model, 0x36) * 8u;
        }
        u32 node = at<u32>(c, 0x70);
        for (u32 n = at<u16>(model, 0x44); n; --n, node += 0xa0) {
            u32 entry = at<u32>(node, 0x6c);  // 16-byte entries, +0 into the +0x50 array
            for (u32 k = at<u16>(node, 0x5a); k; --k, entry += 0x10)
                if (at<u32>(entry, 0)) at<u32>(entry, 0) = at<u32>(c, 0x50) + ((at<u32>(entry, 0) - at<u32>(model, 0x50)) & ~7u);
        }
    }
    if (at<u32>(model, 0x30) & 0x18000) {
        at<u32>(c, 0x54) = p;
        if (p && at<u32>(model, 0x54) && at<u16>(model, 0x36)) {
            copy(p, at<u32>(model, 0x54), at<u16>(model, 0x36) * 16u);
            p += at<u16>(model, 0x36) * 16u;
        }
        u32 to = at<u32>(c, 0x54) + 0xc, from = at<u32>(model, 0x54);
        for (u32 i = 0; i < at<u16>(model, 0x36); ++i, to += 0x10, from += 0x10) {
            if (!(at<u8>(from, 0) & 2)) continue;
            at<u32>(to, 0) = p;
            if (p && at<u32>(from, 0xc)) {
                copy(p, at<u32>(from, 0xc), 0x16);
                p += 0x16;
            }
            at<u32>(at<u32>(to, 0), 0) &= ~2u;
        }
    }
    if (at<u16>(model, 0x48)) {  // the slot table: header, slots (0x14), then 0x40 per used slot
        at<u32>(block, 0x28) = p;
        if (p && at<u32>(holder, 0x28)) {
            copy(p, at<u32>(holder, 0x28), 0x14);
            p += 0x14;
        }
        at<u32>(at<u32>(block, 0x28), 4) = p;
        if (p && at<u32>(at<u32>(holder, 0x28), 4) && at<u16>(model, 0x48)) {
            copy(p, at<u32>(at<u32>(holder, 0x28), 4), at<u16>(model, 0x48) * 20u);
            p += at<u16>(model, 0x48) * 20u;
        }
        if (at<u32>(at<u32>(holder, 0x28), 0xc)) {
            at<u32>(at<u32>(block, 0x28), 0x10) = p;
            const u32 table = at<u32>(holder, 0x28);
            if (p && at<u32>(table, 0x10) && at<u32>(table, 0xc)) copy(p, at<u32>(table, 0x10), at<u32>(table, 0xc) << 6);
            const u32 own = at<u32>(block, 0x28);
            u32 slot = at<u32>(own, 4), data = at<u32>(own, 0x10);
            for (u32 n = at<u16>(at<u32>(block, 4), 0x48); n; --n, slot += 0x14) {
                if (at<u8>(slot, 0) & 1) {
                    at<u32>(slot, 4) = data;
                    data += 0x40;
                }
            }
        }
    }

    *out = reinterpret_cast<u8 *>(block);
    const u8 *copied = reinterpret_cast<const u8 *>(block);
    if (flags & 0x1000) Model_MarkFacesNotValue(&copied, mask);
    else Model_MarkFacesByMask(&copied, mask);
    return 1;
}
}
