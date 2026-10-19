#include "meshdraws.h"

#include <cstdint>

#include "drawbuffer.h"
#include "engineerrors.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

}  // namespace

extern "C" {

u32 __fastcall MeshDraws_Add(u8 *list, void *, u32 matrix, u32, u32, u8 *mesh, u32 flags) {
    const s32 count = at<s32>(list, 0);
    if (count >= 0x1000) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 1, 0xb0754008);
        return 0;
    }
    const u32 vertices = at<u16>(mesh, 0x58);
    if (vertices == 0) return 1;
    u8 *const entry = list + 4 + count * 0x14;
    at<u8 *>(entry, 4) = mesh;
    at<u32>(entry, 0) = at<u32>(global<u8 *>(0x0087a6d0), 0x30);
    at<u32>(entry, 8) = flags | global<u32>(0x008bb624);
    const u32 room = DrawBuffer_Alloc(global<u8 *>(0x008bcbc4), nullptr, vertices);
    if (room == 0) return 1;  // (the count stays; the entry's first three dwords are written, sic)
    at<u32>(entry, 0xc) = room;
    using Slot = u32(__cdecl *)(void *, u32, u32, u32, u32, u32, u32);
    void *const transformer = global<void *>(0x0087b1d8);
    void *const *const vtable = *static_cast<void *const *const *>(transformer);
    reinterpret_cast<Slot>(vtable[2])(transformer, matrix, room, at<u32>(mesh, 0x60), 0x10, 0, vertices);
    at<u32>(entry, 0x10) = at<u32>(mesh, 0) & 0x1000000 ? at<u32>(mesh, 0x64) : at<u32>(mesh, 0x9c) + 4;
    at<s32>(list, 0) += 1;  // (read again: with a count of -1 the entry's faces dword is the count, sic)
    return 1;
}

}
