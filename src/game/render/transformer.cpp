#include "transformer.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../math/transform.h"
#include "../math/x87.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, s32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The virtual base's offset in a transformer block: its virtual base table's (+0) dword +4.
s32 BaseOffset(u8 *block) { return at<s32>(at<u8 *>(block, 0), 4); }

}  // namespace

extern "C" {

u32 __fastcall Transformer_Transform(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f) {
    using Slot = u32(__cdecl *)(void *, void *, u32, u32, u32, u32, u32, u32);
    void *const transformer = global<void *>(0x0087b1d8);
    void *const *const vtable = *static_cast<void *const *const *>(transformer);
    return reinterpret_cast<Slot>(vtable[1])(transformer, object, a, b, c, d, f, e);
}

u32 __cdecl Transformer_Create() {
    global<float>(0x0087b0d0) = (float)(X87Reg(global<float>(0x008bcbcc)) + global<float>(0x008bcbcc));
    global<u32>(0x0087b0e4) = 0xc0000000;
    global<u32>(0x0087afe8) = 0x0087aff0;
    global<u32>(0x0087b154) = 0x0087af68;
    global<u32>(0x0087b088) = 0x0087b158;
    global<u32>(0x0087af5c) = 0x0087b078;
    u32 kind = 1, table = 0x007018f0, vtable = 0x007018d0;
    if ((global<u32>(0x008bd534) & 0x2000000) && Transform_SseProbe())
        kind = 3, table = 0x00701960, vtable = 0x00701920;
    else if (global<u32>(0x008bd53c) != 0)
        kind = 2, table = 0x00701918, vtable = 0x007018f8;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x10));
    u8 *base = nullptr;
    if (block) {
        at<u32>(block, 0) = table;
        at<u32>(block, 8) = 0x00701940;
        at<u32>(block, BaseOffset(block)) = vtable;
        at<u32>(block, BaseOffset(block) - 4) = 0;
        base = block + BaseOffset(block);
    }
    global<u8 *>(0x0087b1d8) = base;
    global<u32>(0x008bd528) = kind;
    return 1;
}
}
