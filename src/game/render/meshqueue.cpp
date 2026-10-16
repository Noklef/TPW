#include "meshqueue.h"

#include <cstdint>
#include <initializer_list>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall MeshQueue_Clear(u8 *queue) { at<u32>(queue, 0x400) = 0; }

void __cdecl MeshQueue2_Init() { global<u32>(0x008bb638) = 0; }

u8 *__fastcall Render3d_Init(u8 *object) {
    for (u32 i = 0; i < 16; i++)
        at<u32>(object, 0x9d58 + 4 * i) = 0;
    for (u32 offset : {0x9d58u, 0x9d6cu, 0x9d80u, 0x9d94u})
        at<u32>(object, offset) = 0x3f800000;
    static const struct { u32 offset, value; } kDefaults[] = {
        {0x3340, 0x3dcccccd}, {0x3314, 0x3dcccccd}, {0x3354, 0x3dcccccd}, {0x0000, 0x1},
        {0x3344, 0}, {0x3318, 0}, {0x331c, 0}, {0x3324, 0}, {0x3330, 0}, {0x332c, 0}, {0x3328, 0},
        {0x3350, 0}, {0x3348, 0}, {0x334c, 0}, {0x3308, 0}, {0xa188, 0}, {0x0004, 0},
        {0xa1b4, 0}, {0xa1b0, 0}, {0xa1ac, 0}, {0xa1a8, 0}, {0xa1a4, 0},
        {0x3334, 0xffffffff}, {0x3320, 0xbf800000}, {0x3338, 0x40a00000}, {0x333c, 0x3f333333},
        {0x3310, 0x3f19999a}, {0xa180, 0x40c00000}, {0xa184, 0x40000000}, {0xa16c, 0x3fa66666},
        {0xa18c, 0x3ecccccd}, {0xa190, 0x40c00000}, {0xa194, 0x41500000}, {0xa1b8, 0x3de147ae},
    };
    for (const auto &d : kDefaults)
        at<u32>(object, d.offset) = d.value;
    return object;
}

u32 __fastcall MeshQueue2_Add(u32 *queue, void *, u32 mesh) {
    s32 n = (s32)queue[0];
    if (n >= 0x100)
        return 0;
    queue[n + 1] = mesh;
    queue[0] = queue[0] + 1;
    return 1;
}
}
