#include "remap.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/globalmem.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u32 length(const char *s) {   // (inline repne scasb)
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}

}  // namespace

extern "C" {

void __cdecl Model_RemapTextures(u8 *model, const s32 *remap, u32 textures, s32 first) {
    auto renumber = [&](s32 v) { return v > 0 ? (u32)first + (u32)v - 1 : ~(u32)v; };  // -1 - v
    u8 *mesh = at<u8 *>(model, 0x70);
    for (u32 i = 0; i < at<u16>(model, 0x44); ++i, mesh += 0xa0) {
        u8 *face = at<u8 *>(mesh, 0x6c);
        for (u32 j = 0; j < at<u16>(mesh, 0x5a); ++j, face += 0x10) {
            const u32 texture = at<u32>(face, 0);
            if (texture) at<u32>(face, 0) = textures + 8 * renumber(remap[(texture - at<u32>(model, 0x50)) >> 3]);
        }
    }
    if (!at<const u8 *>(model, 0x6c)) return;
    for (u32 i = 0; i < at<u16>(at<const u8 *>(model, 0x6c), 6); ++i) {
        u16 &cell = at<u16>(at<const u8 *>(at<const u8 *>(model, 0x6c), 0x2c), 4 * i + 2);
        cell = (u16)renumber(remap[cell]);
    }
}

u32 __cdecl Model_AddTerrainSets(u8 *model, u8 *sets, const char *dir) {
    at<u32>(sets, 0x28) = 0;
    // Each of the model's textures (by name, +0xc of its 0x10 bytes at +0x54) looked up in the sets' tiles in turn:
    // found, its place among them (from 1); not, -1, -2, ... in order.
    s32 *const remap = static_cast<s32 *>(GlobalMem_Alloc(at<u16>(model, 0x36) * 4u, 1));
    s32 added = 0, own = 0;
    bool found = false;   // (sic: the original's is never set for a model without textures; it reads its stack then)
    for (u32 i = 0; i < at<u16>(model, 0x36); ++i) {
        const char *const name = at<const char *>(at<const u8 *>(model, 0x54) + 0x10 * i, 0xc);
        s32 place = 0;
        found = false;
        const u8 *group = at<const u8 *>(sets, 4);
        for (u32 g = at<u32>(sets, 0); g && !found; --g, group += 0xc) {
            const u8 *tile = at<const u8 *>(group, 8);
            for (u32 n = at<u32>(group, 4); n; --n, tile += 0xc) {
                ++place;
                if (Crt().stricmp(at<const char *>(tile, 4), name) == 0) {
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            remap[i] = place;
        } else {
            ++added;
            remap[i] = --own;
        }
    }

    u32 strings = 0;   // each group's directory: dir then its name
    const u8 *group = at<const u8 *>(sets, 4);
    for (u32 g = at<u32>(sets, 0); g; --g, group += 0xc) strings += length(dir) + length(at<const char *>(group, 0)) + 1;

    // One block: the textures (0x10 each), their 8-byte entries, then the directories.
    const u32 total = at<u32>(sets, 0x14) + (u32)added;
    u8 *const block = static_cast<u8 *>(GlobalMem_Alloc(total * 0x18 + strings, 1));
    at<u8 *>(sets, 0x18) = block;
    at<u8 *>(sets, 0x1c) = at<u8 *>(model, 0x54);
    u8 *const entries = block + total * 0x10;
    at<u8 *>(sets, 0x20) = entries;
    at<u8 *>(sets, 0x24) = at<u8 *>(model, 0x50);
    char *pool = reinterpret_cast<char *>(entries + total * 8);

    // The model's own textures (not found) first.
    u8 *texture = block, *entry = entries;
    const u8 *from = at<const u8 *>(sets, 0x1c), *fromEntry = at<const u8 *>(model, 0x50);
    for (u32 i = 0; i < at<u16>(model, 0x36); ++i, from += 0x10, fromEntry += 8) {
        if (remap[i] >= 0) continue;
        if (texture && from) std::memcpy(texture, from, 0x10);
        if (entry && fromEntry) std::memcpy(entry, fromEntry, 8);
        texture += 0x10;
        entry += 8;
        ++at<u32>(sets, 0x28);
    }

    // Then the sets' tiles, numbered on from there, named into a block of 0x14 bytes each (sets +8 of them).
    u8 *names = static_cast<u8 *>(GlobalMem_Alloc(at<u32>(sets, 8) * 20, 1));
    at<u8 *>(sets, 0x2c) = names;
    u32 index = (u32)added;
    group = at<const u8 *>(sets, 4);
    for (s32 g = 0; g < at<s32>(sets, 0); ++g, group += 0xc) {
        u8 *const tiles = at<u8 *>(group, 8);
        Crt().sprintf(pool, reinterpret_cast<const char *>(0x0074d320), dir, at<const char *>(group, 0));   // "%s%s"
        for (s32 j = 0; j < at<s32>(group, 4); ++j) {
            u8 *const tile = tiles + 0xc * j;
            at<u8 *>(texture, 0xc) = names;
            at<u32>(texture, 0) = 0;
            at<char *>(texture, 4) = pool;
            at<u16>(texture, 0xa) = 1;
            names += 0x14;
            Crt().strncpy(at<char *>(texture, 0xc), at<const char *>(tile, 4), 0x14);
            at<char *>(texture, 0xc)[0x13] = 0;
            texture += 0x10;
            at<u16>(entry, 0) = 1;
            at<u32>(entry, 4) = 0;
            at<u16>(tile, 8) = (u16)index++;
            entry += 8;
        }
        pool += length(pool) + 1;
        if (found) break;   // (sic: the last texture's lookup ends it after the first group)
    }

    Model_RemapTextures(model, remap, at<u32>(sets, 0x20), added);
    at<u8 *>(model, 0x54) = at<u8 *>(sets, 0x18);
    at<u32>(model, 0x50) = at<u32>(sets, 0x20);
    at<u16>(model, 0x36) = (u16)total;
    GlobalMem_Free(remap);
    return 1;
}
}
