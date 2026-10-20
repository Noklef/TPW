#include "wctimage.h"

#include <cstdint>
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../core/vwrap.h"
#include "../files/vfs.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The header word at the offset (from the part at +0x2c) as a quantiser step: 1 - q * scale (0x7011b8, and -0.25 at 0x7011c0 or -0.5 at
// 0x7011c4), exact in floats.
float Step(const u8 *part, u32 offset, float scale) { return 1.0f - (float)at<u16>(part, offset) * scale; }

// The format's pixel size (0x7011a0 indexed by [0x8bd554]).
u32 PixelSize() { return reinterpret_cast<const u32 *>((uintptr_t)0x007011a0)[global<u32>(0x008bd554)]; }

constexpr u32 kKind = (u32)-0x28, kWidth = (u32)-0x24, kHeight = (u32)-0x22, kStepB = (u32)-0x20, kStepC = (u32)-0x1e, kStepD = (u32)-0x1c,
              kStepA = (u32)-0x1a, kBlockA = (u32)-0xc, kBlockB = (u32)-8;

}  // namespace

extern "C" {

u32 __fastcall WctImage_Read(u8 *image, void *, u8 *stream) {
    u8 *const header = reinterpret_cast<u8 *>((uintptr_t)0x00879a38);
    Object_CallSlot8Product((u32)(uintptr_t)header, 0x2c, 1, stream);
    at<u8>(image, 6) = 0x20;
    at<u16>(image, 8) = at<u16>(header, 4);
    at<u16>(image, 0xa) = at<u16>(header, 6);
    const float a = at<float>(header, 0x14), b = at<float>(header, 0x18), c = at<float>(header, 0x1c), d = at<float>(header, 0x20);
    at<u16>(image, 0xc) = (u16)(long long)((X87Reg(a) + a) - 2.0f).v;   // 0x7011b0
    at<u16>(image, 0x10) = (u16)(long long)((X87Reg(b) * 4.0f) - 4.0f).v;   // 0x7011b4
    at<u16>(image, 0xe) = (u16)(long long)((X87Reg(c) * 4.0f) - 4.0f).v;
    at<u16>(image, 0x12) = (u16)(long long)(X87Reg(d) - 1.0f).v;   // 0x7011b8
    at<u32>(image, 0x14) = at<u32>(header, 0x24);
    at<u32>(image, 0x18) = at<u32>(header, 0x28);
    at<u32>(image, 0x24) = TrackedGlobal_Alloc(at<u32>(image, 0x14));
    if (!at<u32>(image, 0x24))
        return 0;
    Object_CallSlot8Product(at<u32>(image, 0x24), 1, at<u32>(image, 0x14), stream);
    at<u32>(image, 0x20) = TrackedGlobal_Alloc(at<u32>(image, 0x18));
    if (!at<u32>(image, 0x20)) {
        TrackedGlobal_Free(at<u32>(image, 0x24));
        at<u32>(image, 0x24) = 0;
        return 0;
    }
    Object_CallSlot8Product(at<u32>(image, 0x20), 1, at<u32>(image, 0x18), stream);
    return 1;
}

u32 __fastcall Texture_DecodeWct(const u8 *part, void *, u8 *const *pixels) {
    u8 *const codec = global<u8 *>(0x00879a64);
    vcall<u32>(codec, 8, (u32)at<u8>(part, kKind), (u32)at<u16>(part, kWidth), (u32)at<u16>(part, kHeight), PixelSize(), at<u32>(part, kBlockB),
               at<u32>(part, kBlockA), *pixels, Step(part, kStepB, -0.5f), Step(part, kStepD, -0.25f), Step(part, kStepC, -0.25f),
               (float)at<u16>(part, kStepA) - -1.0f);
    return 1;
}

u32 __fastcall Texture_DecodeWctInto(const u8 *part, void *, u32 *surface, u32 flip) {
    if (at<u8>(part, kKind) <= 0x12)
        return 0;
    if (flip) {
        surface[0] += (surface[2] - 1) * surface[3];
        surface[3] = 0u - surface[3];
    }
    u8 *const codec = global<u8 *>(0x00879a64);
    vcall<u32>(codec, 4, (u32)at<u8>(part, kKind), (u32)at<u16>(part, kWidth), (u32)at<u16>(part, kHeight), surface[1], surface[2], surface[3],
               PixelSize(), at<u32>(part, kBlockB), at<u32>(part, kBlockA), surface[0], Step(part, kStepB, -0.5f), Step(part, kStepD, -0.25f),
               Step(part, kStepC, -0.25f), (float)at<u16>(part, kStepA) - -1.0f);
    return 1;
}

u8 *__fastcall WctImage_DeletingDtor(u8 *part, void *, u32 flags) {
    u8 *image = part - 0x2c;
    at<u32>(image, at<u32>(at<u8 *>(image, 0), 4)) = 0x007011c8;
    const u32 base = at<u32>(at<u8 *>(image, 0), 4);
    at<u32>(image, base - 4) = base - 0x2c;
    if (at<u32>(image, 0x20)) {
        TrackedGlobal_Free(at<u32>(image, 0x20));
        at<u32>(image, 0x20) = 0;
    }
    if (at<u32>(image, 0x24)) {
        TrackedGlobal_Free(at<u32>(image, 0x24));
        at<u32>(image, 0x24) = 0;
    }
    for (u32 offset = 4; offset < 0x20; offset += 4) at<u32>(image, offset) = 0;
    at<u32>(image, 0x2c) = 0x00701178;
    if (flags & 1) GameHeap_Free(image);
    return image;
}

u8 *__fastcall Obj_CtorVirtual564ba0(u8 *object, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(object, 0) = 0x007011d8;
        at<u32>(object, 0x2c) = 0x00701178;
    }
    at<u32>(object, at<u32>(at<u8 *>(object, 0), 4)) = 0x007011c8;
    const u32 base = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object, base - 4) = base - 0x2c;
    for (u32 i = 0; i < 7; i++)
        at<u32>(object, 4 + 4 * i) = 0;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x20) = 0;
    return object;
}

u32 __fastcall WctImage_Load(u8 *image, void *, const char *name) {
    u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(name));
    if (!stream) return 0;
    Object_CallSlot8Product((u32)(uintptr_t)(image + 4), 0x1c, 1, stream);
    const u8 kind = at<u8>(image, 4);
    if (kind < 0x12 || kind > 0x13) {
        Object_FinishAndDelete(stream);
        return 0;
    }
    at<u32>(image, 0x24) = TrackedGlobal_Alloc(at<u32>(image, 0x14));
    if (!at<u32>(image, 0x24)) {
        Object_FinishAndDelete(stream);
        return 0;
    }
    Object_CallSlot8Product(at<u32>(image, 0x24), 1, at<u32>(image, 0x14), stream);
    if (at<u8>(image, 6) == 0x20) {
        at<u32>(image, 0x20) = TrackedGlobal_Alloc(at<u32>(image, 0x18));
        if (!at<u32>(image, 0x20)) {
            Object_FinishAndDelete(stream);
            TrackedGlobal_Free(at<u32>(image, 0x24));
            at<u32>(image, 0x24) = 0;
            return 0;
        }
        Object_CallSlot8Product(at<u32>(image, 0x20), 1, at<u32>(image, 0x18), stream);
    }
    at<u8>(image, 7) &= 3;
    Object_FinishAndDelete(stream);
    return 1;
}
}
