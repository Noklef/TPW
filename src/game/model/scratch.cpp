#include "scratch.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../math/x87.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }
template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

void __cdecl Model_AllocVertexScratch() {
    global<void *>(0x0079f7e0) = GlobalMem_Alloc(global<u32>(0x0079f7d0) * 12, 1);
}

void __cdecl Model_FreeVertexScratch() {
    GlobalMem_Free(global<void *>(0x0079f7e0));
    global<void *>(0x0079f7e0) = nullptr;
}

void __cdecl Model_BeginView(const u8 *view) {
    global<const u8 *>(0x0079fb18) = view;
    const u8 *const holder = at<const u8 *>(view, 0x28);
    if (holder && (at<u8>(holder, 0) & 8)) {
        global<u32>(0x0079fb10) = 2;
        Model_FillVertexScratch(view, 0);
        return;
    }
    const u8 *const model = at<const u8 *>(view, 4);
    const float y = at<float>(model, 0xa0);
    if ((at<u8>(model, 0x30) & 0x80) || y < 0.0f || y > 0.0f) {  // fcomp 0: neither equal nor unordered
        global<u32>(0x0079fb10) = 1;
        return;
    }
    global<u32>(0x0079fb10) = 4;
    Model_FillVertexScratch(view, 1);
}

void __cdecl Model_FillVertexScratch(const u8 *view, s32 mainOnly) {
    const u8 *const model = at<const u8 *>(view, 4);
    const s32 passes = mainOnly ? 1 : 2;
    const u8 *const root = at<const u8 *>(model, 0x78);
    const float ox = at<float>(root, 0x40), oy = at<float>(root, 0x44), oz = at<float>(root, 0x48);
    u32 base = 0;
    uintptr_t flags = 0, instance = 0;
    const u8 *node = nullptr;
    for (s32 pass = 0; pass < passes; ++pass) {
        u32 count;
        if (pass == 0) {
            node = at<const u8 *>(view, 0x2c);
            count = 1;
        } else {
            count = at<u16>(model, 0x48);
            if (count) {
                flags = at<uintptr_t>(model, 0x7c);
                instance = at<uintptr_t>(at<const u8 *>(view, 0x28), 4);
            }
        }
        for (u32 i = 0; i < count; ++i) {
            if (pass == 1) {
                const u32 index = at<u16>(model, 0x46) + i, big = at<u16>(model, 0x44);
                node = reinterpret_cast<const u8 *>(index < big ? at<uintptr_t>(model, 0x70) + index * 0xa0
                                                                : at<uintptr_t>(model, 0x74) + (index - big) * 0x58);
            }
            if (!(node && pass == 0) && !(pass == 1 && (at<u8>((void *)flags, i * 0x14) & 2))) continue;
            // the node's offset and its ancestors', but the root's
            const u8 *const parent = at<const u8 *>(node, 4);
            X87Reg x = 0.0, y = 0.0, z = 0.0;
            if (parent) {
                x = at<float>(node, 0x40);
                y = at<float>(node, 0x44);
                z = at<float>(node, 0x48);
            }
            for (const u8 *p = parent; p;) {
                const u8 *const up = at<const u8 *>(p, 4);
                if (up) {
                    x = x + X87Reg(at<float>(p, 0x40));   // (fld, faddp: x and y register forms, z a memory operand)
                    y = y + X87Reg(at<float>(p, 0x44));
                    z = z + at<float>(p, 0x48);
                }
                p = up;
            }
            float *out = reinterpret_cast<float *>(global<u8 *>(0x0079f7e0) + base * 12);
            if (pass == 1) at<u16>((void *)instance, i * 0x14 + 0x10) = (u16)base;
            const u32 n = at<u16>(node, 0x58);
            for (u32 k = 0; k < n; ++k, out += 3) {
                const float *const v = reinterpret_cast<const float *>(at<const u8 *>(node, 0x60) + (k >> 2) * 0x30) + (k & 3);
                const X87Reg wy = X87Reg(v[4]) + y + oy;
                const X87Reg ax = X87Reg(v[0]) + x;
                const X87Reg az = X87Reg(v[8]) + z;
                out[1] = (float)wy;
                switch (at<s32>(view, 0x10)) {
                case 90:
                    out[0] = (float)(az + ox);
                    out[2] = (float)(X87Reg(oz) - ax);
                    break;
                case 180:
                    out[0] = (float)(X87Reg(ox) - ax);
                    out[2] = (float)(X87Reg(oz) - az);
                    break;
                case 270:
                    out[0] = (float)(X87Reg(ox) - az);
                    out[2] = (float)(ax + oz);
                    break;
                default:
                    out[0] = (float)(ax + ox);
                    out[2] = (float)(az + oz);
                    break;
                }
            }
            base += n;
        }
    }
}

void __cdecl Model_InitUnusedTolerances() {
    global<u32>(0x0079f7dc) = 0x3ca3d70a;  // 0.02f
    global<u32>(0x0079fb1c) = 0x3ca3d70a;
}

void __cdecl Model_InitUnusedOne() { global<float>(0x0079fb24) = 1.0f; }

void __cdecl Model_InitUnusedEpsilon() { global<float>(0x0079fb28) = 1.0f / 8388608.0f; }
}
