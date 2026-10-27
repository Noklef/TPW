#include "ridemodel.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/globalmem.h"
#include "../files/quickload.h"
#include "../gfx/texdirs.h"
#include "../motion/animator.h"
#include "../motion/track.h"
#include "baseuvs.h"
#include "holder.h"
#include "md2file.h"
#include "partstate.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

// Raises the global at address to value (unsigned).
void raise(u32 address, u32 value) {
    if (value > global<u32>(address)) global<u32>(address) = value;
}

}  // namespace

extern "C" {

void __cdecl Ride_MarkModel(const u32 *index) {
    u8 *const instance = reinterpret_cast<u8 *const *>(0x007a4610)[*index];
    at<u32>(at<u8 *>(at<u8 *>(instance, 8), 4), 0x30) |= 0x20;
    at<u32>(at<u8 *>(at<u8 *>(at<u8 *>(instance, 8), 4), 0x78), 0) |= 0x80800;
    at<u32>(instance, 4) |= 0x1000000;
}

u32 __cdecl RideModel_Load(const char *name, u32 flags, u8 **out, char *outName, u32 layers) {
    alignas(4) u8 anim[0xfc] = {};
    char path[260], base[260], scratch[260];
    const char *const dir = TexDirs_First();
    if (!dir || (flags & 0x200))
        Crt().sprintf(path, name);   // (sic: the name is the format)
    else
        Crt().sprintf(path, text(0x0074d320), dir, name);   // "%s%s"
    const char *dot = Crt().strrchr(path, '.');
    if (!dot || dot == path || dot[1] == '\\')
        std::strcat(path, text(0x00747950));   // ".md2"
    std::strcpy(base, name);
    if (char *const cut = Crt().strrchr(base, '.'))
        if (cut != path && cut[1] != '\\')   // (sic: against the other buffer, so never equal)
            *cut = 0;
    u8 *const holder = Holder_LoadMd2(path, 0, 1, global<u32>(0x007b4980) & 8, 0);
    if (!holder)
        return 0;
    at<u8 *>(anim, 8) = holder;
    auto model = [&]() { return at<u8 *>(at<u8 *>(anim, 8), 4); };
    u8 *m = at<u8 *>(holder, 4);
    {
        const u8 *node = at<const u8 *>(m, 0x70);
        for (u32 n = at<u16>(m, 0x44); n; --n, node += 0xa0) {
            raise(0x007a42ac, at<u16>(node, 0x5c));
            raise(0x007a42ac, at<u16>(node, 0x58));
            raise(0x007a42ac, at<u16>(node, 0x5e));
        }
    }
    raise(0x007a42ac, at<u16>(at<u8 *>(holder, 4), 0x42));
    raise(0x007afd90, (u32)at<u16>(at<u8 *>(holder, 4), 0x3a) << 2);
    raise(0x007afd9c, at<u16>(at<u8 *>(holder, 4), 0x3e));
    raise(0x007afe10, at<u16>(at<u8 *>(holder, 4), 0x44));
    if (!(flags & 0x20000) && (at<u8>(at<u8 *>(holder, 4), 0x30) & 4)) {
        at<u32>(at<u8 *>(holder, 4), 0x30) &= ~4u;
        at<u32>(at<u8 *>(holder, 4), 0x30) |= 0x1000000;
    }
    if (outName)
        std::strcpy(outName, path);
    const u32 eight = flags & 8;
    if (eight) {
        u8 *node = at<u8 *>(at<u8 *>(holder, 4), 0x70);
        for (s32 i = 0; i < (s32)at<u16>(at<u8 *>(holder, 4), 0x44); ++i, node += 0xa0)
            if (Crt().stricmp(at<const char *>(node, 0x54), text(0x00747e94)) == 0)   // "Sky"
                at<u32>(node, 0) |= 0x4000000;
    }
    if (flags & 0x100) at<u32>(anim, 4) |= 8;
    if (flags & 0x10) at<u32>(model(), 0x30) |= 0x800;
    if (flags & 0x80) at<u32>(model(), 0x30) |= 0x100;
    if (flags & 0x800) at<u32>(model(), 0x30) |= 0x20000;
    if (flags & 0x1000) at<u32>(anim, 4) |= 0x100000;
    if (flags & 0x2000) at<u32>(anim, 4) |= 0x2000000;
    if (flags & 0x4000) at<u32>(model(), 0x30) |= 0x200000;
    if (flags & 0x8000) at<u32>(anim, 4) |= 0x10000000;
    if (flags & 0x800000) at<u32>(anim, 4) |= 0x20000000;

    // The motions: the first pass counts them, the second loads them.
    u32 &total = at<u32>(anim, 0xa8);
    u8 *cursor = nullptr, *loaded = nullptr;
    for (u32 pass = 0; pass < 2; ++pass) {
        const bool loading = pass != 0;
        if (loading) {
            at<void *>(anim, 0xac) = GlobalMem_Alloc(total * 4, 1);
            at<void *>(anim, 0xdc) = GlobalMem_Alloc(total * 4, 1);
            cursor = static_cast<u8 *>(GlobalMem_Alloc(total * 8, 1));
            at<u8 *>(anim, 0xd8) = cursor;
            total = 0;
        }
        for (u32 kind = 0; kind < 12; ++kind) {
            u8 *const bank = anim + 0x18 + 12 * kind;   // flags, count, entries
            const u32 want = at<u32>(bank, 4);
            at<u32>(bank, 4) = 0;
            u32 numbered = loading ? (at<u32>(bank, 0) & 0xff) >> 1 & 1 : 1;
            const s32 letter = global<s8>(0x006fe6bc + 8 * kind);
            for (;;) {
                if (flags & 9) {   // (sic: the copy is not used)
                    std::strcpy(scratch, base);
                    if (char *const cut = Crt().strrchr(scratch, '.'))
                        if (cut[1] != '\\') *cut = 0;
                }
                if (numbered)
                    Crt().sprintf(path, text(0x0074d2b4), dir, base, letter, at<u32>(bank, 4) + 1);   // "%s%s%c%d.md2"
                else
                    Crt().sprintf(path, text(0x0074d2a8), dir, base, letter);   // "%s%s%c.md2"
                s32 found = 0;
                if (loading) {
                    if (at<u32>(bank, 0) & 3) {
                        loaded = MD2_Load(path, 0);
                        if (loaded) {
                            if (at<u32>(loaded, 0x98))
                                found = 1;
                            else
                                GlobalMem_FreeAtB0(loaded);
                        }
                    }
                } else {
                    found = Quickload_Exists(path, global<u32>(0x007a445c) ? 2 : 0);
                }
                if (loading && at<u32>(bank, 4) == 0)
                    at<u32>(bank, 0) &= ~3u;
                if (!found) {
                    if (numbered && at<u32>(bank, 4) == 0) {
                        numbered = 0;
                        continue;
                    }
                    break;
                }
                if (loading) {
                    if (!at<u8 *>(bank, 8))
                        at<u8 *>(bank, 8) = cursor;
                    cursor += 8;
                    at<u32 *>(anim, 0xac)[total] = at<u32>(loaded, 0x98);
                    at<u8 **>(anim, 0xdc)[total] = loaded;
                    at<u32>(at<u8 *>(bank, 8), at<u32>(bank, 4) * 8) = total;
                    at<u32>(at<u8 *>(bank, 8), at<u32>(bank, 4) * 8 + 4) = at<u32>(loaded, 0x98);
                }
                at<u32>(bank, 0) |= numbered ? 2 : 1;
                ++at<u32>(bank, 4);
                ++total;
                if (at<u32>(bank, 4) == want || !numbered)
                    break;
            }
        }
    }

    u8 *const block = static_cast<u8 *>(GlobalMem_Alloc(layers * 0x38 + 0xfc, 1));
    at<u16>(anim, 0xe) = (u16)layers;
    *out = block;
    at<u8 *>(anim, 0x10) = block + 0xfc;
    for (u32 i = 0; i < total; ++i)
        Block_MarkModel(at<u8 *>(anim, 8), at<u8 **>(anim, 0xac)[i]);
    if (at<u8 *>(anim, 8) && model()) {
        if (flags & 0x20) {
            at<u32>(model(), 0x30) |= 0x20;
            at<u32>(at<u8 *>(model(), 0x78), 0) |= 0x80800;
            at<u32>(anim, 4) |= 0x1000000;
            Animator_SetPlaying(anim);
        }
        if (flags & 0x40) {
            at<u32>(model(), 0x30) |= 0x80;
            u8 *node = at<u8 *>(model(), 0x70);
            for (s32 i = 0; i < (s32)at<u16>(model(), 0x44); ++i, node += 0xa0)
                at<u32>(node, 0) |= 0x20000;
        }
    }
    if (flags & 0x400) {
        const u32 bits = 0x3ca3d70a;   // 0.02f
        float tolerance;
        std::memcpy(&tolerance, &bits, 4);
        RideBase_TidyAllUVs(anim, nullptr, tolerance, nullptr, nullptr);
    }
    u8 *const target = model();
    for (u32 i = 0; i < total; ++i) {
        const u8 *const motion = at<u8 **>(anim, 0xac)[i];
        const u32 tracks = at<u16>(motion, 0x12);
        if (!tracks)
            continue;
        const u8 *track = at<const u8 *>(motion, 0x2c);
        for (u32 n = tracks; n; --n, track += 0x40) {
            const u32 index = at<u16>(track, 0x14);
            const u8 *const mm = model();
            u8 *const node = index < at<u16>(mm, 0x44) ? at<u8 *>(mm, 0x70) + index * 0xa0
                                                        : at<u8 *>(mm, 0x74) + (index - at<u16>(mm, 0x44)) * 0x58;
            if (at<u32>(track, 4) & 0x1000)
                at<u32>(target, 0x30) |= 0x2000;
            if ((at<u32>(track, 4) & 0x10000) && at<u16>(node, 0x5a)) {
                const u8 *group = at<const u8 *>(node, 0x6c);
                for (u32 j = 0; j < at<u16>(node, 0x5a); ++j, group += 0x10) {
                    at<u32>(node, 0) |= 0x80;
                    if (u16 *const word = at<u16 *>(group, 0))
                        *word |= 0x8030;
                }
            }
        }
    }
    if (!eight)
        Holder_CreateAt20(at<u8 *>(anim, 8));
    if (flags & 0xa) {
        Animator_Reset(anim, 1, (s32)(flags & 0x40000), 0);
        if (!eight)
            Holder_Create(at<u8 *>(anim, 8));
        if ((flags & 2) && (flags & 4))
            at<u32>(anim, 4) |= 4;
        if (eight) {
            u8 *const mm = model();
            if (const u8 *const record = at<const u8 *>(mm, 0x6c)) {
                const u32 a = at<u32>(record, 0x18), b = at<u32>(record, 0x1c);
                at<u32>(mm, 0x30) |= 0x400;
                at<u32>(mm, 0xa8) = ((a - 1) & 0xff) << 16 | ((b - 1) & 0xff);
            }
        }
    }
    PartStates_Create(at<u8 *>(anim, 8));
    if (u8 *const to = *out)
        std::memcpy(to, anim, 0xfc);
    if (flags & 0xa)
        at<u8 *>(*out, 0) = *out;
    return 1;
}
}
