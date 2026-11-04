#include "meshtextures.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 textureOf(u32 set, u32 index) { return g_calls.Th(0x00575f90, set, index); }

void touch(u32 texture, u32 level) { g_calls.Th(0x0058c9f0, global<u32>(0x008c4b44), texture, level); }

}  // namespace

void Mesh_SetTextureCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" void __stdcall Mesh_TouchTextures(const u8 *mesh) {
    const u8 *const scene = global<const u8 *>(0x00879a20);
    const u32 level = at<float>(scene, 4) > 0.0f ? at<u32>(scene, 4) : 0;   // 0x7019f4
    const u32 records = at<u16>(mesh, 0x5a);
    for (u32 i = 0; i < records; ++i) {
        u8 *const record = at<u8 *>(at<u8 *>(mesh, 0x6c) + i * 0x10, 0);
        if (!record || !at<u32>(record, 4))
            continue;
        const u32 texture = textureOf(at<u32>(record, 4), global<u32>(0x008bd550) & 0x40 ? at<u16>(record, 2) : 0);
        if (!texture)
            continue;
        if (!(at<u32>(mesh, 0) & 0xc0000))
            g_calls.Th(0x00590f90, texture);
        if (at<u16>(record, 0) & 0x80 && at<u16>(record, 0) & 0x400) {
            g_calls.Th(0x0058dc50, global<u32>(0x008c4b44), texture);
            at<u16>(record, 0) &= 0xfbff;
        }
        touch(texture, level);
    }
    if (at<u32>(mesh, 0) & 0x2000000) {
        const u8 *const holder = at<const u8 *>(mesh, 0x98);
        const u32 set = holder ? at<u32>(holder, 4) : 0;
        touch(set ? textureOf(set, 0) : 0, level);
    }
}
