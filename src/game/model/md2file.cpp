#include "md2file.h"

#include <cstdint>
#include <initializer_list>

#include "../core/globalmem.h"
#include "../core/vwrap.h"
#include "../files/vfs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The file offset at p + offset made a pointer into the model (0 left as it is).
void fix(const u8 *model, void *p, u32 offset) {
    u32 &v = at<u32>(p, offset);
    if (v)
        v += (u32)(uintptr_t)model;
}

// The same without the test for 0 (sic).
void add(const u8 *model, void *p, u32 offset) { at<u32>(p, offset) += (u32)(uintptr_t)model; }

void fix_tracks(u8 *model, u8 *motion) {
    u8 *const tracks = at<u8 *>(motion, 0x2c);
    if (!tracks)
        return;
    for (u32 i = 0; i < at<u16>(motion, 0x12); ++i) {
        u8 *const e = tracks + 0x40 * i;
        fix(model, e, 0x18);
        fix(model, e, 0x20);
        fix(model, e, 0x1c);
        if (at<u32>(e, 0x24)) {
            add(model, e, 0x24);
            fix(model, at<u8 *>(e, 0x24), 0xc);
        }
        if (at<u32>(e, 0x28)) {
            add(model, e, 0x28);
            u8 *const s = at<u8 *>(e, 0x28);
            if (at<u32>(e, 4) & 0x4000) {
                fix(model, s, 0);
                fix(model, s, 8);
            } else {
                if (at<u32>(s, 0xc)) {
                    add(model, s, 0xc);
                    u8 *const keys = at<u8 *>(s, 0xc);
                    for (u32 j = 0; j < at<u16>(s, 2); ++j) {
                        u8 *const key = keys + 0x14 * j;
                        fix(model, key, 4);
                        fix(model, key, 8);
                        fix(model, key, 0xc);
                    }
                }
                fix(model, s, 0x10);
            }
        }
        if (at<u32>(e, 0x2c)) {
            add(model, e, 0x2c);
            fix(model, at<u8 *>(e, 0x2c), 4);
            fix(model, at<u8 *>(e, 0x2c), 0xc);
            fix(model, at<u8 *>(e, 0x2c), 0x10);
        }
        fix(model, e, 0x30);
        fix(model, e, 0x34);
    }
}

void fix_geometry(u8 *model, u32 flags) {
    u8 *const materials = at<u8 *>(model, 0x54);
    for (u32 i = 0; i < at<u16>(model, 0x36); ++i) {
        u8 *const m = materials + 0x10 * i;
        if (at<u32>(m, 4))
            at<u32>(m, 4) = 0;
        fix(model, m, 0xc);
        if (flags & 4) {
            at<u16>(m, 0xa) = 1;
            at<u32>(m, 0) &= ~1u;
        }
    }
    if (at<u8 *>(model, 0x6c)) {
        add(model, at<u8 *>(model, 0x6c), 0x28);
        add(model, at<u8 *>(model, 0x6c), 0x2c);
    }
    u8 *const records = at<u8 *>(model, 0xac);
    for (u32 i = 0; i < at<u16>(model, 0x40); ++i) {
        fix(model, records + 0x10 * i, 8);
        fix(model, records + 0x10 * i, 0xc);
    }
    u8 *const nodes = at<u8 *>(model, 0x70);
    for (u32 i = 0; i < at<u16>(model, 0x44); ++i) {
        u8 *const node = nodes + 0xa0 * i;
        for (u32 offset : {4u, 8u, 0xcu, 0x60u, 0x64u, 0x68u, 0x70u, 0x94u})
            fix(model, node, offset);
        if (at<u32>(node, 0x6c)) {
            add(model, node, 0x6c);
            u8 *const groups = at<u8 *>(node, 0x6c);
            for (u32 j = 0; j < at<u16>(node, 0x5a); ++j)
                fix(model, groups + 0x10 * j, 0);
        }
        fix(model, node, 0x54);
    }
    u8 *other = at<u8 *>(model, 0x74);
    for (u32 n = (u32)at<u16>(model, 0x42) - at<u16>(model, 0x44); n; --n, other += 0x58) {
        fix(model, other, 4);
        fix(model, other, 8);
        fix(model, other, 0xc);
        fix(model, other, 0x54);
    }
}
}

extern "C" u8 *__cdecl MD2_Load(const char *name, u32 flags) {
    if (!name)
        return nullptr;
    u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(name));   // (0x74cfb8, "rb", pushed too)
    if (!stream)
        return nullptr;
    const u32 size = Object_CallSlot14(stream);
    u8 *const block = static_cast<u8 *>(GlobalMem_Alloc(size + 4, 1));
    u8 *const model = reinterpret_cast<u8 *>(((uintptr_t)block + 7) & ~(uintptr_t)7);
    Object_CallSlot8Product((u32)(uintptr_t)model, size, 1, stream);
    Object_FinishAndDelete(stream);
    const u32 magic = at<u32>(model, 0);
    at<u8 *>(model, 0xb0) = block;
    bool ok = magic == 0x1cd15d46 && at<u32>(model, 4) <= 0xdd && (at<u32>(model, 4) == 0xdd || (flags & 1));
    if (ok)
        ok = (flags & 2) ? !at<u32>(model, 0x98) : at<u32>(model, 0x98) != 0;
    if (!ok) {
        GlobalMem_Free(block);
        return nullptr;
    }
    at<u8 *>(model, 0x9c) = model;
    for (u32 offset = 0x4c; offset <= 0x78; offset += 4)
        fix(model, model, offset);
    fix(model, model, 0x98);
    fix(model, model, 0x7c);
    fix(model, model, 0xac);
    if (u8 *const records = at<u8 *>(model, 0x7c))
        for (u32 i = 0; i < at<u16>(model, 0x48); ++i) {
            fix(model, records + 0x14 * i, 0xc);
            fix(model, records + 0x14 * i, 0x10);
        }
    if (at<u8>(model, 0x30) & 1)
        fix_geometry(model, flags);
    u8 *const motion = at<u8 *>(model, 0x98);
    if (!motion)
        return model;
    if (at<u32>(model, 8) != 0xcb) {
        at<u32>(model, 0x98) = 0;
        if (at<u32>(model, 8) > 0xcb)
            at<u32>(model, 0x30) &= ~0x20u;
        else
            at<u32>(model, 0x30) = 0;
        return model;
    }
    if (at<u32>(motion, 0x20)) {
        add(model, motion, 0x20);
        u8 *const records = at<u8 *>(motion, 0x20);
        for (u32 i = 0; i < at<u16>(motion, 0xc); ++i) {
            fix(model, records + 0x10 * i, 8);
            fix(model, records + 0x10 * i, 0xc);
        }
    }
    for (u32 offset : {0x24u, 0x28u, 0x2cu, 0x38u, 0x3cu})
        fix(model, motion, offset);
    if (at<u32>(motion, 0x30)) {
        add(model, motion, 0x30);
        u8 *const records = at<u8 *>(motion, 0x30);
        for (u32 i = 0; i < at<u16>(motion, 0x18); ++i)
            add(model, records + 8 * i, 4);
    }
    fix_tracks(model, motion);
    return model;
}
